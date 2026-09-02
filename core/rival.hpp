// rival.hpp : rival cars
//
// STUB (deliberately deferred). rival.n_ = 0 so mycar.hpp's mychit()
// collision loop and every loop below are no-ops -- everything here mirrors
// rival.lua's shape so it's a mechanical fill-in once the other stages are
// working. rival.spr_ keeps its original constant so mycar's sprite-slot
// arithmetic (`rival.spr_ + 2*rival.n_`) still lines up once n_ is real.
#pragma once
#include "db.hpp"

namespace db {

struct Rival {
    int n_ = 0;
    int spr_ = 384;
    double dst_[7] = {}, lpd_[7] = {}, sde_[7] = {};
    int lpc_[7] = {}, lpi_[7] = {};
    int rnk_[7][2] = {};
    double mov_[7][2] = {};
    double spd_[7] = {}, spa_[7] = {};
    double hdl_[7] = {}, hds_[7] = {};
    double hdp_ = 0;
    double pos_[7][3] = {};
    int smk_[7] = {};
};
inline Rival rival;

inline void rvinit() {}
inline void rvrset() {}
inline void rvloop() {}
inline void rvdraw() {}

} // namespace db
