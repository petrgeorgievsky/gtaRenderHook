#pragma once
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
    void SetupCameraContext();

  private:
    RwCamera *m_pCamera = nullptr;
};

} // namespace rh::rw::engine
