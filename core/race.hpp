// race.hpp : race state (timer, rank tracking, clear/over flags)
#pragma once
#include "db.hpp"

namespace db {

struct Race {
    int stg_ = 0;   // current stage (1-based, matches course.lua's data table)
    int stn_ = 0;   // stage count
    int lop_ = 0;   // laps
    double tim_ = 0;
    int rnk_[8] = {};
    int myc_ = 0;
    int str_ = 0;
    int clr_ = 0;
    int ovr_ = 0;
    int flg_ = 0;
};
inline Race race;

inline void rcinit() {
    race.stn_ = 5;
    race.lop_ = 4;
    for (int i = 0; i < 8; i++) race.rnk_[i] = 0;
}

inline void rcrset() {
    race.tim_ = -63;
    for (int i = 0; i < 8; i++) race.rnk_[i] = -1;
    race.myc_ = -1;
    race.str_ = 0;
    race.clr_ = 0;
    race.ovr_ = 0;
    race.flg_ = 0;
}

inline void rcloop() {
    if (race.myc_ < 0) {
        for (int i = 0; i < 8; i++)
            if (race.rnk_[i] == 7) race.myc_ = i;
    }
    if (race.tim_ < 60 * 60 * 60 - 1 && race.myc_ < 0) {
        race.tim_ += race.str_ * app.vsync_;
        if (race.tim_ > 60 * 60 * 60 - 1) race.tim_ = 60 * 60 * 60 - 1;
    }
    race.clr_ = (race.myc_ >= 0 && race.myc_ <= 2) ? 1 : 0;
    race.ovr_ = (race.rnk_[2] >= 0 && (race.myc_ < 0 || race.myc_ > 2)) ? 1 : 0;
    race.flg_ = (race.tim_ >= 0 && race.str_ != 0 && (race.clr_ + race.ovr_ == 0)) ? 1 : 0;
}

inline void rcdraw() {}

} // namespace db
