// platform_sdl2.hpp : SDL2 desktop backend.
//
// Owns everything OS/SDL-specific: window, event pump -> db::input, and
// presenting db::screen (palette lookup + scale) to the window. The ESP32-P4
// backend will mirror this file's shape but replace present() with a
// palette lookup + rotate + panel transfer instead of an SDL blit -- see
// core/db.hpp's header comment for why drawing never touches rotation.
#pragma once
#include "../../core/db.hpp"
#include <SDL.h>
#include <cstdio>
#include <vector>
#include <functional>

namespace platform {

class Sdl2App {
public:
    bool init(int scale, const char* caption) {
        scale_ = scale;
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
            std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
            return false;
        }
        window_ = SDL_CreateWindow(caption, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                    db::SCREEN_W * scale, db::SCREEN_H * scale, 0);
        if (!window_) {
            std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
            return false;
        }
        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer_) renderer_ = SDL_CreateRenderer(window_, -1, 0);
        texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGB24,
                                      SDL_TEXTUREACCESS_STREAMING, db::SCREEN_W, db::SCREEN_H);
        rgb_buf_.resize((size_t)db::SCREEN_W * db::SCREEN_H * 3);
        return true;
    }

    void shutdown() {
        if (texture_) SDL_DestroyTexture(texture_);
        if (renderer_) SDL_DestroyRenderer(renderer_);
        if (window_) SDL_DestroyWindow(window_);
        SDL_Quit();
    }

    // Pump events; returns false when the app should quit.
    bool pump_events() {
        db::input.keyp1 = db::input.keyp2 = db::input.keyp_p = false;
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) return false;
            if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
                switch (e.key.keysym.sym) {
                    case SDLK_1: db::input.keyp1 = true; break;
                    case SDLK_2: db::input.keyp2 = true; break;
                    case SDLK_p: db::input.keyp_p = true; break;
                    default: break;
                }
            }
        }
        const Uint8* keys = SDL_GetKeyboardState(nullptr);
        db::input.btn[0] = keys[SDL_SCANCODE_LEFT];
        db::input.btn[1] = keys[SDL_SCANCODE_RIGHT];
        db::input.btn[2] = keys[SDL_SCANCODE_UP];
        db::input.btn[3] = keys[SDL_SCANCODE_DOWN];
        db::input.btn[4] = keys[SDL_SCANCODE_Z];
        db::input.btn[5] = keys[SDL_SCANCODE_X];
        db::input.btn[6] = keys[SDL_SCANCODE_ESCAPE];
        return true;
    }

    // Palette lookup db::screen -> RGB24 -> texture -> scaled blit to window.
    void present() {
        const auto& pal = db::PALETTE;
        const auto& src = db::screen.data;
        for (size_t i = 0; i < src.size(); i++) {
            const db::RGB& c = pal[src[i]];
            rgb_buf_[i * 3 + 0] = c.r;
            rgb_buf_[i * 3 + 1] = c.g;
            rgb_buf_[i * 3 + 2] = c.b;
        }
        SDL_UpdateTexture(texture_, nullptr, rgb_buf_.data(), db::SCREEN_W * 3);
        SDL_RenderClear(renderer_);
        SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);
        SDL_RenderPresent(renderer_);
    }

    void run(const std::function<void()>& init_fn,
             const std::function<void()>& update_fn,
             const std::function<void()>& draw_fn) {
        init_fn();
        Uint32 next = SDL_GetTicks();
        bool running = true;
        while (running) {
            running = pump_events();
            if (!running) break;
            update_fn();
            draw_fn();
            present();
            next += 1000 / 60;
            Uint32 now = SDL_GetTicks();
            if (next > now) SDL_Delay(next - now);
            else next = now;
        }
    }

private:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* texture_ = nullptr;
    int scale_ = 1;
    std::vector<uint8_t> rgb_buf_;
};

} // namespace platform
