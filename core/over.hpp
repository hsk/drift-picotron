// over.hpp : game over banner
#pragma once
#include "db.hpp"
#include "sp.hpp"
#include "race.hpp"
#include <cmath>

namespace db {

struct Over {
    double t_ = 0;
    int spr_ = 0;
};
inline Over over;

inline void goinit() { over.spr_ = 496; }

inline void gorset() {
    over.t_ = -48;
    int n = over.spr_;
    spchr(n, 0, 160, 42, 7, 0x00);
    spofs(n, 200 + 40, 120 + 15, -128);
    sphome(n, 21, 3);
    spscale(n, 8, 8);
    spcolor(n, app.color_[0x16]);
}

inline void goloop() {
    if (over.t_ < 32) over.t_ += race.ovr_;
}

inline void godraw() {
    int n = over.spr_;
    if (race.ovr_ != 0 && over.t_ > 0) {
        double y = (120 + 15) - (160 + 15) * (1 + dsin((M_PI * over.t_ / 64) / (2 * M_PI)));
        spofs(n, 200 + 40, y, -128);
        spshow(n);
    } else {
        sphide(n);
    }
}

} // namespace db
