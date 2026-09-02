// course.hpp : course (closed-loop periodic natural cubic spline track)
//
// Direct port of course.lua. The Lua source consistently writes every array
// access as `EXPR + 1` (manual 1-based conversion of an otherwise 0-based
// algorithm), so this port mechanically drops every trailing `+ 1` and
// keeps loop bounds identical -- indices below match the original 1:1.
//
// cstabl()'s forward-sweep step (search "NOTE: as written" below) looks
// like it re-uses the same index on both sides of a subtraction, which is
// unusual for a textbook Thomas-algorithm sweep. Verified twice against
// the original source (not a transcription slip); ported byte-faithful
// rather than "corrected" against a guess, since this is a periodic/cyclic
// tridiagonal solver where the textbook single-sweep shape doesn't
// necessarily apply. If a rendered course ever looks visibly kinked or
// self-intersecting, re-check this block first.
#pragma once
#include "db.hpp"
#include "sp.hpp"
#include "car.hpp"
#include "race.hpp"
#include "cam.hpp"
#include <cmath>
#include <cstdio>
#include <algorithm>

namespace db {

namespace detail {
// stage track control points (x,y,z), terminated by {-1,-1,-1}
inline const double kStageData[5][32][3] = {
    { // stage 1
        {10,0,0}, {10,0,1}, {10,0,31}, {10,0,32}, {0,4,42}, {-10,4,32}, {-10,4,31},
        {-10,1,16}, {-10,2,1}, {-10,2,0}, {-10,2,-1}, {-10,0,-31}, {-10,0,-32},
        {0,0,-42}, {10,0,-32}, {10,0,-31}, {10,0,-1}, {10,0,0}, {-1,-1,-1},
    },
    { // stage 2
        {8,0,24}, {9,0,24}, {15,0,24}, {16,0,24}, {28,0,12}, {16,0,0}, {15,0,0},
        {10,1,0}, {0,2,0}, {-10,1,0}, {-15,0,0}, {-16,0,0}, {-28,0,-12}, {-16,0,-24},
        {-15,0,-24}, {0,0,-24}, {11,0,-24}, {12,0,-24}, {20,0,-16}, {18,0,-10},
        {0,0,0}, {-18,0,10}, {-20,0,16}, {-12,0,24}, {-8,0,24}, {0,0,24}, {7,0,24},
        {8,0,24}, {-1,-1,-1},
    },
    { // stage 3
        {0,0,5}, {0,0,6}, {0,0,7}, {0,0,26}, {0,0,27}, {0,0,28}, {10,1,38}, {20,2,28},
        {20,2,27}, {10,2,12}, {0,2,0}, {0,2,-8}, {-16,2,-29}, {-16,2,-30}, {-6,1,-40},
        {4,0,-30}, {4,0,-29}, {0,0,-11}, {0,0,-10}, {0,0,-9}, {0,0,3}, {0,0,4},
        {0,0,5}, {-1,-1,-1},
    },
    { // stage 4
        {-9,0,8}, {-9,0,9}, {-9,0,31}, {-9,0,32}, {0,0,41}, {9,0,32}, {9,0,31},
        {9,0,16}, {9,3,1}, {9,3,0}, {9,3,-1}, {9,1,-21}, {9,1,-22}, {-9,2,-41},
        {-9,2,-42}, {0,1,-51}, {9,0,-42}, {9,0,-41}, {0,0,-32}, {-9,0,-22}, {-9,0,-21},
        {-9,0,0}, {-9,0,7}, {-9,0,8}, {-1,-1,-1},
    },
    { // stage 5
        {24,0,8}, {24,0,9}, {24,0,15}, {24,0,16}, {16,0,24}, {8,0,16}, {8,0,15},
        {8,3,4}, {8,0,-7}, {8,0,-8}, {-8,0,-24}, {-24,0,-8}, {-24,0,-7}, {-24,2,4},
        {-24,0,15}, {-24,0,16}, {-16,0,24}, {-8,0,16}, {-8,0,15}, {-8,3,0}, {-8,2,-7},
        {-8,2,-8}, {8,1,-24}, {24,0,-8}, {24,0,-7}, {24,0,0}, {24,0,7}, {24,0,8},
        {-1,-1,-1},
    },
};
} // namespace detail

struct Course {
    int n_ = 0;
    int idx_ = 0;
    double pos_[3][256][3] = {};
    double vec_[3][256][3] = {};
    double dst_[256][2] = {};
    double dsl_ = 0;
    double crv_[256] = {};
    double inc_[256] = {};
    double sdi_ = 0, sdo_ = 0, sdl_ = 0, fal_ = 0;
    int spr_ = 0, spn_ = 0;
    int sln_ = 0;
    double slt_[40] = {};
    double slp_[40][3] = {};
    double sla_[40][3] = {};
    double slh_[40] = {}, sld_[40] = {}, slw_[40] = {};
    double arg_[8] = {}, ret_[8] = {};
};
inline Course course;

inline void csinit() {
    course.n_ = 0;
    course.sdi_ = 0.875;
    course.sdo_ = 1.125;
    course.sdl_ = 1.5;
    course.fal_ = 0.25;
    course.spr_ = 0;
    course.spn_ = 256;
}

inline void csread() {
    const auto& data = detail::kStageData[race.stg_ - 1];
    course.sln_ = 0;
    int i = 0;
    do {
        course.slp_[course.sln_][0] = data[i][0];
        course.slp_[course.sln_][1] = data[i][1];
        course.slp_[course.sln_][2] = data[i][2];
        i++;
        course.sln_++;
    } while (course.slp_[course.sln_ - 1][1] >= 0);
    course.sln_ -= 2;
}

inline void cstabl() {
    int n = course.sln_;
    course.slt_[0] = 0;
    for (int i = 1; i <= n; i++) {
        double x = course.slp_[i][0] - course.slp_[i - 1][0];
        double y = course.slp_[i][1] - course.slp_[i - 1][1];
        double z = course.slp_[i][2] - course.slp_[i - 1][2];
        course.slt_[i] = course.slt_[i - 1] + std::sqrt(x * x + y * y + z * z);
    }
    for (int i = 1; i <= n; i++) course.slt_[i] = course.slt_[i] / course.slt_[n];

    for (int p = 0; p <= 2; p++) {
        for (int i = 0; i <= n - 1; i++) {
            course.slh_[i] = course.slt_[i + 1] - course.slt_[i];
            course.slw_[i] = course.slp_[i + 1][p] - course.slp_[i][p];
            course.slw_[i] = course.slw_[i] / course.slh_[i];
        }
        course.slw_[n] = course.slw_[0];
        for (int i = 1; i <= n - 1; i++)
            course.sld_[i] = 2 * (course.slt_[i + 1] - course.slt_[i - 1]);
        course.sld_[n] = 2 * (course.slh_[n - 1] + course.slh_[0]);
        for (int i = 1; i <= n; i++)
            course.sla_[i][p] = course.slw_[i] - course.slw_[i - 1];
        course.slw_[1] = course.slh_[0];
        course.slw_[n - 1] = course.slh_[n - 1];
        course.slw_[n] = course.sld_[n];
        for (int i = 2; i <= n - 2; i++) course.slw_[i] = 0;

        // NOTE: as written (see file header) -- both operands of the
        // subtraction on sla_'s line below share the same index i+1.
        for (int i = 1; i <= n - 1; i++) {
            double t = course.slw_[i] / course.sld_[i];
            course.sla_[i + 1][p] = course.sla_[i + 1][p] - course.sla_[i + 1][p] * t;
            course.sld_[i + 1] = course.sld_[i + 1] - course.slh_[i] * t;
            course.slw_[i + 1] = course.slw_[i + 1] - course.slw_[i] * t;
        }
        course.slw_[0] = course.slw_[n];
        course.sla_[0][p] = course.sla_[n][p];
        for (int i = n - 2; i >= 0; i--) {
            double t = course.slh_[i] / course.sld_[i + 1];
            course.sla_[i][p] = course.sla_[i][p] - course.sla_[i + 1][p] * t;
            course.slw_[i] = course.slw_[i] - course.slw_[i + 1] * t;
        }
        double t = course.sla_[0][p] / course.slw_[0];
        course.sla_[0][p] = t;
        course.sla_[n][p] = t;
        for (int i = 1; i <= n - 1; i++) {
            course.sla_[i][p] = course.sla_[i][p] - course.slw_[i] * t;
            course.sla_[i][p] = course.sla_[i][p] / course.sld_[i];
        }
    }
}

inline void csspln() {
    for (int c = 0; c <= course.n_ - 1; c++) {
        double t = (double)c / course.n_;
        for (int p = 0; p <= 2; p++) {
            double r = course.slt_[course.sln_] - course.slt_[0];
            while (t > course.slt_[course.sln_]) t -= r;
            while (t < course.slt_[0]) t += r;
            int i = 0, j = course.sln_;
            while (i < j) {
                int k = (i + j) / 2;
                if (course.slt_[k] < t) i = k + 1;
                else j = k;
            }
            if (i > 0) i--;
            double h = course.slt_[i + 1] - course.slt_[i];
            double d = t - course.slt_[i];
            double u = (course.sla_[i + 1][p] - course.sla_[i][p]) * d / h;
            u = (u + course.sla_[i][p] * 3) * d;
            double v = (course.slp_[i + 1][p] - course.slp_[i][p]) / h;
            v = v - (course.sla_[i][p] * 2 + course.sla_[i + 1][p]) * h;
            double w = (u + v) * d + course.slp_[i][p];
            course.pos_[0][c][p] = w;
        }
    }
    for (int i = 0; i <= course.n_ - 1; i++)
        if (course.pos_[0][i][1] < 0.075) course.pos_[0][i][1] = 0;
}

inline void csmake() {
    csread();
    cstabl();
    course.n_ = course.spn_;
    csspln();
    double x = course.pos_[0][1][0] - course.pos_[0][0][0];
    double y = course.pos_[0][1][1] - course.pos_[0][0][1];
    double z = course.pos_[0][1][2] - course.pos_[0][0][2];
    course.n_ = (int)std::floor(course.spn_ * std::sqrt(x * x + y * y + z * z));
    if (course.n_ > course.spn_) {
        std::fprintf(stderr, "csmake error - could not make course.\n");
        course.n_ = course.spn_;
    }
    csspln();
    for (int i = 0; i <= course.n_ - 1; i++) {
        int j = (i + 1) % course.n_;
        course.vec_[0][i][0] = course.pos_[0][j][0] - course.pos_[0][i][0];
        course.vec_[0][i][1] = course.pos_[0][j][1] - course.pos_[0][i][1];
        course.vec_[0][i][2] = course.pos_[0][j][2] - course.pos_[0][i][2];
    }
    for (int i = 0; i <= course.n_ - 1; i++) {
        int j = (i + course.n_ - 1) % course.n_;
        double x2 = course.vec_[0][i][0] + course.vec_[0][j][0];
        double z2 = course.vec_[0][i][2] + course.vec_[0][j][2];
        double n2 = std::sqrt(x2 * x2 + z2 * z2);
        x2 /= n2; z2 /= n2;
        course.pos_[1][i][0] = course.pos_[0][i][0] - z2;
        course.pos_[1][i][1] = course.pos_[0][i][1];
        course.pos_[1][i][2] = course.pos_[0][i][2] + x2;
        course.pos_[2][i][0] = course.pos_[0][i][0] + z2;
        course.pos_[2][i][1] = course.pos_[0][i][1];
        course.pos_[2][i][2] = course.pos_[0][i][2] - x2;
    }
    for (int i = 0; i <= course.n_ - 1; i++) {
        int j = (i + 1) % course.n_;
        course.vec_[1][i][0] = course.pos_[1][j][0] - course.pos_[1][i][0];
        course.vec_[1][i][1] = course.pos_[1][j][1] - course.pos_[1][i][1];
        course.vec_[1][i][2] = course.pos_[1][j][2] - course.pos_[1][i][2];
        course.vec_[2][i][0] = course.pos_[2][j][0] - course.pos_[2][i][0];
        course.vec_[2][i][1] = course.pos_[2][j][1] - course.pos_[2][i][1];
        course.vec_[2][i][2] = course.pos_[2][j][2] - course.pos_[2][i][2];
        double x2 = course.vec_[0][i][0];
        double y2 = course.vec_[0][i][1];
        double z2 = course.vec_[0][i][2];
        course.dst_[i][0] = std::sqrt(x2 * x2 + z2 * z2);
        course.dst_[i][1] = course.dst_[i][0];
        if (i > 0) course.dst_[i][1] += course.dst_[i - 1][1];
    }
    course.dsl_ = course.dst_[course.n_ - 1][1];
    for (int i = 0; i <= course.n_ - 1; i++) {
        int j = (i + course.n_ - 1) % course.n_;
        double u = course.vec_[0][i][0];
        double v = course.vec_[0][i][2];
        double s = course.vec_[0][j][0];
        double t = course.vec_[0][j][2];
        double c = (u * s + v * t) / (std::sqrt(u * u + v * v) * std::sqrt(s * s + t * t));
        c = std::min(1.0, std::max(-1.0, c));
        course.crv_[i] = std::acos(c) * 180.0 / M_PI * -sgn(s * v - t * u);
    }
    for (int i = 0; i <= course.n_ - 1; i++) {
        int j = (i + 1) % course.n_;
        double u = course.pos_[0][j][0] - course.pos_[0][i][0];
        double v = course.pos_[0][j][1] - course.pos_[0][i][1];
        double w = course.pos_[0][j][2] - course.pos_[0][i][2];
        course.inc_[i] = v / std::sqrt(u * u + w * w);
    }
    for (int n = course.spr_; n <= course.spr_ + course.spn_ - 1; n++) {
        spchr(n, 0, 128, 64, 8, 0x00);
        sphome(n, 32, 0);
    }
}

inline void csdidx() {
    int i = (int)std::floor(course.arg_[0] / (course.dsl_ / course.n_));
    course.ret_[0] = std::min(course.n_ - 1, i);
}

inline void csdxyz() {
    csdidx();
    double d = course.arg_[0];
    int i = std::min(course.n_ - 1, (int)std::floor(d / (course.dsl_ / course.n_)));
    course.ret_[0] = i;
    if (i > 0) d -= course.dst_[i - 1][1];
    double t = d / course.dst_[i][0];
    for (int j = 0; j <= 2; j++) {
        double u = course.pos_[1][i][j] + course.vec_[1][i][j] * t;
        double v = course.pos_[2][i][j] + course.vec_[2][i][j] * t;
        course.ret_[1 + j] = ((u + v) + (v - u) * course.arg_[1]) / 2;
        course.ret_[4 + j] = v - u;
    }
    double x = course.ret_[4];
    double z = course.ret_[6];
    double n = std::sqrt(x * x + z * z);
    course.ret_[4] = -z / n;
    course.ret_[5] = course.inc_[i];
    course.ret_[6] = x / n;
}

inline void csloop() {}

inline void csdraw() {
    int n = course.spr_;
    int i = (course.idx_ + course.n_) % course.n_;
    int m = i + course.n_;
    while (i < m) {
        int j = i % course.n_;
        double u = course.pos_[0][j][0] - cam.ox_;
        double v = course.pos_[0][j][1] - cam.oy_;
        double w = course.pos_[0][j][2] - cam.oz_;
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
                s = s / 32;
                v = 128;
                if (course.pos_[0][j][1] > 0) v += 8;
                spchr(n, 0, (int)v, 64, 8, 0x01);
                spofs(n, x, y, std::min(1024.0, z * 16));
                sprot(n, cam.ra_);
                spscale(n, s, s);
                spshow(n);
                n++;
            }
        }
        i++;
    }
    while (n < course.spr_ + course.spn_ - 1) {
        sphide(n);
        n++;
    }
}

} // namespace db
