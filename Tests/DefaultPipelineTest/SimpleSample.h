#pragma once
#include <TestUtils\TestSample.h>
#include <dinput.h>
#include <unordered_map>
#include <atomic>
#include <condition_variable>
#include <array>
#include "forward_pbr_pipeline.h"

namespace RHEngine
{
class IRenderingContext;
class D3D11ConstantBuffer;
class D3D11PrimitiveBatch;
}
class ForwardPBRPipeline;

struct ModelData
{
    RpClump* object;
    float drawDist;
    float boundSphereRad;
    bool hasAlpha;
    std::string dffname;
    std::string txdname;
};

struct ModelInfo
{
    uint32_t modelId;
    DirectX::XMMATRIX transform;
    bool selected = false;
};

struct RenderingData
{
    ModelData data;
    ModelInfo info;
};

extern std::unordered_map<std::string, RwTexture*> g_ideTextureList;
extern std::vector<std::string> g_ideLoadedTextureList;
extern std::unordered_map<std::string, MaterialCB> g_materialBuffer;

class MaterialBufferProvider : public IMaterialBufferProvider
{
    MaterialCB* GetBuffer( std::string name )
    {
        if( g_materialBuffer.find( name ) != g_materialBuffer.end() )
            return &g_materialBuffer[name];
        else
            return nullptr;
    }
};
struct RenderBatch 
{
    uint32_t start = 0, end = 0;
};
extern MaterialBufferProvider g_materialBufferProvider;
/**
* @brief Simple test sample for RenderHook, write your own code as you wish
*
*/
class SimpleSample : public RHTests::TestSample
{
public:
    SimpleSample( RHEngine::RHRenderingAPI api, HINSTANCE inst ) : RHTests::TestSample( api, inst ) { }
    virtual ~SimpleSample() {};
    void CustomRender() override;
    bool CustomInitialize() override;
    void CustomUpdate( float dt ) override;
    void CustomShutdown() override;
private:
    void RenderUI();
    void LoadIDE( const RHEngine::String& path );
    void LoadIPL( const RHEngine::String& path, int32_t version );
    void LoadIPLBinary( const RHEngine::String& path );
    void LoadDFFForIPL( const RHEngine::String& models_path );
    void DrawClump( RHEngine::IRenderingContext* context, RpClump* clump, bool selected, DirectX::XMMATRIX transf_mult );
    void RenderWorker( std::uint32_t id );
private:
    std::unordered_map<uint32_t, ModelData> m_ideModelList{};
    std::vector<ModelInfo> m_iplModelInfoMap{};

    std::vector<RenderingData> m_vCurrentFrameRenderList{};
    std::vector<RenderingData> m_vCurrentFrameAlphaRenderList{};
    uint32_t m_nVisibleObjects =0;

    bool button_toggled[4] = { false, false, false, false };
    bool mouse_aq = false;
    bool m_bThreadShouldStaph = false;
    bool m_bUseConstDrawDist = false;
    float m_fMaxDrawDist = 300;
    float m_fCamSpeed = 1;
    char m_sTexFilter[32] = {};
    char m_sModelFilter[32] = {};
    float m_fFPS = 0;
    float m_fCullingTime = 0;
    float m_fRenderingTime = 0;
    int32_t m_nMinMeshPerThread = 100;
    std::vector<float> m_fRenderingThreadTime;
    const uint32_t m_nMaxThreadCount = 64;
    std::vector<void*> m_aCommandListBuffer{};
    std::atomic<uint32_t> m_nCmdListCount;
    std::vector<HANDLE> m_nBeginRenderingEvents;
    std::vector<HANDLE> m_nEndRenderingEvents;
    std::vector<std::thread> m_tWorkers;
    std::condition_variable m_main_thread_cond;
    std::mutex m_main_thread_mutex;
    std::mutex m_io_mtx;
    std::vector<RenderBatch> m_batches;
    int32_t m_nWorkerThreadCount = 0;
    ForwardPBRPipeline* m_pForwardPBRPipeline = nullptr;

    RHEngine::D3D11ConstantBuffer* mBaseConstantBuffer = nullptr;
    RHEngine::D3D11ConstantBuffer* mPerModelConstantBuffer = nullptr;
    std::vector<RHEngine::IRenderingContext*> mDeferredContextList;

    DirectX::XMMATRIX mObjTransformMatrix{};
    DirectX::XMMATRIX mCurrentTransformMultiplier{};
};