// bg.hpp : bg library
//
// bgdraw() is dead code in the original app_draw (commented out even in
// app.lua), since course.* paints the screen directly via tline3d. Kept
// here only for parity with bg.lua/bg.py.
#pragma once
#include "db.hpp"

namespace db {

inline void bginit() { bg = Bg{}; }

inline void bgscreen(int, int w, int h) { bg.screen_w = w; bg.screen_h = h; }
inline void bgofs(int, int x, int y, int z) { bg.ofs_x = x; bg.ofs_y = y; bg.ofs_z = z; }
inline void bghome(int, int x, int y) { bg.home_x = x; bg.home_y = y; }
inline void bgscale(int, double x, double y) { bg.scale_x = x; bg.scale_y = y; }
inline void bgrot(int, double r) { bg.rot = r; }
inline void bgput(int, int x, int y, int c) { mset(x, y, c); }
inline void bgfill(int, int x0, int y0, int x1, int y1, int c) {
    for (int y = y0; y <= y1; y++)
        for (int x = x0; x <= x1; x++)
            mset(x, y, c);
}
inline void bgshow(int) { bg.show = true; }
inline void bghide(int) { bg.show = false; }
inline void bgdraw() {
    if (bg.show) bg_map(0, 0, bg.ofs_x - bg.home_x, bg.ofs_y - bg.home_y);
}

} // namespace db
