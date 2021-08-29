//
// Created by peter on 29.08.2021.
//

#pragma once
struct RwTexture;

namespace rh::rw::engine
{
class RwNativeTextureGetSizeCmd
{
  public:
    RwNativeTextureGetSizeCmd( RwTexture *texture );
    bool Execute( uint32_t &size );

  private:
    RwTexture *Texture;
};
} // namespace rh::rw::engine
