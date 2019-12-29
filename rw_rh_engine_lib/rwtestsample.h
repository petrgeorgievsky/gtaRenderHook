#pragma once
#include "RwRenderEngine.h"
#include <TestUtils/TestSample.h>
namespace rh::rw::engine {
class RwTestSample : public rh::tests::TestSample
{
public:
    RwTestSample( rh::engine::RenderingAPI api, HINSTANCE inst );
    ~RwTestSample() override;
    bool Initialize( HWND wnd ) override;
    bool CustomInitialize() override;
    void CustomShutdown() override;
    void Render() override;

protected:
    RwCamera *m_pMainCamera{};
    RwFrame *m_pMainCameraFrame{};
};
} // namespace rw_rh_engine
