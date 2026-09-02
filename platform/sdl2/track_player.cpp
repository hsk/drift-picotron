// track_player.cpp : dev tool for `make trackplayer` -- plays exactly one
// extracted sfx track in a loop (auto-retriggering when it finishes) so
// individual tracks can be auditioned without the rest of the mix.
//
// Controls: LEFT/RIGHT switch to the previous/next track number (of the
// ones that actually exist, see kSfxTracks in core/sfx_data.hpp), ESC or
// closing the window quits. The current track's pitch/inst/vol rows are
// printed to the console so what you're hearing can be cross-checked
// against the data directly.
#include "../../core/audio.hpp"
#include "platform_sdl2.hpp"
#include "audio_sdl2.hpp"
#include <cstdio>
#include <cstdlib>

static int g_current_idx = 0; // index into kSfxTracks, not a track_no

static void print_track(const SfxTrackDef& t) {
    std::printf("\n=== track_no=%d (spd=%d) ===\n", t.track_no, t.spd);
    std::printf("pitch:");
    for (int i = 0; i < 64; i++) std::printf(" %d", t.pitch[i]);
    std::printf("\ninst :");
    for (int i = 0; i < 64; i++) std::printf(" %d", t.inst[i]);
    std::printf("\nvol  :");
    for (int i = 0; i < 64; i++) std::printf(" %d", t.vol[i]);
    std::printf("\n");
}

static void switch_to(int idx) {
    if (kSfxTracks_count == 0) return;
    idx = ((idx % kSfxTracks_count) + kSfxTracks_count) % kSfxTracks_count;
    g_current_idx = idx;
    db::audio::stop_sequence();
    db::audio::stop_all_looping();
    for (auto& v : db::audio::voices) v.active = false; // hard reset, dev tool only
    db::audio::play_track_no(kSfxTracks[idx].track_no, /*loop=*/false);
    print_track(kSfxTracks[idx]);
}

int main(int argc, char** argv) {
    int start_idx = 0;
    if (argc > 1) {
        int track_no = std::atoi(argv[1]);
        for (int i = 0; i < kSfxTracks_count; i++)
            if (kSfxTracks[i].track_no == track_no) { start_idx = i; break; }
    }

    platform::Sdl2App app;
    if (!app.init(3, "track_player -- LEFT/RIGHT to switch, ESC to quit")) return 1;
    platform::Sdl2Audio audio;
    audio.init();

    switch_to(start_idx);

    bool running = true;
    bool prev_left = false, prev_right = false;
    while (running) {
        running = app.pump_events();

        const Uint8* keys = SDL_GetKeyboardState(nullptr);
        bool left = keys[SDL_SCANCODE_LEFT];
        bool right = keys[SDL_SCANCODE_RIGHT];
        bool esc = keys[SDL_SCANCODE_ESCAPE];
        if (esc) running = false;
        if (left && !prev_left) switch_to(g_current_idx - 1);
        if (right && !prev_right) switch_to(g_current_idx + 1);
        prev_left = left;
        prev_right = right;

        // auto-retrigger the current track forever, one-shot style, so it
        // keeps playing without needing to touch the keyboard
        bool any_active = false;
        for (auto& v : db::audio::voices) if (v.active) any_active = true;
        if (!any_active) db::audio::play_track_no(kSfxTracks[g_current_idx].track_no, false);

        db::cls(0x01);
        app.present();
        SDL_Delay(16);
    }

    audio.shutdown();
    app.shutdown();
    return 0;
}
