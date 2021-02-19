//
// Created by peter on 19.02.2021.
//

#pragma once
#include <common_headers.h>

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
} // namespace rh::rw::engine