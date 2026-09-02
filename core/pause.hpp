// pause.hpp : pause overlay
#pragma once
#include "db.hpp"
#include "sp.hpp"

namespace db {

struct Pause {
    int spr_ = 0;
};
inline Pause pause_state;

inline void psrset() {
    int n = pause_state.spr_;
    spchr(n, 0, 168, 26, 7, 0x00);
    spofs(n, 200 + 40, 120 + 15, -256);
    sphome(n, 13, 3);
    spscale(n, 2, 2);
}

inline void psinit() {
    pause_state.spr_ = 511;
    psrset();
}

inline void psloop() {}

inline void psdraw() {
    int n = pause_state.spr_;
    if (app.pause_ == 1) spshow(n);
    else sphide(n);
}

} // namespace db
