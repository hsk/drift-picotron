// navi.hpp : mini-map
//
// Paints the course centerline into the *second* (unused-by-chara.hpp)
// half of app.gpage2_ (v=256..511) once per stage, then shows it as one
// big sprite each frame with small colored digit-glyphs (reusing the
// cockpit/rank digit strip at v=8) as car position markers.
#pragma once
#include "db.hpp"
#include "sp.hpp"
#include "car.hpp"
#include "course.hpp"
#include "rank.hpp"
#include <cmath>
#include <algorithm>

namespace db {

struct Navi {
    double x_ = 0, y_ = 0, w_ = 0, h_ = 0;
    int spr_ = 0, bg_ = 0;
};
inline Navi navi;

inline void nvinit() {
    navi.spr_ = 400;
    navi.bg_ = 510;
}

inline void nvrset() {
    for (int v = 256; v <= 511; v++)
        for (int u = 0; u <= 255; u++)
            app.gpage2_.set(u, v, 0x3f);

    int l = 0, r = 0, t = 0, b = 0;
    for (int i = 0; i <= 1; i++) {
        for (int j = 0; j <= course.n_ - 1; j++) {
            if (i == 0 || (i == 1 && course.pos_[0][j][1] > 0)) {
                int c = app.color_[0x0f - 0x08 * i];
                int x = (int)std::floor(course.pos_[0][j][0] + 0.5);
                int y = (int)std::floor(course.pos_[0][j][2] + 0.5);
                for (int v = 383 - y; v <= 385 - y; v++)
                    for (int u = 127 + x; u <= 129 + x; u++)
                        app.gpage2_.set(u, v, c);
                l = std::min(l, x);
                r = std::max(r, x);
                t = std::min(t, y);
                b = std::max(b, y);
            }
        }
    }
    {
        int x0 = 128 + (int)std::floor(course.pos_[1][0][0] + 0.5);
        int y0 = 384 - (int)std::floor(course.pos_[1][0][2] + 0.5);
        int x1 = 128 + (int)std::floor(course.pos_[2][0][0] + 0.5);
        int y1 = 384 - (int)std::floor(course.pos_[2][0][2] + 0.5);
        int c = app.color_[0x28];
        for (int y = y0; y <= y1; y++)   // no-op if y0 > y1, matching Lua's for-loop
            for (int x = x0; x <= x1; x++)
                app.gpage2_.set(x, y, c);
    }
    navi.x_ = 384 - r * 2 + 80;
    navi.y_ = 16 + b * 2;
    (void)l; (void)t;

    spchr(navi.bg_, 0, 256, 256, 256, 0x00);
    sphome(navi.bg_, 128, 128);
    spofs(navi.bg_, navi.x_, navi.y_, -40);
    spscale(navi.bg_, 2, 2);
    for (int i = 0; i <= car.n_ - 1; i++) {
        int n = navi.spr_ + i;
        spchr(n, 6 * i + 6, 8, 6, 7, 0x00);
        sphome(n, 3, 3);
        spscale(n, 2, 2);
    }
}

inline void nvloop() {}

inline void nvdraw() {
    spshow(navi.bg_);
    for (int i = 0; i <= car.n_ - 1; i++) {
        int r = rank.car_[i];
        double x, y;
        double z;
        if (r < rival.n_) {
            x = navi.x_ + std::floor(rival.pos_[r][0] + 0.5) * 2;
            y = navi.y_ - std::floor(rival.pos_[r][2] + 0.5) * 2;
            z = i - 48;
        } else {
            x = navi.x_ + std::floor(mycar.pos_[0] + 0.5) * 2;
            y = navi.y_ - std::floor(mycar.pos_[2] + 0.5) * 2;
            z = -56;
        }
        int n = navi.spr_ + i;
        spofs(n, x, y, z);
        spcolor(n, car.clr_[r]);
        spshow(n);
    }
}

} // namespace db
