// navi.hpp : STUB. navi.lua (mini-map) is not ported yet -- these no-ops
// exist only so game.hpp's loop/draw call list is complete.
#pragma once
#include "db.hpp"

namespace db {
inline void nvinit() {}
inline void nvrset() {}
inline void nvloop() {}
inline void nvdraw() {}
} // namespace db
