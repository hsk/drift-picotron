// rival.hpp : rival cars (AI)
//
// See db.hpp for why the Rival (and Race) state structs live there instead
// of here -- this file and race.hpp reference each other's fields and
// would otherwise form an #include cycle.
#pragma once
#include "db.hpp"
#include "sp.hpp"
#include "car.hpp"
#include "course.hpp"
#include "cam.hpp"
#include <cmath>
#include <algorithm>

namespace db {

inline void rvinit() {
    rival.n_ = 7;
    rival.hdp_ = 1.0 / 32;
    rival.spr_ = 384;
    for (int i = 0; i < 7; i++) {
        rival.dst_[i] = 0;
        rival.lpc_[i] = 0;
        rival.lpd_[i] = 0;
        rival.lpi_[i] = 0;
        rival.sde_[i] = 0;
        rival.rnk_[i][0] = 0;
        rival.rnk_[i][1] = 0;
        rival.mov_[i][0] = 0;
        rival.mov_[i][1] = 15.0 / 32 - i / 32.0;
        rival.spd_[i] = 0;
        rival.spa_[i] = 1.0 / 256;
        rival.hdl_[i] = 0;
        rival.hds_[i] = 0;
        rival.pos_[i][0] = rival.pos_[i][1] = rival.pos_[i][2] = 0;
        rival.smk_[i] = 0;
    }
}

inline void rvrset() {
    for (int i = 0; i <= rival.n_ - 1; i++) {
        rival.dst_[i] = course.dsl_ - (2 * i + 0.5);
        rival.lpc_[i] = 0;
        rival.lpd_[i] = rival.dst_[i];
        course.arg_[0] = rival.lpd_[i];
        csdidx();
        rival.lpi_[i] = (int)course.ret_[0];
        rival.sde_[i] = 0.375 * ((i & 1) * 2 - 1);
        rival.mov_[i][0] = rival.mov_[i][1];
        rival.spd_[i] = 0;
        rival.hdl_[i] = 0;
        rival.hds_[i] = rival.sde_[i];
        rival.smk_[i] = 0;
        int n = rival.spr_ + i * 2;
        spchr(n + 0, 128, i * 16, 16, 12, 0x00);
        sphome(n + 0, 8, 12);
        spchr(n + 1, 0, 32, 16, 4, 0x00);
        sphome(n + 1, 8, 1);
    }
}

inline void rvloop() {
    for (int i = 0; i <= rival.n_ - 1; i++) {
        rival.spd_[i] += rival.spa_[i] * race.flg_;
        rival.spd_[i] = std::min(1.0, rival.spd_[i]);
        double m = rival.mov_[i][0] * rival.spd_[i];
        rival.dst_[i] += m;
        rival.lpd_[i] += m;
        if (rival.lpd_[i] >= course.dsl_) {
            if (rival.lpc_[i] <= race.lop_) {
                rival.lpc_[i]++;
                rival.lpd_[i] -= course.dsl_;
            } else {
                rival.dst_[i] -= course.dsl_;
                rival.lpd_[i] -= course.dsl_;
            }
        }
        if (rival.hdl_[i] <= 0) {
            double d = rival.hds_[i] - rival.sde_[i];
            rival.sde_[i] += std::min(rival.hdp_, std::max(-rival.hdp_, d));
            if (rival.sde_[i] == rival.hds_[i]) {
                rival.hdl_[i] = 384 + std::floor(rnd(384));
                rival.hds_[i] = 0.5 * rnd() * sgn(-rival.sde_[i]);
                double s = rnd() * (1.0 / 24) * sgn(rival.rnk_[i][0]);
                rival.mov_[i][0] = rival.mov_[i][1] + s;
            }
        } else {
            rival.hdl_[i] -= 1;
        }
        course.arg_[0] = rival.lpd_[i];
        course.arg_[1] = rival.sde_[i];
        csdxyz();
        rival.lpi_[i] = (int)course.ret_[0];
        rival.pos_[i][0] = course.ret_[1];
        rival.pos_[i][1] = course.ret_[2];
        rival.pos_[i][2] = course.ret_[3];
        int s = rival.lpi_[i] & 1;
        rival.smk_[i] = s;
    }
}

inline void rvdraw() {
    for (int i = 0; i <= rival.n_ - 1; i++) {
        int n = rival.spr_ + i * 2;
        double u = rival.pos_[i][0] - cam.ox_;
        double v = rival.pos_[i][1] - cam.oy_;
        double w = rival.pos_[i][2] - cam.oz_;
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
                int cu = car.chr_[0][0];
                int cv = car.chr_[0][1] + i * 16;
                int a = car.chr_[0][2];
                z = std::min(1024.0, z * 16);
                s = s * car.szw_ / 16;
                spchr(n + 0, cu, cv, 16, 12, 0x01 + a);
                spofs(n + 0, x, y, z);
                sprot(n + 0, cam.ra_);
                spscale(n + 0, s, s);
                spshow(n + 0);
                spchr(n + 1, rival.smk_[i] * 16, 32, 16, 4, 0x01);
                spofs(n + 1, x, y, z - 1);
                sprot(n + 1, cam.ra_);
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
}

} // namespace db
