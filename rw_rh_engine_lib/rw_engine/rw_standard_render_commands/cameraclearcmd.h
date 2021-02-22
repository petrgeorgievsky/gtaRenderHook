#pragma once
#include <array>
#include <cstdint>
struct RwCamera;
struct RwRGBA;

namespace rh::rw::engine
{

class RwCameraClearCmd
{
  public:
    RwCameraClearCmd( RwCamera *camera, RwRGBA *color, int32_t clear_mode );
    ~RwCameraClearCmd();
    bool Execute();

  private:
    [[maybe_unused]] RwCamera *m_pCamera = nullptr;
    RwRGBA *                   m_aClearColor;
    int32_t                    m_nClearMode = 0;
};

} // namespace rh::rw::engine
