// signal.hpp : start signal (countdown lights)
#pragma once
#include "db.hpp"
#include "sp.hpp"
#include "race.hpp"
#include <cmath>
#include <algorithm>

namespace db {

struct Signal {
    int x_ = 0, y_ = 0, h_ = 0;
    int spr_ = 0;
};
inline Signal signal;

inline void sginit() {
    signal.x_ = 200 + 40;
    signal.y_ = 64 + 15;
    signal.h_ = 80 + 15;
    signal.spr_ = 440;
}

inline void sgrset() {
    int n = signal.spr_;
    spchr(n, 48, 96, 32, 8, 0x00);
    sphome(n, 16, 3);
    spscale(n, 4, 4);
    n++;
    for (int i = 0; i <= 3; i++) {
        int f = (i == 3) ? 1 : 0;
        spchr(n, 48 + f * 5, 104, 5, 5, 0x00);
        sphome(n, 2, 2);
        spscale(n, 4, 4);
        n++;
    }
}

inline void sgloop() {
    double t = std::abs(race.tim_);
    if (race.tim_ <= 0) {
        if (std::fmod(t, 16) < app.vsync_) {
            if ((int)(t / 16) == 0) sfx(20);
            else sfx(19);
        }
    }
}

inline void sgdraw() {
    int n = signal.spr_;
    double t = std::min(32.0, std::max(0.0, race.tim_));
    double x = signal.x_;
    double y = signal.y_ - signal.h_ * (1 - dcos((t * M_PI / 64) / (2 * M_PI)));
    int s = 3 - ((int)std::max(0.0, -race.tim_ + 15) / 16);
    spofs(n, x, y, -64);
    spshow(n);
    n++;
    for (int i = 0; i <= 3; i++) {
        if (i <= s) {
            spofs(n, x - 44 + i * 28, y, -72);
            spshow(n);
        } else {
            sphide(n);
        }
        n++;
    }
}

} // namespace db
