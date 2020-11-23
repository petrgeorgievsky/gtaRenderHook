#pragma once
#include <common_headers.h>
#include <rwtestsample.h>
#include <string>
#include <vector>

class ForwardPBRPipeline;
namespace rw_raytracing_lib
{
class BVHBuilder;
class RayTracingScene;
class RayTracer;
class RTShadowsPass;
class RTPerPixelGIPass;
class LowFreqGIFilterPass;
} // namespace rw_raytracing_lib

class GTAMapLoader;
class GBufferPass;
class GBufferPipeline;
/**
 * @brief GTA Model loading test sample for RenderHook
 *
 */
class ModelLoadingTest : public rh::rw::engine::RwTestSample
{
  public:
    ModelLoadingTest( rh::engine::RenderingAPI api, HINSTANCE inst )
        : rh::rw::engine::RwTestSample( api, inst )
    {
    }
    bool CustomInitialize() final;
    void CustomShutdown() final;
    void CustomRender() override;
    void CustomUpdate( float dt ) override;
    void GenerateQuad( float w, float h );

  private:
    void LoadDFF( const std::string &path );
    void PrepareDFFs();
    void RenderUI();

  private:
    // ForwardPBRPipeline *m_pPipeline  = nullptr;
    // GBufferPipeline *   m_pGBPipe    = nullptr;
    // GBufferPass *       m_pGBPass    = nullptr;
    GTAMapLoader *m_pMapLoader = nullptr;
    /*
    rw_raytracing_lib::BVHBuilder *         bvh_builder       = nullptr;
    rw_raytracing_lib::RayTracingScene *    rt_scene          = nullptr;
    rw_raytracing_lib::RayTracer *          ray_tracer        = nullptr;
    rw_raytracing_lib::RTShadowsPass *      m_pShadowsPass    = nullptr;
    rw_raytracing_lib::RTPerPixelGIPass *   m_pGIPass         = nullptr;
    rw_raytracing_lib::LowFreqGIFilterPass *m_pLFGIFilterPass = nullptr;
     */
    std::vector<RpClump *> m_vClumpList;
    // std::vector<RwIm2DVertex> m_vQuad;
    bool   m_bMouseAquired = false;
    bool   padd[3];
    double m_fFrameRate = 0.0;
};
