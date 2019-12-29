#pragma once

struct RwCamera;
namespace rh::rw::engine
{

class RwCameraEndUpdateCmd
{
public:
    RwCameraEndUpdateCmd( RwCamera * camera );
    ~RwCameraEndUpdateCmd();
    bool Execute();
private:
    RwCamera * m_pCamera = nullptr;
};

}
