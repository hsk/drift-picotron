// mycar.hpp : player car (physics, camera follow, collision, drawing)
#pragma once
#include "db.hpp"
#include "sp.hpp"
#include "car.hpp"
#include "course.hpp"
#include "cam.hpp"
#include "race.hpp"
#include "rival.hpp"
#include "obj.hpp"
#include <cmath>
#include <algorithm>

namespace db {

struct MyCar {
    double dst_ = 0, lpd_ = 0, sde_ = 0;
    int lpc_ = 0, lpi_ = 0;
    int rnk_[6] = {};
    double tim_[6] = {};
    double mov_ = 0, spd_ = 0, spa_ = 0;
    double acl_ = 0, aca_ = 0, acb_ = 0, acf_ = 0;
    double hdl_ = 0, hdp_ = 0, crv_ = 0;
    double hit_ = 0, hip_ = 0, hir_ = 0;
    double yps_ = 0, ypt_ = 0;
    double cmd_[2] = {}, cmh_[2] = {}, cms_[2] = {};
    double cmm_ = 0;
    double pos_[3] = {};
    double poy_ = 0;
    double vec_[3] = {};
    double roz_ = 0;
    int ndg_ = 0;
    int spr_ = 0, chr_ = 0, smk_ = 0, smc_ = 0;
    double tmp_[8] = {};
    void (*prc_)() = nullptr;
    int prm_ = 0;
};
inline MyCar mycar;

void mystrt();
void myrest();
void mydriv();
void mydrivctrl();
void mydrivauto();
void mycrsh();
void myfall();
void mymove();
void mycamr();
void mychit();

inline void myinit() {
    mycar.mov_ = 1.0 / 2;
    mycar.spa_ = 1.0 / 256;
    mycar.aca_ = 1.0 / 32;
    mycar.acb_ = 1.0 / 48;
    mycar.acf_ = 1.0 / 96;
    mycar.hdp_ = 1.0 / 8;
    mycar.crv_ = mycar.hdp_ / 6;
    mycar.hip_ = 1.0 / 6;
    mycar.cmm_ = 1.0 / 64;
    mycar.spr_ = rival.spr_ + 2 * rival.n_;

    for (int i = 0; i < 6; i++) { mycar.rnk_[i] = 0; mycar.tim_[i] = 0; }
    mycar.cmd_[0] = 0; mycar.cmd_[1] = 0.25;
    mycar.cmh_[0] = 0; mycar.cmh_[1] = 0.3125;
    mycar.cms_[0] = 0; mycar.cms_[1] = 0;
    for (int i = 0; i < 3; i++) { mycar.pos_[i] = 0; mycar.vec_[i] = 0; }
    for (int i = 0; i < 8; i++) mycar.tmp_[i] = 0;
}

inline void myrset() {
    mycar.dst_ = course.dsl_ - (2 * 7 + 0.5);
    mycar.lpc_ = 0;
    mycar.lpd_ = mycar.dst_;
    course.arg_[0] = mycar.lpd_;
    csdidx();
    mycar.lpi_ = course.ret_[0];
    mycar.sde_ = 0.375;
    mycar.ndg_ = 0;
    int n = mycar.spr_;
    spchr(n + 0, 128, 112, 16, 12, 0x00);
    sphome(n + 0, 8, 6);
    spchr(n + 1, 0, 32, 16, 4, 0x00);
    sphome(n + 1, 8, -5);
    mycar.prc_ = mystrt;
    mycar.prm_ = 0;
}

inline void myloop() { mycar.prc_(); }

inline void mystrt() {
    if (mycar.prm_ == 0) {
        mycar.spd_ = 0;
        mycar.acl_ = 0;
        mycar.hdl_ = 0;
        mycar.hit_ = 0;
        mycar.poy_ = 0;
        mycar.roz_ = 0;
        mycar.tmp_[0] = 96;
        mycar.tmp_[1] = -8 / std::sqrt(8.0 * 8 + 48 * 48);
        mycar.chr_ = 0;
        mycar.smk_ = 0;
        mycar.smc_ = 0;
        app.zf_ = 128;
        mycar.prm_++;
    }
    mymove();
    if (mycar.tmp_[0] > 0) mycar.tmp_[0]--;
    double t = mycar.tmp_[0] / 96;
    mycar.cmd_[0] = (48 - mycar.cmd_[1]) * t + mycar.cmd_[1];
    mycar.cmh_[0] = (8 - mycar.cmh_[1]) * t + mycar.cmh_[1];
    mycar.cms_[0] = (mycar.tmp_[1] - mycar.cms_[1]) * t + mycar.cms_[1];
    double x = mycar.vec_[0];
    double z = mycar.vec_[2];
    double a = (M_PI * mycar.tmp_[0] + 32) / 32;
    double s = dsin(a / (2 * M_PI));
    double c = dcos(a / (2 * M_PI));
    mycar.vec_[0] = x * c - z * s;
    mycar.vec_[2] = x * s + z * c;
    mycamr();
    if (mycar.tmp_[0] == 0) {
        app.zf_ = 48;
        race.str_ = 1;
        mycar.prc_ = mydriv;
        mycar.prm_ = 0;
    }
}

inline void myrest() {
    if (mycar.prm_ == 0) {
        if (std::abs(mycar.sde_) < course.sdi_) mycar.sde_ = mycar.sde_;
        else mycar.sde_ = 0;
        mycar.hdl_ = 0;
        mycar.hit_ = 0;
        mycar.poy_ = 0;
        mycar.roz_ = 0;
        mycar.cmd_[0] = 0.5;
        mycar.cmh_[0] = 0.75;
        mycar.cms_[0] = 0;
        mycar.chr_ = 0;
        mycar.smc_ = 0;
        mycar.prm_++;
    }
    mymove();
    mycar.cmd_[0] -= (0.5 - mycar.cmd_[1]) / 16;
    mycar.cmh_[0] -= (0.75 - mycar.cmh_[1]) / 16;
    mycar.cmd_[0] = std::max(mycar.cmd_[1], mycar.cmd_[0]);
    mycar.cmh_[0] = std::max(mycar.cmh_[1], mycar.cmh_[0]);
    mycamr();
    mycar.smk_ = 0;
    if (mycar.cmd_[0] == mycar.cmd_[1]) {
        mycar.prc_ = mydriv;
        mycar.prm_ = 0;
    }
}

inline void mydriv() {
    if (mycar.prm_ == 0) {
        mycar.cmd_[0] = mycar.cmd_[1];
        mycar.cmh_[0] = mycar.cmh_[1];
        mycar.ndg_ = 96;
        mycar.prm_++;
    }
    if (app.debug_ != 0) {
        mycar.acl_ = std::min(1.0, mycar.acl_ + mycar.aca_) * race.flg_;
        mycar.spd_ = std::min(1.0, mycar.spd_ + mycar.spa_ * mycar.acl_);
        mydrivauto();
    } else {
        if (race.flg_ != 0) mydrivctrl();
        else mydrivauto();
    }
    if (std::abs(mycar.hdl_) > 0.75) sfx(21);
}

inline void mydrivctrl() {
    double s = 0;
    if (mycar.spd_ > 0) s = std::max(0.375, mycar.spd_);
    double h = mycar.hdp_ * app.sx_ * s * s;
    double c = mycar.crv_ * -course.crv_[mycar.lpi_] * mycar.spd_ * mycar.spd_;
    if (h == 0) {
        mycar.hdl_ -= std::min(mycar.hdp_, std::max(-mycar.hdp_, mycar.hdl_));
    } else {
        if (mycar.hdl_ * h > 0) mycar.hdl_ = std::min(1.0, std::max(-1.0, mycar.hdl_ * 1 + h));
        else mycar.hdl_ = std::min(1.0, std::max(-1.0, mycar.hdl_ * 0 + h));
    }
    mycar.sde_ = std::min(course.sdl_, std::max(-course.sdl_, mycar.sde_ + h + c));
    if ((app.bp_ & 16) != 0) {
        mycar.acl_ = std::min(1.0, mycar.acl_ + mycar.aca_);
        mycar.spd_ = std::min(1.0, mycar.spd_ + mycar.spa_ * mycar.acl_);
    } else {
        mycar.acl_ = std::max(0.0, mycar.acl_ - mycar.aca_);
        mycar.spd_ = std::max(0.0, mycar.spd_ - mycar.acb_);
    }
    if (std::abs(mycar.sde_) > course.sdi_ && mycar.spd_ > 0.5)
        mycar.spd_ = std::max(0.0, mycar.spd_ - mycar.acb_);
    mymove();
    mycamr();
    if (mycar.ndg_ > 0) mycar.ndg_--;
    mycar.chr_ = (int)std::ceil(std::abs(mycar.hdl_ * 2)) * (int)sgn(mycar.hdl_);
    if (mycar.chr_ < 0) mycar.chr_ += 16;
    mycar.smk_ = mycar.lpi_ & 1;
    if (std::abs(mycar.hdl_) >= 1) mycar.smk_++;
    if (!(mycar.spd_ > 0)) mycar.smk_ = 0;
    if (std::abs(mycar.sde_) > course.sdo_ && mycar.pos_[1] > course.fal_) {
        mycar.prc_ = myfall;
        mycar.prm_ = 0;
    } else if (mycar.ndg_ == 0) {
        mychit();
        if (mycar.hit_ != 0) {
            mycar.prc_ = mycrsh;
            mycar.prm_ = 0;
        }
    }
}

inline void mydrivauto() {
    // Not in the original mycar.lua (which just relaxes hdl_ straight to
    // 0 here) -- this function only ever runs in the app.debug_ autopilot
    // dev mode (see app.hpp's 'a' key toggle), so making it wobble the
    // wheel doesn't touch normal manual play at all. Requested so the
    // steering-warning buzz (sfx(21), mydrivctrl()'s `abs(hdl_) > 0.75`)
    // and the camera roll mycamr() derives from hdl_ are still
    // audible/visible while auto-driving instead of always sitting at 0.
    mycar.hdl_ = 0.9 * dsin(mycar.lpd_ * 0.05);
    mymove();
    mycamr();
    double h = 0;
    if (std::abs(course.crv_[mycar.lpi_]) > 1) h = sgn(course.crv_[mycar.lpi_]);
    mycar.chr_ = (int)h;
    if (h < 0) mycar.chr_ += 16;
    mycar.smk_ = mycar.lpi_ & 1;
    if (!(mycar.spd_ > 0)) mycar.smk_ = 0;
}

inline void mycrsh() {
    if (mycar.prm_ == 0) {
        mycar.hit_ = std::max(mycar.spa_, mycar.spd_ / 8) * mycar.hit_;
        mycar.hip_ = mycar.hit_ / 32;
        mycar.hir_ = 0;
        sfx(23);
        mycar.prm_++;
    }
    mycar.hdl_ -= std::min(mycar.hdp_, std::max(-mycar.hdp_, mycar.hdl_));
    mycar.acl_ = std::max(0.0, mycar.acl_ - mycar.aca_);
    mycar.spd_ = std::max(0.0, mycar.spd_ - mycar.acb_);
    double d = std::min(std::abs(mycar.hip_), std::abs(mycar.hit_)) * sgn(mycar.hit_);
    mycar.hit_ -= d;
    mycar.sde_ = std::min(course.sdl_, std::max(-course.sdl_, mycar.sde_ + mycar.hit_));
    mycar.hir_ += sgn(mycar.hdl_);
    if (mycar.hdl_ == 0) mycar.hir_ += 1;
    mycar.chr_ = (int)mycar.hir_ & 0x0e;
    mymove();
    mycamr();
    mycar.smc_++;
    mycar.smk_ = 1;
    if ((mycar.smc_ & 4) > 0) mycar.smk_++;
    if (std::abs(mycar.sde_) > course.sdo_ && mycar.pos_[1] > course.fal_) {
        mycar.prc_ = myfall;
        mycar.prm_ = 0;
    } else if (mycar.hit_ == 0) {
        if (std::abs(mycar.sde_) > course.sdo_) { mycar.prc_ = myrest; mycar.prm_ = 0; }
        else { mycar.prc_ = mydriv; mycar.prm_ = 0; }
    }
}

inline void myfall() {
    if (mycar.prm_ == 0) {
        mycar.yps_ = mycar.pos_[1];
        mycar.ypt_ = 0;
        mycar.prm_++;
    }
    mycar.hdl_ -= std::min(mycar.hdp_, std::max(-mycar.hdp_, mycar.hdl_));
    mycar.acl_ = std::max(0.0, mycar.acl_ - mycar.aca_);
    mycar.spd_ = std::max(0.0, mycar.spd_ - mycar.acf_);
    mycar.ypt_++;
    if (mycar.ypt_ > 16) mycar.ypt_++;
    double a = -(std::min(16.0, mycar.ypt_) * M_PI / 32);
    mycar.poy_ = mycar.yps_ * std::abs(dcos(a / (2 * M_PI)));
    mycar.roz_ = -(360 * mycar.ypt_ * sgn(mycar.sde_) / 32);
    if (mycar.ypt_ > 32) mycar.roz_ = 0;
    mymove();
    mycamr();
    mycar.smc_++;
    mycar.smk_ = 0;
    if (mycar.ypt_ > 16) {
        mycar.smk_++;
        if ((mycar.smc_ & 4) > 0) mycar.smk_++;
    }
    // mycar.lua calls sfx(22) here (at the fall's midpoint), but the user
    // confirmed on the original web version that falling is silent --
    // sfx(22) is actually the rival/tree collision impact sound (now
    // played from mychit() instead, see its comment). Left commented
    // rather than deleted so the deviation from the original source stays
    // visible: `if (mycar.ypt_ == 16) sfx(22);`
    if (mycar.ypt_ > 80) { mycar.prc_ = myrest; mycar.prm_ = 0; }
}

inline void mymove() {
    double m = mycar.mov_ * mycar.spd_;
    mycar.dst_ += m;
    mycar.lpd_ += m;
    if (mycar.lpd_ >= course.dsl_) {
        if (mycar.lpc_ <= race.lop_) {
            mycar.lpc_++;
            mycar.lpd_ -= course.dsl_;
        } else {
            mycar.dst_ -= course.dsl_;
            mycar.lpd_ -= course.dsl_;
        }
    }
    course.arg_[0] = mycar.lpd_;
    course.arg_[1] = mycar.sde_;
    csdxyz();
    mycar.lpi_ = (int)course.ret_[0];
    mycar.pos_[0] = course.ret_[1];
    if (mycar.poy_ == 0) mycar.pos_[1] = course.ret_[2] + mycar.poy_;
    else mycar.pos_[1] = mycar.poy_;
    mycar.pos_[2] = course.ret_[3];
    mycar.vec_[0] = course.ret_[4];
    mycar.vec_[1] = course.ret_[5];
    mycar.vec_[2] = course.ret_[6];
    if (mycar.poy_ == 0) mycar.cms_[1] = course.ret_[5];
    else mycar.cms_[1] = 0;
    double s = mycar.cms_[1] - mycar.cms_[0];
    if (std::abs(s) <= mycar.cmm_) mycar.cms_[0] = mycar.cms_[1];
    else mycar.cms_[0] += mycar.cmm_ * sgn(s);
}

inline void mycamr() {
    course.idx_ = mycar.lpi_ - (int)app.zn_;
    cam.os_ = mycar.cms_[0];
    cam.oc_ = 1 / std::sqrt(1 + cam.os_ * cam.os_);
    double d = app.zn_ + mycar.cmd_[0];
    double h = mycar.cmh_[0] - d * cam.os_;
    cam.ox_ = mycar.pos_[0] - d * mycar.vec_[0];
    cam.oy_ = mycar.pos_[1] + h;
    cam.oz_ = mycar.pos_[2] - d * mycar.vec_[2];
    cam.vx_ = mycar.vec_[0];
    cam.vy_ = mycar.vec_[1];
    cam.vz_ = mycar.vec_[2];
    double u = course.ret_[1] - cam.ox_;
    double v = course.ret_[2] - cam.oy_;
    double w = course.ret_[3] - cam.oz_;
    double x = u * cam.vz_ - w * cam.vx_;
    double y = -v;
    double z = u * cam.vx_ + w * cam.vz_;
    double s = app.or_ / (z / 2);
    cam.rx_ = x * s + app.ox_;
    cam.ry_ = y * s + app.oy_;
    cam.ra_ = -(15 * std::min(1.0, std::abs(mycar.hdl_)) * sgn(mycar.hdl_));
    cam.rs_ = dsin((cam.ra_ * M_PI / 180) / (2 * M_PI));
    cam.rc_ = dcos((cam.ra_ * M_PI / 180) / (2 * M_PI));
}

inline void mychit() {
    double w = car.szw_ * 0.5;
    mycar.hit_ = 0;
    for (int i = 0; i <= rival.n_ - 1; i++) {
        double x = mycar.sde_ - rival.sde_[i];
        double z = mycar.lpd_ - rival.lpd_[i];
        if (std::abs(x) < w && z <= 0 && z > -car.szw_) {
            if (x == 0) mycar.hit_ = sgn(x + 1);
            else mycar.hit_ = sgn(x + 0);
            break;
        }
    }
    if (mycar.hit_ == 0) {
        if (std::abs(mycar.sde_) >= obj.sde_ - car.szw_ / 2) {
            int f = mycar.sde_ > 0 ? 1 : 0;
            if (obj.hit_[mycar.lpi_][1 + f] > 0) mycar.hit_ = sgn(-mycar.sde_);
        }
    }
    // Not in the original mycar.lua (which has no sfx() call here at all)
    // -- added per direct user confirmation from the original web version:
    // the impact sound on hitting a rival car or roadside object (tree/
    // cone) is sfx(22), not sfx(23) (which mycrsh() already plays once
    // when the crash *state* is entered) or sfx(21) (steering warning).
    if (mycar.hit_ != 0) sfx(22);
}

inline void mydraw() {
    int n = mycar.spr_;
    double u = mycar.pos_[0] - cam.ox_;
    double v = mycar.pos_[1] - cam.oy_ + car.szh_ / 2;
    double w = mycar.pos_[2] - cam.oz_;
    double x = u * cam.vz_ - w * cam.vx_;
    v = -v;
    w = u * cam.vx_ + w * cam.vz_;
    double y = v * cam.oc_ + w * cam.os_;
    double z = -v * cam.os_ + w * cam.oc_;
    if (z >= app.zn_ && z <= app.zf_) {
        double s = app.or_ / (z / 2);
        u = x * s + app.ox_ - cam.rx_;
        v = y * s + app.oy_ - cam.ry_;
        x = u * cam.rc_ - v * cam.rs_ + cam.rx_;
        y = u * cam.rs_ + v * cam.rc_ + cam.ry_;
        if (x > -(64 + 40) && x < (464 + 40) && y > -(64 + 15) && y < (304 + 15)) {
            int cu = car.chr_[mycar.chr_][0];
            int cv = car.chr_[mycar.chr_][1] + 112;
            int a = car.chr_[mycar.chr_][2];
            z = std::min(1024.0, z * 16);
            s = s * car.szw_ / 16;
            spchr(n + 0, cu, cv, 16, 12, 0x01 + a);
            spofs(n + 0, x, y, z);
            sprot(n + 0, mycar.roz_ + cam.ra_);
            spscale(n + 0, s, s);
            spshow(n + 0);
            spchr(n + 1, mycar.smk_ * 16, 32, 16, 4, 0x01);
            spofs(n + 1, x, y, z - 1);
            sprot(n + 1, mycar.roz_ + cam.ra_);
            spscale(n + 1, s, s);
            spshow(n + 1);
        } else {
            sphide(n + 0);
            sphide(n + 1);
        }
    } else {
        sphide(n + 0);
        sphide(n + 1);
    }
}

} // namespace db
