// sp.hpp : sprite library
#pragma once
#include "db.hpp"
#include <cmath>

namespace db {

// initialize sprite
inline void sprset() {
    sp.assign(512, Sp{});
    for (auto& s : sp) s.page = &app.gpage2_;
}
inline void spinit() { sprset(); }

// set sprite character
inline void spchr(int n, int u, int v, int w, int h, int a) {
    sp[n].chr_u = u; sp[n].chr_v = v; sp[n].chr_w = w; sp[n].chr_h = h; sp[n].chr_a = a;
}
// set sprite offset
inline void spofs(int n, double x, double y, double z) {
    sp[n].ofs_x = x; sp[n].ofs_y = y; sp[n].ofs_z = z;
}
// set sprite home
inline void sphome(int n, double x, double y) { sp[n].home_x = x; sp[n].home_y = y; }
// set sprite scale
inline void spscale(int n, double x, double y) { sp[n].scale_x = x; sp[n].scale_y = y; }
// set sprite rotate
inline void sprot(int n, double r) { sp[n].rot = r / 360.0; }
// set sprite color
inline void spcolor(int n, int c) { sp[n].color = c; }
// set sprite page
inline void sppage(int n, Userdata* p) { sp[n].page = p; }
// show / hide sprite
inline void spshow(int n) { sp[n].show = true; }
inline void sphide(int n) { sp[n].show = false; }

// draw scale sprite
inline void spsspr(Userdata* sprite, int u, int v, int w, int h, double x, double y,
                    double offset_x, double offset_y, double scale_x, double scale_y, int a) {
    bool flip_x = (a & 0b00001000) != 0;
    sspr(sprite, u, v, w, h,
         x - offset_x * scale_x, y - offset_y * scale_y,
         w * scale_x, h * scale_y, flip_x);
}

struct UV { double u, v; };
struct XY { double x, y; };

// draw scale & rotate sprite
//
// NOTE: uv/xy/id below keep Lua's 1-based indexing (index 0 is an unused
// placeholder) to mirror the original geometry code exactly -- this was
// ported once already (Lua -> Python) and the 1-based/0-based mismatches
// found there are already fixed; don't re-introduce them here.
inline void sprspr(Userdata* sprite, int u, int v, int w, int h, double x, double y,
                    double offset_x, double offset_y, double scale_x, double scale_y,
                    double rotate, int a) {
    // [1] - [2]
    //  |     |
    // [3] - [4]
    UV uv[5] = {
        {0,0},
        {(double)u, (double)v},
        {(double)(u + w - 1), (double)v},
        {(double)u, (double)(v + h - 1)},
        {(double)(u + w - 1), (double)(v + h - 1)},
    };
    if ((a & 0b00001000) != 0) {
        std::swap(uv[1].u, uv[2].u);
        std::swap(uv[3].u, uv[4].u);
    }

    double sin_r = dsin(rotate);
    double cos_r = dcos(rotate);
    double x0 = -offset_x, y0 = -offset_y;
    double x1 = x0 + w, y1 = y0 + h;
    x0 = x0 * scale_x; y0 = y0 * scale_y;
    x1 = x1 * scale_x - 1; y1 = y1 * scale_y - 1;
    XY xy[5] = {
        {0,0},
        { dceil((x0 * cos_r - y0 * sin_r) + x), dceil((x0 * sin_r + y0 * cos_r) + y) },
        { dceil((x1 * cos_r - y0 * sin_r) + x), dceil((x1 * sin_r + y0 * cos_r) + y) },
        { dceil((x0 * cos_r - y1 * sin_r) + x), dceil((x0 * sin_r + y1 * cos_r) + y) },
        { dceil((x1 * cos_r - y1 * sin_r) + x), dceil((x1 * sin_r + y1 * cos_r) + y) },
    };

    int id[5] = {0,0,0,0,0};
    if (xy[1].y <= xy[2].y) {
        if (xy[1].y < xy[3].y) { id[1]=1; id[2]=2; id[3]=3; id[4]=4; }
        else if (xy[3].y < xy[4].y) { id[1]=3; id[2]=1; id[3]=4; id[4]=2; }
        else { id[1]=4; id[2]=3; id[3]=2; id[4]=1; }
    } else {
        if (xy[2].y <= xy[4].y) { id[1]=2; id[2]=4; id[3]=1; id[4]=3; }
        else { id[1]=4; id[2]=3; id[3]=2; id[4]=1; }
    }

    if (xy[id[1]].y == xy[id[2]].y) {
        double lu = uv[id[3]].u - uv[id[1]].u;
        double lv = uv[id[3]].v - uv[id[1]].v;
        double ru = uv[id[4]].u - uv[id[2]].u;
        double rv = uv[id[4]].v - uv[id[2]].v;
        double d = xy[id[3]].y - xy[id[1]].y;
        int di = (int)d;
        for (int i = 0; i <= di; i++) {
            double x0d = xy[id[1]].x;
            double x1d = xy[id[4]].x;
            double yy = xy[id[1]].y + i;
            if (x0d < 480 && x1d >= 0 && yy >= 0 && yy < 270) {
                double t = d != 0 ? (double)i / d : 0;
                tline3d(sprite, x0d, yy, x1d, yy,
                        lu * t + uv[id[1]].u, lv * t + uv[id[1]].v,
                        ru * t + uv[id[2]].u, rv * t + uv[id[2]].v, 1, 1);
            }
        }
    } else {
        int ls = id[1], le = id[3];
        double lu = uv[le].u - uv[ls].u, lv = uv[le].v - uv[ls].v;
        double lx = xy[le].x - xy[ls].x, ly = xy[le].y - xy[ls].y;
        double li = 0;
        int rs = id[1], re = id[2];
        double ru = uv[re].u - uv[rs].u, rv = uv[re].v - uv[rs].v;
        double rx = xy[re].x - xy[rs].x, ry = xy[re].y - xy[rs].y;
        double ri = 0;
        double d = xy[id[4]].y - xy[id[1]].y;
        int di = (int)d;
        for (int i = 0; i <= di; i++) {
            double lt = ly != 0 ? li / ly : 0;
            double rt = ry != 0 ? ri / ry : 0;
            double x0d = lx * lt + xy[ls].x;
            double x1d = rx * rt + xy[rs].x;
            double yy = li + xy[ls].y;
            if (x0d < 480 && x1d >= 0 && yy >= 0 && yy < 270) {
                tline3d(sprite, x0d, yy, x1d, yy,
                        lu * lt + uv[ls].u, lv * lt + uv[ls].v,
                        ru * rt + uv[rs].u, rv * rt + uv[rs].v, 1, 1);
            }
            li += 1;
            if (li > ly) {
                ls = id[3]; le = id[4];
                lu = uv[le].u - uv[ls].u; lv = uv[le].v - uv[ls].v;
                lx = xy[le].x - xy[ls].x; ly = xy[le].y - xy[ls].y;
                li = 0;
            }
            ri += 1;
            if (ri > ry) {
                rs = id[2]; re = id[4];
                ru = uv[re].u - uv[rs].u; rv = uv[re].v - uv[rs].v;
                rx = xy[re].x - xy[rs].x; ry = xy[re].y - xy[rs].y;
                ri = 0;
            }
        }
    }
}

// sort sprite (descending by ofs_z; t and [first, last] are 0-based/inclusive)
inline void spqsort(std::vector<int>& t, int first, int last) {
    if (first > last) return;
    int p = first;
    for (int i = first + 1; i <= last; i++) {
        if (sp[t[i]].ofs_z > sp[t[first]].ofs_z) {
            p++;
            std::swap(t[p], t[i]);
        }
    }
    std::swap(t[p], t[first]);
    spqsort(t, first, p - 1);
    spqsort(t, p + 1, last);
}

// draw sprite
inline void spdraw() {
    std::vector<int> order;
    for (int i = 0; i < 512; i++) if (sp[i].show) order.push_back(i);
    spqsort(order, 0, (int)order.size() - 1);
    for (int n : order) {
        if (sp[n].color != 0x07) pal(0x07, sp[n].color);
        if (sp[n].rot == 0) {
            spsspr(sp[n].page, sp[n].chr_u, sp[n].chr_v, sp[n].chr_w, sp[n].chr_h,
                   sp[n].ofs_x, sp[n].ofs_y, sp[n].home_x, sp[n].home_y,
                   sp[n].scale_x, sp[n].scale_y, sp[n].chr_a);
        } else {
            sprspr(sp[n].page, sp[n].chr_u, sp[n].chr_v, sp[n].chr_w, sp[n].chr_h,
                   sp[n].ofs_x, sp[n].ofs_y, sp[n].home_x, sp[n].home_y,
                   sp[n].scale_x, sp[n].scale_y, sp[n].rot, sp[n].chr_a);
        }
        if (sp[n].color != 0x07) pal(0x07, 0x07);
    }
}

} // namespace db
