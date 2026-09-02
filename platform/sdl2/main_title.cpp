// main_title.cpp : title-screen-only demo, for `make title`.
//
// Runs the normal app_update()/app_draw() cycle but pins app.proc_ back to
// tlloop every frame, so the title -> race hand-off (tlloop3, once gmloop
// is more than a stub) never actually leaves the title screen. Useful for
// iterating on title.hpp in isolation.
//
// This also doubles as a first experiment toward the eventual ESP32-P4
// target: game logic still renders into the normal, untouched 480x270
// db::screen (core/db.hpp's SCREEN_W/H, and everything derived from it
// like app.ox_/oy_, stay exactly as they are -- changing those would
// ripple through course/car/HUD math well beyond the title screen), but
// *presentation* here fits that into a 320x320 square canvas (letterboxed,
// matching the "draw at 320x320" plan for the real panel) and then crops
// the centered 256x192 window out of it -- no rotation yet, that's a
// separate later step once this pipeline looks right.
#include "../../core/app.hpp"
#include "audio_sdl2.hpp"
#include <SDL.h>
#include <cstdio>
#include <vector>

namespace {

constexpr int kPreW = 320, kPreH = 320;   // pre-rotation ESP32-P4 canvas
constexpr int kOutW = 256, kOutH = 192;   // physical panel resolution
constexpr int kWinScale = 3;              // just for a comfortably-sized dev window

void title_update() {
    db::app_update();
    db::app.proc_ = db::tlloop;
}

} // namespace

int main(int, char**) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Window* window = SDL_CreateWindow(
        "drift-picotron -- title demo (256x192 via 320x320, no rotation yet)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        kOutW * kWinScale, kOutH * kWinScale, 0);
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture* out_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24,
                                              SDL_TEXTUREACCESS_STREAMING, kOutW, kOutH);

    platform::Sdl2Audio audio;
    audio.init();
    db::app_init();

    std::vector<uint8_t> src_rgb((size_t)db::SCREEN_W * db::SCREEN_H * 3);

    // pre-rotation 320x320 canvas, and the source/dest rects for fitting
    // the 480x270 game render into it (letterboxed, aspect preserved).
    SDL_Surface* pre = SDL_CreateRGBSurfaceWithFormat(0, kPreW, kPreH, 24, SDL_PIXELFORMAT_RGB24);
    double fit_scale = std::min((double)kPreW / db::SCREEN_W, (double)kPreH / db::SCREEN_H);
    SDL_Rect fit_dst;
    fit_dst.w = (int)(db::SCREEN_W * fit_scale);
    fit_dst.h = (int)(db::SCREEN_H * fit_scale);
    fit_dst.x = (kPreW - fit_dst.w) / 2;
    fit_dst.y = (kPreH - fit_dst.h) / 2;

    // centered 256x192 crop out of the 320x320 canvas -- this is the part
    // that will become "crop after rotation" once rotation is added.
    SDL_Rect crop;
    crop.x = (kPreW - kOutW) / 2;
    crop.y = (kPreH - kOutH) / 2;
    crop.w = kOutW;
    crop.h = kOutH;

    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e))
            if (e.type == SDL_QUIT) running = false;
        const Uint8* keys = SDL_GetKeyboardState(nullptr);
        if (keys[SDL_SCANCODE_ESCAPE]) running = false;

        title_update();
        db::app_draw();

        // db::screen (indexed) -> RGB24
        for (size_t i = 0; i < db::screen.data.size(); i++) {
            const db::RGB& c = db::PALETTE[db::screen.data[i]];
            src_rgb[i * 3 + 0] = c.r;
            src_rgb[i * 3 + 1] = c.g;
            src_rgb[i * 3 + 2] = c.b;
        }
        SDL_Surface* src = SDL_CreateRGBSurfaceWithFormatFrom(
            src_rgb.data(), db::SCREEN_W, db::SCREEN_H, 24, db::SCREEN_W * 3, SDL_PIXELFORMAT_RGB24);

        SDL_FillRect(pre, nullptr, SDL_MapRGB(pre->format, 0, 0, 0));
        SDL_BlitScaled(src, nullptr, pre, &fit_dst);
        SDL_FreeSurface(src);

        SDL_UpdateTexture(out_tex, nullptr, (Uint8*)pre->pixels + crop.y * pre->pitch + crop.x * 3, pre->pitch);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, out_tex, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    SDL_FreeSurface(pre);
    audio.shutdown();
    SDL_DestroyTexture(out_tex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
