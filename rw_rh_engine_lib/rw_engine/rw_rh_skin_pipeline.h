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
class IAnimHierarcy
{
  public:
    virtual void             Init( void *_base )     = 0;
    virtual uint32_t         GetFlags()              = 0;
    virtual uint32_t         GetNumNodes()           = 0;
    virtual RpHAnimNodeInfo *GetNodeInfo()           = 0;
    virtual RwMatrix *       GetSkinToBoneMatrices() = 0;
};
class AnimHierarcyRw36 : public IAnimHierarcy
{
  private:
    RpHAnimHierarchy *base;

  public:
    void Init( void *_base ) override
    {
        base = ( static_cast<RpHAnimHierarchy *>( _base ) );
    }
    uint32_t         GetFlags() override { return base->flags; }
    uint32_t         GetNumNodes() override { return base->numNodes; }
    RpHAnimNodeInfo *GetNodeInfo() override { return base->pNodeInfo; }
    RwMatrix *GetSkinToBoneMatrices() override { return base->pMatrixArray; }
};
void PrepareBoneMatrices( DirectX::XMFLOAT4X3 *matrix_cache, RpAtomic *atomic,
                          IAnimHierarcy &anim_hier );
RenderStatus RwRHInstanceSkinAtomic( RpAtomic *           atomic,
                                     RpGeometryInterface *geom_io );
} // namespace rh::rw::engine