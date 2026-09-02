// app.hpp : application
//
// Mirrors app.lua/app.py. race/course/obj/rival/mycar/signal/rank/clear/
// over/navi/cockpit/back are still Lua-only and not yet ported, so gminit()
// / gmloop() (game.hpp) are placeholder stubs -- the title screen is fully
// wired up and is where app.proc_ starts and (for now) stays.
#pragma once
#include "db.hpp"
#include "sp.hpp"
#include "bg.hpp"
#include "color.hpp"
#include "chara.hpp"
#include "car.hpp"
#include "pause.hpp"
#include "game.hpp"
#include "title.hpp"

namespace db {

// create application instance
inline void app_init() {
    // setup application
    palt(0x00, false); // opaque
    palt(0x3f, true);  // transparent

    app = App{};
    app.ox_ = 200 + 40; // 200
    app.oy_ = 160 + 15; // 160
    app.ow_ = 200 + 40; // 200
    app.oh_ = 80 + 15;  // 80
    app.or_ = 256;
    app.zn_ = 2;
    app.zf_ = 48;
    app.proc_ = tlloop;
    app.vsync_ = 2;
    app.sound_ = true;

    // initialize libraries
    spinit();
    bginit();

    // initialize others
    clinit();
    chinit();
    cainit();
    psinit();
    tlinit();
    gminit();
}

// update
inline void app_update() {
    // switch speed
    if (keyp('1')) app.vsync_ = 1;
    if (keyp('2')) app.vsync_ = 2;

    // update cycle
    app.cycle_++;
    if (app.cycle_ >= app.vsync_) app.cycle_ = 0;

    // update button
    /*
        PICOTRON:
            0 1 2 3     LEFT RIGHT UP DOWN
            4 5         Buttons: O X
            6           MENU
        SMILE BASIC:
            b00 UP b01 DOWN b02 LEFT b03 RIGHT
            b04 A  b05 B    b06 X    b07 Y
            b08 L  b09 R    b11 ZL   b12 ZR
    */
    int b = 0b00000000;
    static const int b2b[8] = {
        0b00000100, 0b00001000, 0b00000001, 0b00000010,
        0b00010000, 0b00100000, 0b01000000, 0b00000000,
    };
    for (int i = 1; i < 8; i++)
        if (btn(i - 1)) b |= b2b[i];

    // control pause
    if (keyp('p')) app.pause_ = 1 - app.pause_;

    // update frame
    if (app.cycle_ == 0) {
        // control button
        b |= app.bl_;
        app.be_ = (b ^ app.bp_) & b;
        app.bp_ = b;
        app.bl_ = 0;
        if ((app.bp_ & 0b00000100) != 0) app.sx_ = -1;
        else if ((app.bp_ & 0b00001000) != 0) app.sx_ = 1;
        else app.sx_ = 0;
        if ((app.bp_ & 0b00000001) != 0) app.sy_ = -1;
        else if ((app.bp_ & 0b00000010) != 0) app.sy_ = 1;
        else app.sy_ = 0;

        // update scene
        if (app.pause_ == 0 && app.proc_) app.proc_();

        // update pause
        psloop();
        psdraw();
    } else {
        // skip frame
        app.bl_ = b;
    }
}

// draw
inline void app_draw() {
    if (app.cycle_ == 0) {
        spdraw();
    }
}

} // namespace db
