// clear.hpp : game clear (confetti + car portrait)
#pragma once
#include "db.hpp"
#include "sp.hpp"
#include "car.hpp"
#include "race.hpp"

namespace db {

struct Clear {
    int n_ = 0;
    double t_ = 0;
    int spr_ = 0;
};
inline Clear clear;

inline void gcinit() {
    clear.n_ = 32;
    clear.spr_ = 464;
}

inline void gcrset() {
    clear.t_ = -96;
    int n = clear.spr_;
    for (int i = 0; i <= clear.n_ - 2; i++) {
        int f = (rnd(4) < 1) ? 1 : 0;
        spchr(n, 0, 144 + f * 8, 80, 7, 0x00);
        spofs(n, rnd(272 + 80) + 64, rnd(208 + 30) + 16, -128 - i);
        sphome(n, 40, 3);
        sprot(n, rnd(60) - 30);
        spscale(n, 4, 4);
        spcolor(n, (int)rnd(31) + 1);
        n++;
    }
    spchr(n, car.chr_[5][0], car.chr_[5][1] + (7 * 16), 16, 12, 0x08);
    spofs(n, 200 + 40, 120 + 15, -192);
    sphome(n, 8, 6);
    spscale(n, 8, 8);
}

inline void gcloop() {
    if (race.stg_ == race.stn_) clear.t_ += race.clr_;
}

inline void gcdraw() {
    for (int i = 0; i <= clear.n_ - 1; i++) {
        int n = clear.spr_ + i;
        if (clear.t_ > i * 2) spshow(n);
        else sphide(n);
    }
}

} // namespace db
