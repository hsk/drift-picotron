// rank.hpp : rank (overall standings HUD + finish-order bookkeeping)
//
// rcloop() (race.hpp) only knows a race is "cleared" once race.rnk_[]
// records mycar's finishing position, and rkloop() below is what actually
// writes that. So even though this looks like pure HUD code, it's a real
// gameplay dependency, not decoration -- without it, stages can never
// clear.
#pragma once
#include "db.hpp"
#include "sp.hpp"
#include "car.hpp"
#include "race.hpp"
#include "course.hpp"
#include "rival.hpp"
#include "mycar.hpp"
#include <algorithm>

namespace db {

struct Rank {
    int car_[8] = {};
    double dst_[8] = {};
    int spr_ = 0;
};
inline Rank rank;

inline void rkinit() {
    rank.spr_ = 448;
    for (int i = 0; i < 8; i++) { rank.car_[i] = 0; rank.dst_[i] = 0; }
}

inline void rkrset() {
    int n = rank.spr_;
    for (int i = 0; i <= 2; i++) {
        spchr(n, 0, 48, 30, 8, 0x00);
        spofs(n, 200 + 40, 64 + i * 48 + 15, -64);
        sphome(n, 0, 0);
        spscale(n, 2, 2);
        n++;
        spchr(n, (i + 1) * 6, 8, 6, 7, 0x00);
        spofs(n, 256 + 40, 76 + i * 48 + 15, -72);
        sphome(n, 6, 7);
        spscale(n, 5, 5);
        spcolor(n, app.color_[0x28]);
        n++;
        spchr(n, 30 + i * 18, 48, 18, 8, 0x00);
        spofs(n, 260 + 40, 64 + i * 48 + 15, -64);
        sphome(n, 0, 0);
        spscale(n, 2, 2);
        n++;
        spchr(n, car.chr_[8][0], car.chr_[8][1], 16, 12, 0x00);
        spofs(n, 160 + 40, 56 + i * 48 + 15, -72);
        sphome(n, 8, 6);
        spscale(n, 4, 4);
        n++;
    }
}

// descending sort of rank.dst_[first..last] (0-based, inclusive),
// rank.car_ permuted alongside it
inline void rkqsort(int first, int last) {
    if (first > last) return;
    int p = first;
    for (int i = first + 1; i <= last; i++) {
        if (rank.dst_[i] > rank.dst_[first]) {
            p++;
            std::swap(rank.dst_[p], rank.dst_[i]);
            std::swap(rank.car_[p], rank.car_[i]);
        }
    }
    std::swap(rank.dst_[p], rank.dst_[first]);
    std::swap(rank.car_[p], rank.car_[first]);
    rkqsort(first, p - 1);
    rkqsort(p + 1, last);
}

inline void rkloop() {
    // Total real competitors right now: rival.n_ rivals + mycar. The
    // original hardcodes this field size as car.n_ (== 8) because it's
    // always 7 rivals + 1 player in the finished game; with rival.hpp
    // still a stub (n_ = 0), car.n_ would be wrong here -- it'd walk 7
    // nonexistent "slots" that all degenerate to the same car-index 0 and
    // repeatedly clobber race.rnk_/mycar.rnk_, ending on the *last* of
    // those phantom slots instead of mycar's real finishing position.
    // This is exactly car.n_ once rival.n_ is 7 again, so no further
    // change will be needed when rival.lua is for-real ported.
    int field = rival.n_ + 1;

    for (int i = 0; i <= rival.n_ - 1; i++) {
        rank.car_[i] = i;
        rank.dst_[i] = rival.dst_[i];
    }
    rank.car_[rival.n_] = rival.n_;
    rank.dst_[rival.n_] = mycar.dst_;

    for (int i = 0; i <= field - 1; i++) {
        int j = race.rnk_[i];
        if (j >= 0) rank.dst_[j] += (car.n_ - i) * 10 * course.dsl_;
    }
    rkqsort(0, 7);

    for (int i = 0; i <= field - 1; i++) {
        int j = rank.car_[i];
        if (j < rival.n_) {
            rival.rnk_[j][0] = i;
            if (race.rnk_[i] < 0 && rival.lpc_[j] > race.lop_) race.rnk_[i] = j;
        } else {
            mycar.rnk_[race.stg_] = i;
            if (race.rnk_[i] < 0 && mycar.lpc_ > race.lop_) race.rnk_[i] = j;
        }
    }
    for (int i = 0; i <= rival.n_ - 1; i++)
        rival.rnk_[i][1] = rival.rnk_[i][0] - mycar.rnk_[race.stg_];
}

inline void rkdraw() {
    int n = rank.spr_;
    for (int i = 0; i <= 2; i++) {
        int j = race.rnk_[i];
        if (j >= 0) {
            spchr(n + 3, car.chr_[8][0], car.chr_[8][1] + j * 16, 16, 12, 0x01);
            spshow(n + 0); spshow(n + 1); spshow(n + 2); spshow(n + 3);
        } else {
            sphide(n + 0); sphide(n + 1); sphide(n + 2); sphide(n + 3);
        }
        n += 4;
    }
}

} // namespace db
