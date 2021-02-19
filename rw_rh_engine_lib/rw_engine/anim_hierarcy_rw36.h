//
// Created by peter on 19.02.2021.
//
#pragma once
#include "i_anim_hierarcy.h"

namespace rh::rw::engine
{

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
}