// game.hpp : game state machine
//
// STUB. race.lua/course.lua/mycar.lua/rival.lua/... are not ported yet, so
// gmloop() has nothing real to drive. It exists purely so title.hpp's
// `app.proc_ = gmloop` transition compiles/links; title screen -> race
// hand-off is the next thing to port.
#pragma once
#include "db.hpp"

namespace db {

inline void gminit() {}
inline void gmloop() {}

} // namespace db
