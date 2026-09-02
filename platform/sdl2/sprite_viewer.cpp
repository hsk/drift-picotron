// sprite_viewer.cpp : dev tool -- lays out every procedurally-painted
// character sprite (and a strip of the bg-derived 16x16 tiles) on screen
// and dumps a PNG, so pixel-art/palette conversion can be sanity-checked
// without eyeballing a live window. Not part of the game; SDL2_image is an
// extra dependency scoped to this one tool.
#include "../../core/app.hpp"
#include <SDL.h>
#include <SDL_image.h>
#include <cstdio>
#include <vector>

int main(int argc, char** argv) {
    const char* out_path = argc > 1 ? argv[1] : "sprite_sheet.png";

    db::app_init(); // spinit/bginit/clinit/chinit -- populates gpage2_/gpage3_/spr_bank

    // lay out every chara sprite def in a grid
    const int cell_w = 40, cell_h = 28, cols = 12;
    int n = 0;
    for (int i = 0; i < kCharaSprites_count; i++, n++) {
        const SpriteDef& def = kCharaSprites[i];
        int col = i % cols, row = i / cols;
        int cx = 4 + col * cell_w + (cell_w - def.w) / 2;
        int cy = 4 + row * cell_h + (cell_h - def.h) / 2;
        db::spchr(n, def.u, def.v, def.w, def.h, 0);
        db::sppage(n, &db::app.gpage2_);
        db::sphome(n, 0, 0);
        db::spscale(n, 1, 1);
        db::spofs(n, cx, cy, 0);
        db::spcolor(n, 0x07);
        db::spshow(n);
    }

    // strip of bg-derived 16x16 tiles (mostly blank -- course.* isn't ported
    // yet, only chara.lua's 4 ground-texture patches are pre-painted).
    int strip_y = 4 + ((kCharaSprites_count + cols - 1) / cols) * cell_h + 8;
    int strip_cols = 20;
    for (int i = 0; i < 140 && n < 512; i++, n++) {
        db::Userdata* tile = db::get_spr(i);
        if (!tile) continue;
        int col = i % strip_cols, row = i / strip_cols;
        int cx = 4 + col * 17;
        int cy = strip_y + row * 17;
        if (cy + 16 > db::SCREEN_H) break;
        db::spchr(n, 0, 0, 16, 16, 0);
        db::sppage(n, tile);
        db::sphome(n, 0, 0);
        db::spscale(n, 1, 1);
        db::spofs(n, cx, cy, 0);
        db::spcolor(n, 0x07);
        db::spshow(n);
    }

    db::cls(0x06); // light-gray background so transparent (0x3f) areas read clearly
    db::spdraw();

    // Render db::screen -> RGB24 -> SDL_Surface -> PNG, no window needed.
    SDL_Init(SDL_INIT_VIDEO);
    std::vector<uint8_t> rgb((size_t)db::SCREEN_W * db::SCREEN_H * 3);
    for (size_t i = 0; i < db::screen.data.size(); i++) {
        const db::RGB& c = db::PALETTE[db::screen.data[i]];
        rgb[i * 3 + 0] = c.r;
        rgb[i * 3 + 1] = c.g;
        rgb[i * 3 + 2] = c.b;
    }
    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormatFrom(
        rgb.data(), db::SCREEN_W, db::SCREEN_H, 24, db::SCREEN_W * 3, SDL_PIXELFORMAT_RGB24);
    if (!surf) {
        std::fprintf(stderr, "SDL_CreateRGBSurfaceWithFormatFrom failed: %s\n", SDL_GetError());
        return 1;
    }
    if (IMG_SavePNG(surf, out_path) != 0) {
        std::fprintf(stderr, "IMG_SavePNG failed: %s\n", IMG_GetError());
        return 1;
    }
    std::printf("wrote %s (%d sprites shown)\n", out_path, n);
    SDL_FreeSurface(surf);
    SDL_Quit();
    return 0;
}
