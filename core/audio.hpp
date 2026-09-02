// audio.hpp : playback for the note data extracted by tools/gen_sfx_data.py
// (core/sfx_data.hpp) -- platform-independent (no SDL), so a future
// ESP32-P4 I2S backend can drive the exact same render() function SDL2's
// callback does.
//
// This does NOT emulate Picotron's real PFX6416 synth (node graph with
// FM/ring modulation, filters, wavetable morphing) -- that would need the
// instrument/envelope sections of the sfx data, which weren't decoded with
// full confidence. Instead each row's real extracted pitch and volume
// drives a simple oscillator (waveform picked from the row's instrument
// id), which is enough to play the original composition's actual melodies
// and rhythm with a chiptune-ish, not byte-exact, timbre.
#pragma once
#include "db.hpp"
#include "sfx_data.hpp"
#include <cmath>
#include <cstdint>
#include <vector>
#include <array>
#include <mutex>

namespace db {

namespace audio {

constexpr double kTicksPerSecond = 120.0; // tick_len==0 default, see picotron_synth.html
constexpr int kMaxVoices = 24;            // enough for one 8-track BGM group + a few one-shot sfx

inline double pitch_to_freq(int pitch) {
    // picotron pitch: 0 = c-0, 48 = middle c (c-4); midi = pitch + 12
    return 440.0 * std::pow(2.0, (pitch - 57) / 12.0);
}

enum class Wave { Square, Triangle, Saw };

// Deliberately NOT keyed off the row's real `inst` id: sections A (tracks
// 0-7) and B (tracks 8-17, see the MusicStage comment below) happen to
// lean on different inst ids in the source data (mostly square/saw vs
// mostly triangle), so picking the waveform from `inst` made the whole
// mix's timbre visibly change every time the section sequencer switched
// -- reported as "the timbre changed" once section B kicked in. Keying off
// the track number instead keeps the same square/triangle pattern in both
// sections since it doesn't depend on which section's data is playing.
inline Wave wave_for_track(int track_no) {
    return (track_no % 2 == 0) ? Wave::Square : Wave::Triangle;
}

inline double wave_sample(Wave w, double phase01) {
    double p = phase01 - std::floor(phase01); // wrap to [0,1)
    switch (w) {
        case Wave::Square:
            return p < 0.5 ? 1.0 : -1.0;
        case Wave::Triangle:
            return p < 0.5 ? (4.0 * p - 1.0) : (3.0 - 4.0 * p);
        case Wave::Saw:
            return 2.0 * p - 1.0;
    }
    return 0.0;
}

struct Voice {
    const SfxTrackDef* track = nullptr;
    bool active = false;
    bool loop = false;
    double row_elapsed_ticks = 0; // ticks elapsed within the current row
    int row = 0;
    double phase = 0; // oscillator phase, 0..1
};
inline std::array<Voice, kMaxVoices> voices;

// `voices` is written by db::sfx()/db::music() on the game thread and read
// + advanced by render() on the platform's audio callback thread (a
// separate OS thread on SDL2; will be an interrupt/task context on
// ESP32-P4 too). Without this, e.g. music() reassigning a voice mid-way
// through render() stepping it can tear its row/phase state -- observed
// as playback that never seems to advance past its first note/row.
// Recursive because the section sequencer (see below) needs to start/stop
// voices from inside render(), which already holds the lock for mixing.
inline std::recursive_mutex voices_mutex;

inline void stop_voice(Voice& v) {
    v.active = false;
    v.track = nullptr;
}

// start a one-shot (sfx) or looping (music) voice for a track number. If
// that exact track is already playing, leave it alone (don't restart it)
// instead of grabbing a new slot -- some call sites (e.g. mycar.hpp's
// mydrivctrl, matching the original mycar.lua) call sfx() every tick for
// as long as a condition holds, with no edge detection. Restarting on
// every one of those calls would re-trigger far faster than the sfx's own
// tempo, getting stuck retriggering just its first note (heard as a
// stuck buzz) instead of playing through it; always grabbing a new slot
// would instead exhaust every slot within a second or two, silently
// starving later sounds (including music()'s own tracks) of a free voice.
inline bool play_track_no(int track_no, bool loop) {
    const SfxTrackDef* t = find_sfx_track(track_no);
    if (!t) return false;
    std::lock_guard<std::recursive_mutex> lock(voices_mutex);
    for (auto& v : voices) {
        if (v.active && v.track == t) {
            v.loop = loop;
            return true;
        }
    }
    for (auto& v : voices) {
        if (!v.active) {
            v.track = t;
            v.active = true;
            v.loop = loop;
            v.row_elapsed_ticks = 0;
            v.row = 0;
            v.phase = 0;
            return true;
        }
    }
    return false;
}

inline void stop_all_looping() {
    std::lock_guard<std::recursive_mutex> lock(voices_mutex);
    for (auto& v : voices)
        if (v.active && v.loop) stop_voice(v);
}

// -- multi-section songs ---------------------------------------------------
//
// The user reported that the race BGM they remember has two distinct
// phrase types played long,long,short,short (repeating) -- not eight
// tracks all mixed together forever. tracks 0-7 and 8-17 (see
// tools/gen_sfx_data.py) are almost certainly exactly those two sections.
// Every extracted track has loop0 >= loop1 (no self-loop), so a track
// played once just stops -- the "x2" repeats have to be the sequencer
// explicitly re-triggering the same section twice, not a track looping
// itself. Without the sfx pod's Pattern data (attempted twice, never
// decoded with confidence -- see project notes; the "next pattern" index
// values it produced were nonsense) there's no confirmed source for the
// real section order/lengths, so this just cycles a hard-coded stage list
// on a timer sized to one natural track loop (64 rows) instead.
struct MusicStage {
    std::vector<int> track_nos;
    double hold_seconds;
};
struct MusicSequencer {
    std::vector<MusicStage> stages;
    int stage_idx = -1;
    double elapsed = 0;
    bool active = false;
};
inline MusicSequencer music_seq;

// caller must already hold voices_mutex
inline void enter_stage_locked(int idx) {
    for (auto& v : voices)
        if (v.active && v.loop) stop_voice(v);
    music_seq.stage_idx = idx;
    music_seq.elapsed = 0;
    if (idx < 0 || idx >= (int)music_seq.stages.size()) return;
    for (int track_no : music_seq.stages[idx].track_nos) {
        const SfxTrackDef* t = find_sfx_track(track_no);
        if (!t) continue;
        for (auto& v : voices) {
            if (!v.active) {
                v.track = t; v.active = true; v.loop = true;
                v.row_elapsed_ticks = 0; v.row = 0; v.phase = 0;
                break;
            }
        }
    }
}

inline void start_sequence(std::vector<MusicStage> stages) {
    std::lock_guard<std::recursive_mutex> lock(voices_mutex);
    music_seq.stages = std::move(stages);
    music_seq.active = !music_seq.stages.empty();
    enter_stage_locked(music_seq.active ? 0 : -1);
}

inline void stop_sequence() {
    std::lock_guard<std::recursive_mutex> lock(voices_mutex);
    music_seq.active = false;
    music_seq.stages.clear();
    for (auto& v : voices)
        if (v.active && v.loop) stop_voice(v);
}

// advances the section timer by n_frames/sample_rate seconds, switching to
// the next stage once the current one's hold time elapses. Caller must
// already hold voices_mutex.
inline void advance_sequence_locked(double dt_seconds) {
    if (!music_seq.active || music_seq.stages.empty()) return;
    music_seq.elapsed += dt_seconds;
    const auto& cur = music_seq.stages[music_seq.stage_idx < 0 ? 0 : music_seq.stage_idx];
    if (music_seq.elapsed >= cur.hold_seconds) {
        int next = (music_seq.stage_idx + 1) % (int)music_seq.stages.size();
        enter_stage_locked(next);
    }
}

// mixes `n_frames` mono samples (each in roughly [-1,1] before the caller's
// own headroom scaling) into `out`, advancing every active voice by
// n_frames/sample_rate seconds.
inline void render(float* out, int n_frames, int sample_rate) {
    for (int i = 0; i < n_frames; i++) out[i] = 0;

    std::lock_guard<std::recursive_mutex> lock(voices_mutex);
    advance_sequence_locked((double)n_frames / sample_rate);
    for (auto& v : voices) {
        if (!v.active || !v.track) continue;
        const SfxTrackDef& t = *v.track;
        double ticks_per_sample = kTicksPerSecond / sample_rate;

        for (int i = 0; i < n_frames; i++) {
            if (v.row >= 64) {
                // every extracted track has loop0 >= loop1 (no self-loop --
                // see project notes), so a track always just plays once and
                // stops; repetition is entirely the section sequencer's
                // job (re-triggering fresh voices), not this row wrapping.
                stop_voice(v);
                break;
            }
            int pitch = t.pitch[v.row];
            int vol = t.vol[v.row];
            if (pitch != 0xff && vol != 0xff && vol > 0) {
                double freq = pitch_to_freq(pitch);
                Wave w = wave_for_track(t.track_no);
                double amp = (vol / 64.0) * 0.12; // headroom: up to ~8 simultaneous voices
                out[i] += (float)(wave_sample(w, v.phase) * amp);
                v.phase += freq / sample_rate;
            }
            v.row_elapsed_ticks += ticks_per_sample;
            if (v.row_elapsed_ticks >= std::max(1, t.spd)) {
                v.row_elapsed_ticks -= std::max(1, t.spd);
                v.row++;
            }
        }
    }

    // safety clamp: headroom above assumes typical polyphony, but nothing
    // stops every voice slot from being active and loud at once.
    for (int i = 0; i < n_frames; i++) {
        if (out[i] > 1.0f) out[i] = 1.0f;
        else if (out[i] < -1.0f) out[i] = -1.0f;
    }
}

// track numbering per tools/gen_sfx_data.py: tracks 0-7 are one 8-part BGM
// section (spd=8) and 18-23 map 1:1 to the game's sfx(18..23) one-shots.
// Tracks 8-17 looked like one more section at first, but tracks 8-10 run
// at spd=6 (not 8, so they drift out of tempo against everything else)
// and their volume column fades out to ~0 by the end of the track --
// characteristic of a one-shot ending chord, not a section meant to loop
// forever alongside the rest. Reported as "too many/cluttered notes" when
// all ten were mixed together. Splitting them: 11-17 (spd=8, matching
// tempo) is section B of the main loop; 8-10 is reserved as a short
// ending stinger for the stage-clear/game-over cues instead.
// ~4.27s = one 64-row loop at spd=8, 120 ticks/sec.
constexpr double kSectionLoopSeconds = 64.0 * 8.0 / kTicksPerSecond;

inline const std::vector<int>& section_b_group() {
    static const std::vector<int> g = {11, 12, 13, 14, 15, 16, 17};
    return g;
}

inline const std::vector<int>& ending_group() {
    static const std::vector<int> g = {8, 9, 10};
    return g;
}

} // namespace audio

inline void sfx(int n) {
    audio::play_track_no(n, false);
}

inline void music(int n) {
    audio::stop_sequence();
    audio::stop_all_looping();
    if (n < 0) return;
    if (n == 0) {
        // Race BGM: reported as track pairs 01,23,45,67 cycling -- two
        // tracks (one melodic-ish even track, one drone-ish odd track,
        // matching wave_for_track's split) playing together at a time,
        // not all eight mixed into one wall of sound (which is what made
        // it sound cluttered/stuck-buzzing before).
        audio::start_sequence({
            {{0, 1}, audio::kSectionLoopSeconds},
            {{2, 3}, audio::kSectionLoopSeconds},
            {{4, 5}, audio::kSectionLoopSeconds},
            {{6, 7}, audio::kSectionLoopSeconds},
        });
        return;
    }
    // stage-clear / game-over stingers: the short fade-out chord (see
    // ending_group's comment above). No confirmed data on how music(4) vs
    // (5) vs (6) should really differ, so all three just play this once.
    for (int track_no : audio::ending_group())
        audio::play_track_no(track_no, true);
}

} // namespace db
