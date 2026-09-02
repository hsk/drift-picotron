// race.hpp : race logic (timer, rank tracking, clear/over flags)
//
// The Race and Rival state structs both live in db.hpp (not here or in
// rival.hpp) specifically so this file and rival.hpp can reference each
// other's fields without an #include cycle -- see db.hpp's comment there.
#pragma once
#include "db.hpp"

namespace db {

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
        // race.rnk_[i] holds which car-index finished in placement i.
        // rank.hpp always assigns mycar the car-index rival.n_ (the slot
        // right after the last rival) -- the original hardcoded this as
        // `7` since rival.n_ was always 7 (a full 7-rival field). With
        // rival.hpp currently stubbed to n_ = 0, mycar's car-index is 0,
        // not 7, so this has to track rival.n_ instead of the constant or
        // a finished race could never be detected as "cleared". Once
        // rival.lua is for-real ported (n_ back to 7) this is a no-op
        // change -- rival.n_ will equal 7 again.
        for (int i = 0; i < 8; i++)
            if (race.rnk_[i] == rival.n_) race.myc_ = i;
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
