#pragma once
#include <rwtestsample.h>
class ForwardPBRPipeline;
/**
 * @brief GTA Model loading test sample for RenderHook
 *
 */
class ModelLoadingTest : public rh::rw::engine::RwTestSample
{
public:
    ModelLoadingTest( rh::engine::RenderingAPI api, HINSTANCE inst )
        : rh::rw::engine::RwTestSample( api, inst )
    {}
    bool CustomInitialize() final;
    void CustomShutdown() final;
    void CustomRender() override;
    void CustomUpdate( float dt ) override;

private:
    void LoadDFF( const rh::engine::String &path );

private:
    rh::engine::String m_sDffPath;
    ForwardPBRPipeline *m_pPipeline;
    bool m_bMouseAquired = false;
    rh::engine::IGPUResource *mBaseConstantBuffer = nullptr;
    rh::engine::IGPUResource *mPerModelConstantBuffer = nullptr;
    std::vector<RpClump *> m_vClumpList;
};
