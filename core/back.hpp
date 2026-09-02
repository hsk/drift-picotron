// back.hpp : back ground (wrap-around sky/horizon panorama)
//
// Builds a 1024x1024 panorama into app.gpage0_ once (sky color, ground
// color, and a tiled mountain-silhouette strip reusing chara.hpp's
// gpage3_ tile bank), then bkdraw() each frame samples a window out of it
// based on camera heading/pitch so it pans as the camera turns.
#pragma once
#include "db.hpp"
#include "sp.hpp"
#include "cam.hpp"
#include <cmath>

namespace db {

struct Back {
    int spr_ = 0;
};
inline Back back;

void bkbgput(int x, int y, int c);

inline void bkinit() {
    back.spr_ = 509;
    app.gpage0_ = Userdata(1024, 1024);

    static const char* data[4] = {
        "1111111111111111",
        "1234111234111234",
        "5555615555615556",
        "7787777887778777",
    };
    int i = 0;
    for (int y = 28; y <= 31; y++) {
        const char* s = data[i];
        i++;
        for (int x = 0; x <= 15; x++) {
            int c = s[x] - '0';
            bkbgput(x + 0, y, c);
            bkbgput(x + 16, y, c);
            bkbgput(x + 32, y, c);
            bkbgput(x + 48, y, c);
        }
    }
    int c0 = app.color_[0x22];
    for (int y = 0; y <= 28 * 16 - 1; y++)
        for (int x = 0; x <= 64 * 16 - 1; x++)
            app.gpage0_.set(x, y, c0);
    int c1 = app.color_[0x37];
    for (int y = 32 * 16; y <= 64 * 16 - 1; y++)
        for (int x = 0; x <= 64 * 16 - 1; x++)
            app.gpage0_.set(x, y, c1);
}

inline void bkrset() {}
inline void bkloop() {}

inline void bkdraw() {
    int n = back.spr_;
    double x = 0;
    double y = -cam.oy_ * cam.oc_ + 128 * cam.os_;
    double z = cam.oy_ * cam.os_ + 128 * cam.oc_;
    double s = app.or_ / (z / 2);
    double a = std::atan2(cam.vx_, cam.vz_); // math.atan(y,x) in the original == atan2
    double r = 280;
    double u = (512 - 256 * a / M_PI) - (x * s + r);
    double v = 512 - (y * s + r);
    spchr(n, (int)u, (int)v, (int)(r * 2), (int)(r * 2), 0x01);
    spofs(n, app.ox_, app.oy_, 1024);
    sphome(n, r, r);
    spscale(n, 1, 1);
    sprot(n, cam.ra_);
    sppage(n, &app.gpage0_);
    spshow(n);
}

// put a 16x16 tile (index c into the gpage3_ tile bank) at bg tile (x,y)
inline void bkbgput(int x, int y, int c) {
    int u = (c % 32) * 16;
    int v = (c / 32) * 16;
    x *= 16;
    y *= 16;
    for (int j = 0; j <= 15; j++)
        for (int i = 0; i <= 15; i++) {
            int p = app.gpage3_.get(u + i, v + j);
            app.gpage0_.set(x + i, y + j, p);
        }
}

inline void bkbgfill(int x0, int y0, int x1, int y1, int c) {
    for (int y = y0; y <= y1; y++)
        for (int x = x0; x <= x1; x++)
            bkbgput(x, y, c);
}

inline void bkbgcopy(int sx, int sy, int ex, int ey, int dx, int dy) {
    sx *= 16; sy *= 16;
    ex = ex * 16 + 15; ey = ey * 16 + 15;
    dx *= 16; dy *= 16;
    for (int y = sy; y <= ey; y++)
        for (int x = sx; x <= ex; x++) {
            int p = app.gpage0_.get(x, y);
            app.gpage0_.set(dx + x - sx, dy + y - sy, p);
        }
}

} // namespace db
