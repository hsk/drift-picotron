// test_render.cpp : headless regression test for the core rendering
// pipeline (db/color/sp/bg/chara). No SDL dependency -- core/*.hpp never
// includes SDL, so this exercises exactly the platform-independent code
// that will also run on the future ESP32-P4 backend.
#include "../core/app.hpp"
#include <cstdio>
#include <cstdlib>

static int g_failures = 0;

#define CHECK(cond, msg) \
    do { \
        if (!(cond)) { \
            std::fprintf(stderr, "FAIL: %s (%s:%d)\n", msg, __FILE__, __LINE__); \
            g_failures++; \
        } else { \
            std::printf("ok   %s\n", msg); \
        } \
    } while (0)

int main() {
    db::app_init(); // spinit/bginit/clinit/chinit

    // color table: all 64 slots resolve to a valid palette index (0-31)
    bool color_ok = true;
    for (int i = 0; i < 64; i++)
        if (db::app.color_[i] < 0 || db::app.color_[i] > 31) color_ok = false;
    CHECK(color_ok, "app.color_[] entries are all valid 0-31 palette indices");

    // gpage2_ / gpage3_ were allocated at the expected size
    CHECK(db::app.gpage2_.w == 256 && db::app.gpage2_.h == 512, "gpage2_ is 256x512");
    CHECK(db::app.gpage3_.w == 256 && db::app.gpage3_.h == 512, "gpage3_ is 256x512");

    // far corner of gpage2_, well outside any sprite def, stays the default
    // transparent fill (0x3f)
    CHECK(db::app.gpage2_.get(255, 511) == 0x3f, "gpage2_ unpainted region reads default 0x3f");

    // first sprite def (u=0,v=0,w=30,h=7) paints a non-transparent, in-palette
    // pixel at its first nonzero pixel (row0, x=9 is '2' in the source data)
    int c = db::app.gpage2_.get(9, 0);
    CHECK(c != 0x3f && c >= 0 && c <= 31, "gpage2_ known-painted pixel is a real palette color");

    // bg-to-spr slicing produced 256 16x16 tiles
    bool spr_ok = true;
    for (int n = 0; n < 256; n++) {
        db::Userdata* t = db::get_spr(n);
        if (!t || t->w != 16 || t->h != 16) { spr_ok = false; break; }
    }
    CHECK(spr_ok, "get_spr(0..255) all return valid 16x16 tiles");

    // spdraw() end-to-end: show one known sprite and confirm it actually
    // marks a pixel on the logical screen buffer
    db::cls(0x06);
    db::spchr(0, 0, 0, 30, 7, 0);
    db::sppage(0, &db::app.gpage2_);
    db::sphome(0, 0, 0);
    db::spscale(0, 1, 1);
    db::spofs(0, 10, 10, 0);
    db::spcolor(0, 0x07);
    db::spshow(0);
    db::spdraw();
    bool drew_something = false;
    for (int y = 10; y < 17 && !drew_something; y++)
        for (int x = 10; x < 40 && !drew_something; x++)
            if (db::screen.get(x, y) != 0x06) drew_something = true;
    CHECK(drew_something, "spdraw() paints the shown sprite onto db::screen");

    std::printf("\n%s\n", g_failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED");
    return g_failures == 0 ? 0 : 1;
}
