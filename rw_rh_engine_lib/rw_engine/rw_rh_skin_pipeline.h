//
// Created by peter on 12.05.2020.
//
#pragma once
#include "rw_rh_pipeline.h"
#include <common_headers.h>
#include <cstdint>
#include <functional>
#include <vector>

namespace rh::rw::engine
{
class IAnimHierarcy;
void PrepareBoneMatrices( DirectX::XMFLOAT4X3 *matrix_cache, RpAtomic *atomic,
                          IAnimHierarcy &anim_hier );
RenderStatus InstanceSkinAtomic( RpAtomic *           atomic,
                                 RpGeometryInterface *geom_io );
} // namespace rh::rw::engine