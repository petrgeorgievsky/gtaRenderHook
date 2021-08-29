#pragma once
struct RwStream;
struct RwTexture;

namespace rh::rw::engine
{
class RwNativeTextureWriteCmd
{
  public:
    RwNativeTextureWriteCmd( RwStream *stream, RwTexture *texture );
    bool Execute();

  private:
    RwStream  *Stream;
    RwTexture *Texture;
};
} // namespace rh::rw::engine
