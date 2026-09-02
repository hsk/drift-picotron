// main_sdl2.cpp : desktop entry point.
//
// Only the sole non-header translation unit in this project on purpose --
// everything else lives in headers so the game core stays trivially
// reusable from a future ESP32-P4 platform layer.
#include "../../core/app.hpp"
#include "platform_sdl2.hpp"
#include "audio_sdl2.hpp"

int main(int, char**) {
    platform::Sdl2App app;
    if (!app.init(3, "drift-picotron (SDL2)")) return 1;
    platform::Sdl2Audio audio;
    audio.init();
    app.run(db::app_init, db::app_update, db::app_draw);
    audio.shutdown();
    app.shutdown();
    return 0;
}
