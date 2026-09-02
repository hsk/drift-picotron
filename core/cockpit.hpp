// cockpit.hpp : dashboard HUD (speedometer, rank, lap counter, timer)
//
// cprset() lays out a fixed sequence of sprite slots (housings + digit
// placeholders) once; cpdraw() only ever changes which glyph each already-
// positioned slot shows, so the digit math below never touches spofs --
// the on-screen left-to-right layout comes entirely from the positions
// cprset() assigned.
#pragma once
#include "db.hpp"
#include "sp.hpp"
#include "race.hpp"
#include "mycar.hpp"
#include <algorithm>

namespace db {

struct Cockpit {
    int n_ = 0;
    int rnk_ = 0;
    int spr_ = 0;
};
inline Cockpit cockpit;

inline void cpinit() {
    cockpit.n_ = 0;
    cockpit.spr_ = 408;
}

inline void cprset() {
    cockpit.rnk_ = 0;
    int n = cockpit.spr_;
    spchr(n, 0, 40, 48, 8, 0x00);
    spofs(n, 0, 216 + 30, -24);
    sphome(n, 0, 0);
    spscale(n, 2, 2);
    n++;
    for (int i = 0; i <= 2; i++) {
        spchr(n, 0, 8, 6, 7, 0x00);
        spofs(n, (2 - i) * 15 + 8, 209 + 30, -32);
        sphome(n, 0, 0);
        spscale(n, 3, 3);
        n++;
    }
    spchr(n, 48, 40, 48, 8, 0x00);
    spofs(n, 304 + 80, 216 + 30, -24);
    sphome(n, 0, 0);
    spscale(n, 2, 2);
    n++;
    spchr(n, 96, 40, 7, 8, 0x00);
    spofs(n, 336 + 80, 216 + 30, -32);
    sphome(n, 0, 0);
    spscale(n, 2, 2);
    n++;
    spchr(n, 0, 48, 30, 8, 0x00);
    spofs(n, 304 + 80, 200 + 30, -24);
    sphome(n, 0, 0);
    spscale(n, 2, 2);
    n++;
    spchr(n, 0, 8, 6, 7, 0x00);
    spofs(n, 360 + 80, 212 + 30, -96);
    sphome(n, 6, 7);
    spscale(n, 5, 5);
    spcolor(n, app.color_[0x28]);
    n++;
    spchr(n, 30, 48, 18, 8, 0x00);
    spofs(n, 364 + 80, 200 + 30, -24);
    sphome(n, 0, 0);
    spscale(n, 2, 2);
    n++;
    spchr(n, 0, 56, 64, 8, 0x00);
    spofs(n, 16, 16, -24);
    sphome(n, 0, 0);
    spscale(n, 2, 2);
    n++;
    for (int i = 1; i <= race.stn_; i++) {
        spchr(n, 64, 56, 7, 8, 0x00);
        spofs(n, 72 + (i - 1) * 14, 16, -32);
        sphome(n, 0, 0);
        spscale(n, 2, 2);
        n++;
    }
    spchr(n, 96, 56, 20, 8, 0x00);
    spofs(n, 16, 32, -24);
    sphome(n, 0, 0);
    spscale(n, 2, 2);
    n++;
    spchr(n, 42, 0, 6, 7, 0x00);
    spofs(n, 92, 32, -24);
    sphome(n, 0, 0);
    spscale(n, 2, 2);
    n++;
    spchr(n, 12, 0, 6, 7, 0x00);
    spofs(n, 122, 32, -24);
    sphome(n, 0, 0);
    spscale(n, 2, 2);
    n++;
    for (int i = 0; i <= 5; i++) {
        int f0 = (i > 1) ? 1 : 0;
        int f1 = (i > 3) ? 1 : 0;
        spchr(n, 0, 8, 6, 7, 0x00);
        spofs(n, 142 - (i + f0 + f1) * 10, 32, -32);
        sphome(n, 0, 0);
        spscale(n, 2, 2);
        n++;
    }
    cockpit.n_ = n - cockpit.spr_;
}

inline void cploop() {
    if (cockpit.rnk_ < 32) cockpit.rnk_ += race.clr_;
}

inline void cpdraw() {
    int n = cockpit.spr_;
    spshow(n);
    n++;

    int s = (int)std::floor(mycar.spd_ * 244);
    int i = 0;
    do {
        int u = s % 10;
        s /= 10;
        spchr(n, u * 6, 8, 6, 7, 0x01);
        spshow(n);
        n++;
        i++;
    } while (s != 0);
    while (i < 3) {
        sphide(n);
        n++;
        i++;
    }

    spshow(n);
    n++;
    s = std::min(4, std::max(1, mycar.lpc_)) - 1;
    spchr(n, 96 + 7 * s, 40, 7, 8, 0x01);
    spofs(n, 336 + 14 * s + 80, 216 + 30, -32);
    spshow(n);
    n++;

    s = 5 + 19 * (cockpit.rnk_ / 32);
    int t = mycar.rnk_[race.stg_] + 1;
    spshow(n);
    n++;
    spchr(n, t * 6, 8, 6, 7, 0x01);
    spscale(n, s, s);
    spshow(n);
    n++;

    s = std::min(4, t) - 1;
    spchr(n, 30 + s * 18, 48, 18, 8, 0x01);
    spshow(n);
    n++;

    spshow(n);
    n++;
    for (int j = 1; j <= race.stg_ - 1; j++) {
        spchr(n, 64 + mycar.rnk_[j] * 7, 56, 7, 8, 0x01);
        spshow(n);
        n++;
    }
    spchr(n, 85, 56, 8, 8, 0x01);
    spshow(n);
    n++;
    for (int j = race.stg_ + 1; j <= race.stn_; j++) {
        sphide(n);
        n++;
    }

    spshow(n);
    n++;
    spshow(n);
    n++;
    spshow(n);
    n++;

    s = (int)std::max(0.0, race.tim_);
    t = (s % 60) * 100 / 60;
    s /= 60;
    spchr(n + 0, (t % 10) * 6, 8, 6, 7, 0x01);
    spchr(n + 1, (t / 10) * 6, 8, 6, 7, 0x01);
    t = s % 60;
    s /= 60;
    spchr(n + 2, (t % 10) * 6, 8, 6, 7, 0x01);
    spchr(n + 3, (t / 10) * 6, 8, 6, 7, 0x01);
    spchr(n + 4, (s % 10) * 6, 8, 6, 7, 0x01);
    spchr(n + 5, (s / 10) * 6, 8, 6, 7, 0x01);
    spshow(n + 0);
    spshow(n + 1);
    spshow(n + 2);
    spshow(n + 3);
    spshow(n + 4);
    spshow(n + 5);
    n += 6;
    (void)n;
}

} // namespace db
