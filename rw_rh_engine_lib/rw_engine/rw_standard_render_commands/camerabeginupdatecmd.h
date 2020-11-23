#pragma once
#include <common_headers.h>
struct RwCamera;
namespace rh::rw::engine
{

class RwCameraBeginUpdateCmd
{
  public:
    RwCameraBeginUpdateCmd( RwCamera *camera );
    ~RwCameraBeginUpdateCmd();
    bool Execute();

  private:
    void                SetupCameraContext();
    DirectX::XMFLOAT4X4 GetCameraTransform();
    DirectX::XMFLOAT4X4 GetProjectionTransform();

  private:
    RwCamera *m_pCamera = nullptr;
};

} // namespace rh::rw::engine
