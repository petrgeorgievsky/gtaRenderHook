#pragma once
#include <common_headers.h>
struct RwCamera;
namespace rh::rw::engine {

class RwCameraBeginUpdateCmd
{
public:
    RwCameraBeginUpdateCmd( RwCamera *camera );
    ~RwCameraBeginUpdateCmd();
    bool Execute();

private:
    void SetupCameraContext();
    DirectX::XMMATRIX GetCameraTransform();
    DirectX::XMMATRIX GetProjectionTransform();

private:
    RwCamera *m_pCamera = nullptr;
};

} // namespace rw_rh_engine
