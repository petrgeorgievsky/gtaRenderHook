#pragma once
#include <array>
struct RwCamera;
struct RwRGBA;

namespace rh::rw::engine
{

class RwCameraClearCmd
{
public:
    RwCameraClearCmd( RwCamera * camera, RwRGBA* color, int32_t clear_mode );
    ~RwCameraClearCmd();
    bool Execute();
private:
    RwCamera * m_pCamera = nullptr;
    std::array<float, 4> m_aClearColor;
    int32_t m_nClearMode = 0;
};

}
