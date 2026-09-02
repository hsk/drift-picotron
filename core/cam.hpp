// cam.hpp : camera
#pragma once
#include "db.hpp"

namespace db {

struct Cam {
    double ox_ = 0, oy_ = 0, oz_ = 0, os_ = 0, oc_ = 0;
    double vx_ = 0, vy_ = 0, vz_ = 0;
    double rx_ = 0, ry_ = 0, ra_ = 0, rs_ = 0, rc_ = 0;
};
inline Cam cam;

inline void cminit() {}

} // namespace db
