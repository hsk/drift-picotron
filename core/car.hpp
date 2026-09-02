// car.hpp : car sprite palette variants
//
// chara.hpp already painted one car template (6 view-angle frames) at
// gpage2_ u=128..229, v=0..11 using a single reference color. This module
// stamps out 7 additional recolored copies below it (v = 16, 32, ... 112)
// so each rival/player car can be tinted without runtime palette swaps.
#pragma once
#include "db.hpp"

namespace db {

struct Car {
    int n_ = 0;
    double szw_ = 0, szh_ = 0;
    int clr_[8] = {};
    int chr_[16][3] = {};
};
inline Car car;

inline void cainit() {
    static const int raw_clr[8] = { 0x24, 0x21, 0x29, 0x27, 0x3c, 0x38, 0x09, 0x05 };
    static const int raw_chr[16][3] = {
        {128 + 0, 0, 0x00}, {144 + 1, 0, 0x00}, {160 + 2, 0, 0x00}, {160 + 2, 0, 0x00},
        {176 + 3, 0, 0x00}, {192 + 4, 0, 0x00}, {192 + 4, 0, 0x00}, {192 + 4, 0, 0x00},
        {208 + 5, 0, 0x00}, {192 + 4, 0, 0x08}, {192 + 4, 0, 0x08}, {192 + 4, 0, 0x08},
        {176 + 3, 0, 0x08}, {160 + 2, 0, 0x08}, {160 + 2, 0, 0x08}, {144 + 1, 0, 0x08},
    };

    car.n_ = 8;
    car.szw_ = 0.375;
    car.szh_ = car.szw_ * 0.75;

    for (int i = 0; i < 8; i++) car.clr_[i] = app.color_[raw_clr[i]];
    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 3; j++)
            car.chr_[i][j] = raw_chr[i][j];

    // recolor: for each of the 7 extra variants (car.clr_[1..7]), copy the
    // reference frame (v=0..15) replacing the reference color with the
    // variant color.
    for (int i = 1; i < 8; i++) {
        int r = car.clr_[i];
        for (int y = 0; y < 16; y++) {
            for (int x = 0; x < 17 * 6; x++) {
                int c = app.gpage2_.get(128 + x, y);
                if (c == car.clr_[0]) c = r;
                app.gpage2_.set(128 + x, i * 16 + y, c);
            }
        }
    }
}

} // namespace db
