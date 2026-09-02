// cockpit.hpp : STUB. cockpit.lua (dashboard overlay) is not ported yet --
// these no-ops exist only so game.hpp's loop/draw call list is complete.
#pragma once
#include "db.hpp"

namespace db {
inline void cpinit() {}
inline void cprset() {}
inline void cploop() {}
inline void cpdraw() {}
} // namespace db
