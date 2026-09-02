// title.hpp : title screen
//
// tlloop1() in the original reads glyph pixels out of app.gpage2_ at
// (a%16)*6+1+u, (a//16)*8+1+v (a 16-column, 6x8-cell grid keyed off ASCII
// code - 0x20) to decide where to spawn each car-shaped title particle.
// That only makes sense if gpage2_ contains a system font bitmap, and
// nothing in this repo (chara.lua's data table, included here as
// chara_data.hpp) ever paints one there -- Picotron's default-font memory
// layout isn't documented anywhere we could find. Rather than guess at
// undocumented engine internals, kGlyphs below is a small hand-authored
// 4x5 bitmap font covering just the letters "PICOT DRIFT" needs, used in
// place of the gpage2_ scan. Everything else here (state machine,
// projection math, rotation) is a direct port.
#pragma once
#include "db.hpp"
#include "sp.hpp"
#include "car.hpp"
#include "pause.hpp"
#include "game.hpp"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace db {

namespace detail {
struct Glyph { char ch; bool rows[5][4]; };
inline const Glyph kGlyphs[] = {
    {'P', {{1,1,1,0},{1,0,0,1},{1,1,1,0},{1,0,0,0},{1,0,0,0}}},
    {'I', {{0,1,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,1,1,0}}},
    {'C', {{0,1,1,1},{1,0,0,0},{1,0,0,0},{1,0,0,0},{0,1,1,1}}},
    {'O', {{0,1,1,0},{1,0,0,1},{1,0,0,1},{1,0,0,1},{0,1,1,0}}},
    {'T', {{1,1,1,1},{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}}},
    {'D', {{1,1,1,0},{1,0,0,1},{1,0,0,1},{1,0,0,1},{1,1,1,0}}},
    {'R', {{1,1,1,0},{1,0,0,1},{1,1,1,0},{1,0,1,0},{1,0,0,1}}},
    {'F', {{1,1,1,1},{1,0,0,0},{1,1,1,0},{1,0,0,0},{1,0,0,0}}},
};
inline const Glyph* find_glyph(char ch) {
    for (const auto& g : kGlyphs) if (g.ch == ch) return &g;
    return nullptr;
}
} // namespace detail

struct Title {
    int n_ = 0;
    std::string s_[2];
    struct Pos { double x = 0, y = 0, z = 0; };
    std::vector<Pos> pos_ = std::vector<Pos>(512);
    double rot_[3] = {};
    double sin_[3] = {};
    double cos_[3] = {};
    int blk_ = 0;
    int bgn_ = 0;
};
inline Title title;

void tlloop0();
void tlloop1();
void tlloop2();
void tlloop3();
void tlrotate();
void tldraw();

inline void tlinit() {
    app.gpage1_ = Userdata(30, 17);

    title.s_[0] = "PICOT";
    title.s_[1] = "DRIFT";
    for (int i = 0; i < 3; i++) { title.rot_[i] = 0; title.sin_[i] = 0; title.cos_[i] = 0; }

    int c0 = app.color_[0x01];
    for (int y = 0; y <= 16; y++)
        for (int x = 0; x <= 29; x++)
            app.gpage1_.set(x, y, c0);
    int c1 = app.color_[0x20];
    for (int y = 4; y <= 12; y++)
        for (int x = 4; x <= 25; x++)
            app.gpage1_.set(x, y, c1);
}

inline void tlloop() {
    if (app.state_ == 0) tlloop0();
    else if (app.state_ == 1) tlloop1();
    else if (app.state_ == 2) tlloop2();
    else if (app.state_ == 3) tlloop3();
    if (app.state_ >= 2) tldraw();
}

inline void tlloop0() {
    title.blk_ = 0;
    title.bgn_ = 0;
    sprset();
    psrset();
    app.state_ = 1;
}

inline void tlloop1() {
    int n = 0;
    double h = 2 * 6 - 1;
    for (int i = 0; i < 2; i++) {
        const std::string& str = title.s_[i];
        int l = (int)str.size();
        double w = l * 5 - 1;
        for (int j = 0; j < l; j++) {
            const detail::Glyph* g = detail::find_glyph(str[j]);
            double x = -w / 2 + j * 5;
            double y = -h / 2 + i * 6;
            if (!g) continue;
            for (int v = 0; v < 5; v++) {
                for (int u = 0; u < 4; u++) {
                    if (g->rows[v][u]) {
                        spchr(n, car.chr_[8][0], car.chr_[8][1] + 7 * 16, 16, 12, 0x00);
                        sphome(n, 8, 6);
                        title.pos_[n].x = x + u;
                        title.pos_[n].y = y + v;
                        title.pos_[n].z = 0;
                        n++;
                    }
                }
            }
        }
    }
    title.n_ = n;
    for (int i = 0; i < 3; i++) title.rot_[i] = 0;

    spchr(n, 0, 0, 30, 17, 0x00);
    spofs(n, 0, 0, 1024);
    sphome(n, 0, 0);
    spscale(n, 16, 16);
    sppage(n, &app.gpage1_);
    n++;
    spchr(n, 0, 176, 61, 7, 0x00);
    spofs(n, 240, 176, -64);
    sphome(n, 30, 3);
    spscale(n, 4, 4);
    spcolor(n, 0x18);

    app.state_ = 2;
}

inline void tlloop2() {
    if ((app.be_ & (16 | 128)) != 0) {
        sfx(18);
        app.debug_ = (app.be_ & 128) != 0 ? 1 : 0;
        app.state_ = 3;
    }
    tlrotate();
    title.blk_ += 1 * app.vsync_;
}

inline void tlloop3() {
    tlrotate();
    title.bgn_ += 1;
    if (title.bgn_ > 90.0 / app.vsync_) {
        app.state_ = 0;
        app.proc_ = gmloop;
    }
    title.blk_ += 8 * app.vsync_;
}

inline void tlrotate() {
    title.rot_[0] = std::fmod(title.rot_[0] + 4 * app.vsync_, 360.0);
    title.rot_[1] = std::fmod(title.rot_[1] + 2 * app.vsync_, 360.0);
    title.rot_[2] = std::fmod(title.rot_[2] + 1 * app.vsync_, 360.0);
    for (int i = 0; i < 3; i++) {
        double a = title.rot_[i] / 360.0;
        title.sin_[i] = dsin(a);
        title.cos_[i] = dcos(a);
    }
}

inline void tldraw() {
    for (int n = 0; n < title.n_; n++) {
        double x = title.pos_[n].x, y = title.pos_[n].y, z = title.pos_[n].z;
        double u = x;
        double v = y * title.cos_[0] - z * title.sin_[0];
        double w = y * title.sin_[0] + z * title.cos_[0];
        x = u * title.cos_[1] - w * title.sin_[1];
        y = v;
        z = u * title.sin_[1] + w * title.cos_[1];
        u = x * title.cos_[2] - y * title.sin_[2];
        v = x * title.sin_[2] + y * title.cos_[2];
        w = z + 36;
        if (w >= app.zn_) {
            double s = app.or_ / (w / 2.0);
            x = (u * s + 200) + 40;
            y = (v * s + 120) + 15;
            s = s / 12.0;
            spofs(n, x, y, std::min(1024.0, w * 16));
            spscale(n, s, s);
            sprot(n, 1.0 / 360.0);
            spshow(n);
        } else {
            sphide(n);
        }
    }
    int n = title.n_;
    spshow(n);
    n++;
    if ((title.blk_ & 0b00100000) == 0) spshow(n);
    else sphide(n);
}

} // namespace db
