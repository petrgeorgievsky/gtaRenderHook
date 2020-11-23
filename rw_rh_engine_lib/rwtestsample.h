#pragma once
#include <TestUtils/TestSample.h>
struct RwCamera;
struct RwFrame;
namespace rh::rw::engine
{
class RwTestSample : public rh::tests::TestSample
{
  public:
    RwTestSample( rh::engine::RenderingAPI api, void *inst );
    ~RwTestSample() override;
    bool Initialize( void *wnd ) override;
    bool CustomInitialize() override;
    void CustomShutdown() override;
    void Render() override;

  protected:
    RwCamera *m_pMainCamera{};
    RwFrame * m_pMainCameraFrame{};
};
} // namespace rh::rw::engine
