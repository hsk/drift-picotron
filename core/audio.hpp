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

namespace db {

namespace audio {

constexpr double kTicksPerSecond = 120.0; // tick_len==0 default, see picotron_synth.html
constexpr int kMaxVoices = 24;            // enough for one 8-track BGM group + a few one-shot sfx

inline double pitch_to_freq(int pitch) {
    // picotron pitch: 0 = c-0, 48 = middle c (c-4); midi = pitch + 12
    return 440.0 * std::pow(2.0, (pitch - 57) / 12.0);
}

enum class Wave { Square, Triangle, Saw };

inline Wave inst_to_wave(int inst) {
    switch (inst) {
        case 2: return Wave::Triangle;
        case 4: return Wave::Saw;
        default: return Wave::Square;
    }
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
    for (auto& v : voices)
        if (v.active && v.loop) stop_voice(v);
}

// mixes `n_frames` mono samples (each in roughly [-1,1] before the caller's
// own headroom scaling) into `out`, advancing every active voice by
// n_frames/sample_rate seconds.
inline void render(float* out, int n_frames, int sample_rate) {
    for (int i = 0; i < n_frames; i++) out[i] = 0;

    for (auto& v : voices) {
        if (!v.active || !v.track) continue;
        const SfxTrackDef& t = *v.track;
        double ticks_per_sample = kTicksPerSecond / sample_rate;

        for (int i = 0; i < n_frames; i++) {
            if (v.row >= 64) {
                if (v.loop) { v.row = 0; v.row_elapsed_ticks = 0; }
                else { stop_voice(v); break; }
            }
            int pitch = t.pitch[v.row];
            int vol = t.vol[v.row];
            if (pitch != 0xff && vol != 0xff && vol > 0) {
                double freq = pitch_to_freq(pitch);
                Wave w = inst_to_wave(t.inst[v.row] == 0xff ? 0 : t.inst[v.row]);
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

// which underlying tracks each music() id loops together, see
// tools/gen_sfx_data.py's docstring for how track numbering was found:
// tracks 0-7 are one 8-part BGM arrangement, 8-17 a second (fanfare-ish)
// section, and 18-23 map 1:1 to the game's sfx(18..23) one-shots.
inline const std::vector<int>& music_group(int n) {
    static const std::vector<int> race_bgm = {0, 1, 2, 3, 4, 5, 6, 7};
    static const std::vector<int> fanfare = {8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
    static const std::vector<int> empty = {};
    switch (n) {
        case 0: return race_bgm;
        case 4:
        case 5:
        case 6: return fanfare;
        default: return empty;
    }
}

} // namespace audio

inline void sfx(int n) {
    audio::play_track_no(n, false);
}

inline void music(int n) {
    audio::stop_all_looping();
    if (n < 0) return;
    for (int track_no : audio::music_group(n))
        audio::play_track_no(track_no, true);
}

} // namespace db
