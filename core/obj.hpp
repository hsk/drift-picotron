// obj.hpp : roadside objects (start/goal banner, trees, cones)
#pragma once
#include "db.hpp"
#include "sp.hpp"
#include "course.hpp"
#include "cam.hpp"
#include <cmath>
#include <algorithm>

namespace db {

struct Obj {
    int n_ = 0;
    int typ_[128] = {};
    double pos_[128][3] = {};
    int hit_[256][3] = {};
    double sde_ = 0;
    int spr_ = 0, spn_ = 0;
    double siz_ = 0;
};
inline Obj obj;

inline void obinit() {
    obj.sde_ = 1.5;
    obj.spr_ = course.spn_;
    obj.spn_ = 384 - course.spn_;
    obj.siz_ = 1;
}

inline void obmake() {
    obj.n_ = 0;
    for (int i = 0; i <= 255; i++) {
        obj.hit_[i][0] = 0;
        obj.hit_[i][1] = 0;
        obj.hit_[i][2] = 0;
    }
    obj.typ_[obj.n_] = 3;
    obj.pos_[obj.n_][0] = course.pos_[0][0][0];
    obj.pos_[obj.n_][1] = course.pos_[0][0][1];
    obj.pos_[obj.n_][2] = course.pos_[0][0][2];
    int n = obj.spr_ + obj.n_;
    spchr(n, 0, 96, 48, 32, 0x00);
    sphome(n, 24, 31);
    obj.n_++;

    for (int s = 1; s <= 2; s++) {
        double x = course.pos_[s][0][0] - course.pos_[0][0][0];
        double y = course.pos_[s][0][1] - course.pos_[0][0][1];
        double z = course.pos_[s][0][2] - course.pos_[0][0][2];
        obj.typ_[obj.n_] = 1;
        obj.pos_[obj.n_][0] = course.pos_[0][0][0] + x * obj.sde_;
        obj.pos_[obj.n_][1] = course.pos_[0][0][1] + y * obj.sde_;
        obj.pos_[obj.n_][2] = course.pos_[0][0][2] + z * obj.sde_;
        obj.hit_[0][s] = 1;
        n = obj.spr_ + obj.n_;
        spchr(n, 24 * obj.typ_[obj.n_], 64, 24, 32, (s - 1) << 3);
        sphome(n, 12, 31);
        obj.n_++;
    }

    int c = 0, a = 0;
    for (int i = 1; i <= course.n_ - 1; i++) {
        if ((i % 2) == 0) {
            int j = (i + 2) % course.n_;
            if (std::abs(course.crv_[j]) > 0.5) {
                if (course.pos_[0][i][1] < course.fal_) {
                    if (a == 0) {
                        c = 3;
                        a = 1;
                        if (course.crv_[j] < 0) a++;
                    }
                }
            } else {
                if (c == 0) a = 0;
            }
            int t = -1;
            int s = 0;
            if (c > 0) {
                t = 2;
                s = a;
                c--;
            } else if (course.pos_[0][i][1] < course.fal_) {
                t = 0;
                s = 1;
                if ((i % 4) == 0) s++;
            }
            if (t >= 0) {
                double x = course.pos_[s][i][0] - course.pos_[0][i][0];
                double z = course.pos_[s][i][2] - course.pos_[0][i][2];
                obj.typ_[obj.n_] = t;
                obj.pos_[obj.n_][0] = course.pos_[0][i][0] + x * obj.sde_;
                obj.pos_[obj.n_][1] = 0;
                obj.pos_[obj.n_][2] = course.pos_[0][i][2] + z * obj.sde_;
                obj.hit_[i][s] = 1;
                n = obj.spr_ + obj.n_;
                spchr(n, 24 * obj.typ_[obj.n_], 64, 24, 32, (s - 1) << 3);
                sphome(n, 12, 31);
                obj.n_++;
            }
        }
    }
    for (int nn = obj.spr_ + obj.n_; nn <= obj.spr_ + obj.spn_ - 1; nn++)
        sphide(nn);
}

inline void obloop() {}

inline void obdraw() {
    for (int i = 0; i <= obj.n_ - 1; i++) {
        int n = obj.spr_ + i;
        double u = obj.pos_[i][0] - cam.ox_;
        double v = obj.pos_[i][1] - cam.oy_;
        double w = obj.pos_[i][2] - cam.oz_;
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
                s = s * obj.siz_ / 24;
                spofs(n, x, y, std::min(1024.0, z * 16));
                sprot(n, cam.ra_);
                spscale(n, s, s);
                spshow(n);
            } else {
                sphide(n);
            }
        } else {
            sphide(n);
        }
    }
}

} // namespace db
