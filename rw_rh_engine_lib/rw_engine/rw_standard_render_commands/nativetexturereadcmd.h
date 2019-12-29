#pragma once
struct RwStream;
struct RwTexture;

namespace rh::rw::engine {
class RwNativeTextureReadCmd
{
public:
    RwNativeTextureReadCmd(RwStream *stream, RwTexture **texture);
    bool Execute();

private:
    RwStream *m_pStream;
    RwTexture **m_pTexture;
};
} // namespace rw_rh_engine
