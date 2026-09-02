// main_title.cpp : title-screen-only demo, for `make title`.
//
// Runs the normal app_update()/app_draw() cycle but pins app.proc_ back to
// tlloop every frame, so the title -> race hand-off (tlloop3, once gmloop
// is more than a stub) never actually leaves the title screen. Useful for
// iterating on title.hpp in isolation.
#include "../../core/app.hpp"
#include "platform_sdl2.hpp"
#include "audio_sdl2.hpp"

static void title_update() {
    db::app_update();
    db::app.proc_ = db::tlloop;
}

int main(int, char**) {
    platform::Sdl2App app;
    if (!app.init(3, "drift-picotron -- title demo")) return 1;
    platform::Sdl2Audio audio;
    audio.init();
    app.run(db::app_init, title_update, db::app_draw);
    audio.shutdown();
    app.shutdown();
    return 0;
}
