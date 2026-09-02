// game.hpp : top-level game state machine (title hand-off -> race -> clear/
// over -> back to title)
//
// rank/navi/cockpit/back are still stubs (core/{rank,navi,cockpit,back}.hpp)
// -- their loop/draw calls are wired in below (matching gmloop's original
// call list) so adding the real implementations later is a pure fill-in,
// but for now they're no-ops.
#pragma once
#include "db.hpp"
#include "sp.hpp"
#include "race.hpp"
#include "course.hpp"
#include "obj.hpp"
#include "rival.hpp"
#include "mycar.hpp"
#include "signal.hpp"
#include "rank.hpp"
#include "clear.hpp"
#include "over.hpp"
#include "navi.hpp"
#include "cockpit.hpp"
#include "back.hpp"

namespace db {

void tlloop(); // defined in title.hpp; forward-declared to avoid a title.hpp <-> game.hpp cycle

struct Game {
    int t_ = 0;
};
inline Game game;

void gmloop0();
void gmloop1();
void gmloop2();

inline void gminit() {}

inline void gmloop() {
    if (app.state_ == 0) gmloop0();
    else if (app.state_ == 1) gmloop1();
    else if (app.state_ == 2) gmloop2();

    if (app.state_ >= 2) {
        rcloop(); csloop(); obloop(); rvloop(); myloop(); sgloop();
        rkloop(); gcloop(); goloop(); nvloop(); cploop(); bkloop();
        rcdraw(); csdraw(); obdraw(); rvdraw(); mydraw(); sgdraw();
        rkdraw(); gcdraw(); godraw(); nvdraw(); cpdraw(); bkdraw();
    }
}

inline void gmloop0() {
    race.stg_ = 1;
    sprset();
    psrset();
    app.state_ = 1;
}

inline void gmloop1() {
    rcrset();
    csmake();
    obmake();
    rvrset();
    myrset();
    sgrset();
    rkrset();
    gcrset();
    gorset();
    nvrset();
    cprset();
    bkrset();
    if (app.sound_) music(0);
    game.t_ = 0;
    app.state_ = 2;
}

inline void gmloop2() {
    if (race.clr_ + race.ovr_ > 0) {
        if (race.clr_ > 0) {
            if (game.t_ == 0) {
                music(-1);
                if (race.stg_ == race.stn_) { if (app.sound_) music(4); }
                else { if (app.sound_) music(5); }
            }
        } else {
            if (game.t_ == 0) { if (app.sound_) music(6); }
        }
        game.t_++;
        int t = 256;
        if (race.stg_ == race.stn_) t += 256 * race.clr_;
        if ((game.t_ > 32 && (app.be_ & 16) != 0) || game.t_ > t) {
            music(-1);
            if (race.clr_ != 0) {
                if (race.stg_ < race.stn_) {
                    race.stg_++;
                    app.state_ = 1;
                } else {
                    app.proc_ = tlloop;
                    app.state_ = 0;
                }
            } else {
                app.proc_ = tlloop;
                app.state_ = 0;
            }
        }
    }
}

} // namespace db
