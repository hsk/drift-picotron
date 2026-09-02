// db.hpp : picotron-like primitives, platform-independent (no SDL/OS deps).
//
// Drawing here only ever touches the logical `screen` buffer (indexed
// color, unrotated, native 480x270). Presenting that buffer to a real
// display -- palette lookup, scaling, and on ESP32-P4 eventually a final
// rotate-on-transfer -- is entirely the platform layer's job (see
// platform/sdl2/platform_sdl2.hpp). Keep it that way: this header must stay
// includable from a future ESP32-P4 backend with no changes.
#pragma once
#include <cstdint>
#include <cstring>
#include <cmath>
#include <array>
#include <vector>
#include <memory>
#include <algorithm>

namespace db {

constexpr int SCREEN_W = 480;
constexpr int SCREEN_H = 270;

struct RGB { uint8_t r, g, b; };

// Picotron default (WIP v5) 32-color palette. Indices 32-63 are unused by
// the game's color table but padded so palette-slot math (0x00-0x3f) never
// goes out of range; 0x3f is the conventional transparent sentinel.
inline const std::array<RGB, 64> PALETTE = {{
    {0x00,0x00,0x00}, {0x1d,0x2b,0x53}, {0x7e,0x25,0x53}, {0x00,0x87,0x51},
    {0xab,0x52,0x36}, {0x5f,0x57,0x4f}, {0xc2,0xc3,0xc7}, {0xff,0xf1,0xe8},
    {0xff,0x00,0x4d}, {0xff,0xa3,0x00}, {0xff,0xec,0x27}, {0x00,0xe4,0x36},
    {0x29,0xad,0xff}, {0x83,0x76,0x9c}, {0xff,0x77,0xa8}, {0xff,0xcc,0xaa},
    {0x1c,0x5e,0xac}, {0x00,0xa5,0xa1}, {0x75,0x4e,0x97}, {0x12,0x53,0x59},
    {0x74,0x2f,0x29}, {0x49,0x2d,0x38}, {0xa2,0x88,0x79}, {0xff,0xac,0xc5},
    {0xc3,0x00,0x4c}, {0xeb,0x6b,0x00}, {0x90,0xec,0x42}, {0x00,0xb2,0x51},
    {0x64,0xdf,0xf6}, {0xbd,0x9a,0xdf}, {0xe4,0x0d,0xab}, {0xff,0x85,0x6d},
    {0xff,0x00,0xff}, {0xff,0x00,0xff}, {0xff,0x00,0xff}, {0xff,0x00,0xff},
    {0xff,0x00,0xff}, {0xff,0x00,0xff}, {0xff,0x00,0xff}, {0xff,0x00,0xff},
    {0xff,0x00,0xff}, {0xff,0x00,0xff}, {0xff,0x00,0xff}, {0xff,0x00,0xff},
    {0xff,0x00,0xff}, {0xff,0x00,0xff}, {0xff,0x00,0xff}, {0xff,0x00,0xff},
    {0xff,0x00,0xff}, {0xff,0x00,0xff}, {0xff,0x00,0xff}, {0xff,0x00,0xff},
    {0xff,0x00,0xff}, {0xff,0x00,0xff}, {0xff,0x00,0xff}, {0xff,0x00,0xff},
    {0xff,0x00,0xff}, {0xff,0x00,0xff}, {0xff,0x00,0xff}, {0xff,0x00,0xff},
    {0xff,0x00,0xff}, {0xff,0x00,0xff}, {0xff,0x00,0xff}, {0xff,0x00,0xff},
}};

// Indexed-color 2D pixel buffer, analogous to Picotron's userdata('u8', w, h).
class Userdata {
public:
    int w = 0, h = 0;
    std::vector<uint8_t> data;

    Userdata() = default;
    Userdata(int w_, int h_) : w(w_), h(h_), data((size_t)w_ * h_, 0) {}

    inline void set(int x, int y, int c) {
        if (x >= 0 && x < w && y >= 0 && y < h)
            data[(size_t)y * w + x] = (uint8_t)(c & 0xff);
    }
    inline int get(int x, int y) const {
        if (x >= 0 && x < w && y >= 0 && y < h)
            return data[(size_t)y * w + x];
        return 0;
    }
};

inline Userdata screen(SCREEN_W, SCREEN_H);

// -- shared game state (app.py / sp.py / bg.py equivalents) -----------------

struct App {
    int bp_ = 0, br_ = 0, be_ = 0, bl_ = 0;
    int sx_ = 0, sy_ = 0;
    int ox_ = 0, oy_ = 0, ow_ = 0, oh_ = 0, or_ = 0;
    int zn_ = 0, zf_ = 0;
    std::array<int, 64> color_{};
    Userdata gpage0_, gpage1_, gpage2_, gpage3_;
    int pause_ = 0, debug_ = 0, state_ = 0;
    void (*proc_)() = nullptr;
    int cycle_ = 0, vsync_ = 2;
    bool sound_ = true;
};
inline App app;

struct Sp {
    int chr_u = 0, chr_v = 0, chr_w = 16, chr_h = 16, chr_a = 0x00;
    double ofs_x = 0, ofs_y = 0, ofs_z = 0;
    double home_x = 0, home_y = 0;
    double scale_x = 1, scale_y = 1;
    double rot = 0;
    int color = 0x07;
    Userdata* page = nullptr;
    bool show = false;
};
inline std::vector<Sp> sp; // sized by sp.hpp's sprset()

struct Bg {
    int screen_w = 0, screen_h = 0;
    int ofs_x = 0, ofs_y = 0, ofs_z = 0;
    int home_x = 0, home_y = 0;
    double scale_x = 1, scale_y = 1;
    double rot = 0;
    bool show = false;
};
inline Bg bg;

inline std::vector<std::unique_ptr<Userdata>> spr_bank; // 256 background-derived sprite pages
inline void set_spr(int n, Userdata ud) {
    if ((int)spr_bank.size() <= n) spr_bank.resize(n + 1);
    spr_bank[n] = std::make_unique<Userdata>(std::move(ud));
}
inline Userdata* get_spr(int n) {
    return (n >= 0 && n < (int)spr_bank.size()) ? spr_bank[n].get() : nullptr;
}

// -- palette control --------------------------------------------------------

inline std::array<uint8_t, 64> pal_map = [] {
    std::array<uint8_t, 64> m{};
    for (int i = 0; i < 64; i++) m[i] = (uint8_t)i;
    return m;
}();
inline std::array<bool, 64> palt_flags{};

inline void pal() { for (int i = 0; i < 64; i++) pal_map[i] = (uint8_t)i; }
inline void pal(int a, int b) { pal_map[a] = (uint8_t)b; }
inline void palt(int idx, bool flag) { palt_flags[idx] = flag; }

// -- picotron-style trig (turns, y-flipped like pico-8/picotron) ------------

inline double dsin(double x) { return -std::sin(x * 2.0 * M_PI); }
inline double dcos(double x) { return std::cos(x * 2.0 * M_PI); }
inline double dceil(double x) { return std::ceil(x); }

// -- drawing ------------------------------------------------------------

inline void cls(int c = 0) {
    std::memset(screen.data.data(), c & 0xff, screen.data.size());
}

inline void plot(int x, int y, int c) {
    if (palt_flags[c]) return;
    if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H)
        screen.data[(size_t)y * SCREEN_W + x] = pal_map[c];
}

// Axis-aligned scaled (optionally x-flipped) sprite blit.
inline void sspr(Userdata* sprite, double sx, double sy, double sw, double sh,
                  double dx, double dy, double dw, double dh, bool flip_x = false) {
    if (!sprite || dw <= 0 || dh <= 0 || sw <= 0 || sh <= 0) return;
    int dx0 = (int)std::lround(dx);
    int dy0 = (int)std::lround(dy);
    int dw_i = std::max(1, (int)std::lround(dw));
    int dh_i = std::max(1, (int)std::lround(dh));
    for (int j = 0; j < dh_i; j++) {
        int ty = dy0 + j;
        if (ty < 0 || ty >= SCREEN_H) continue;
        int v = (int)sy + (j * (int)sh) / dh_i;
        for (int i = 0; i < dw_i; i++) {
            int tx = dx0 + i;
            if (tx < 0 || tx >= SCREEN_W) continue;
            int su = (i * (int)sw) / dw_i;
            int u = (int)sx + (flip_x ? ((int)sw - 1 - su) : su);
            int c = sprite->get(u, v);
            plot(tx, ty, c);
        }
    }
}

// Textured horizontal(-ish) scanline; sp.hpp always calls with y0 == y1.
inline void tline3d(Userdata* sprite, double x0, double y0, double x1, double y1,
                     double u0, double v0, double u1, double v1, int mx = 1, int my = 1) {
    (void)y1; (void)mx; (void)my;
    if (!sprite) return;
    int x0i = (int)std::lround(x0);
    int x1i = (int)std::lround(x1);
    int y = (int)std::lround(y0);
    if (y < 0 || y >= SCREEN_H) return;
    int n = x1i - x0i;
    if (n == 0) {
        plot(x0i, y, sprite->get((int)std::lround(u0), (int)std::lround(v0)));
        return;
    }
    int step = n > 0 ? 1 : -1;
    int steps = std::abs(n);
    for (int i = 0; i <= steps; i++) {
        int x = x0i + i * step;
        if (x < 0 || x >= SCREEN_W) continue;
        double t = (double)i / steps;
        int u = (int)std::lround(u0 + (u1 - u0) * t);
        int v = (int)std::lround(v0 + (v1 - v0) * t);
        plot(x, y, sprite->get(u, v));
    }
}

// -- background / tilemap (unused by the game's actual render path, kept as
//    a light stand-in since bg.hpp's bgdraw() is dead code in app_draw) ----

inline void mset(int, int, int) {}
inline void bg_map(int, int, int, int) {}

// -- audio (deferred; sfx/0.sfx's binary format is unreverse-engineered, see
//    project notes -- these are no-ops so callers compile/link cleanly) ----

inline void sfx(int) {}
inline void music(int) {}

// -- input (platform layer fills these in once per frame) -------------------

struct Input {
    bool btn[7] = {};       // LEFT, RIGHT, UP, DOWN, O, X, MENU
    bool keyp1 = false;     // edge-triggered '1' key (speed x1)
    bool keyp2 = false;     // edge-triggered '2' key (speed x2)
    bool keyp_p = false;    // edge-triggered 'p' key (pause toggle)
};
inline Input input;

inline bool btn(int i) { return i >= 0 && i < 7 && input.btn[i]; }
inline bool keyp(char ch) {
    switch (ch) {
        case '1': return input.keyp1;
        case '2': return input.keyp2;
        case 'p': return input.keyp_p;
        default: return false;
    }
}

} // namespace db
