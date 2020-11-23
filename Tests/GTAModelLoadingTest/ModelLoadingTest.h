#pragma once
#include <Engine/Common/IBuffer.h>
#include <rwtestsample.h>
#include <string>
#include <vector>
class ForwardPBRPipeline;
struct RpClump;
/**
 * @brief GTA Model loading test sample for RenderHook
 *
 */
class ModelLoadingTest : public rh::rw::engine::RwTestSample
{
  public:
    ModelLoadingTest( rh::engine::RenderingAPI api, void *inst )
        : rh::rw::engine::RwTestSample( api, inst )
    {
    }
    bool CustomInitialize() final;
    void CustomShutdown() final;
    void CustomRender() override;
    void CustomUpdate( float dt ) override;

  private:
    void LoadDFF( const std::string &path );

  private:
    std::string            m_sDffPath;
    ForwardPBRPipeline *   m_pPipeline;
    bool                   m_bMouseAquired         = false;
    rh::engine::IBuffer *  mBaseConstantBuffer     = nullptr;
    rh::engine::IBuffer *  mPerModelConstantBuffer = nullptr;
    std::vector<RpClump *> m_vClumpList;
};
