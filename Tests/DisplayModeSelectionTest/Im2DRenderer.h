#pragma once
#include <Engine/Common/ArrayProxy.h>
#include <Engine/Common/IBuffer.h>
#include <Engine/Common/IDescriptorSet.h>
#include <Engine/Common/IDeviceState.h>
#include <Engine/Common/IPipeline.h>

/// renderdoc fmt: float4 pos; float2 tc; unormb4 color; uint padd;
struct Im2DVertex
{
    float    x, y, z, w;
    float    u, v;
    uint32_t color;
    uint32_t padding;
};

struct Im2DDrawCall
{
    uint32_t vertexBase;
    uint32_t vertexCount;
    uint32_t indexBase;
    uint32_t indexCount;
};

struct Im2DRendererInitParams
{
    // Dependencies...
    rh::engine::IDeviceState *        mDeviceState;
    rh::engine::IRenderPass *         mRenderPass;
    rh::engine::IDescriptorSetLayout *mGlobalsDescriptorSetLayout;
    // params
    uint32_t mCmdBufferCount;
};

class Im2DRenderer
{
    using VertexType = Im2DVertex;
    using IndexType  = uint16_t;

  public:
    Im2DRenderer( const Im2DRendererInitParams &params );
    ~Im2DRenderer();

    void RecordDrawCall( const rh::engine::ArrayProxy<VertexType> &vertex_list,
                         const rh::engine::ArrayProxy<IndexType> & index_list );
    void
         RecordDrawCall( const rh::engine::ArrayProxy<VertexType> &vertex_list );
    void SetImageView( rh::engine::IImageView *img_view );
    void DrawBatch( rh::engine::ICommandBuffer *cmdbuffer );
    void FrameEnd();

    rh::engine::IPipelineLayout *GetLayout();

  private:
    void          CreatePipelineLayout( const Im2DRendererInitParams &params );
    void CreatePipeline( const Im2DRendererInitParams &params );
    void          CreateBuffers( const Im2DRendererInitParams &params );

    std::vector<VertexType>   mVertexScratchSpace;
    std::vector<Im2DVertex>   mIndexScratchSpace;
    std::vector<Im2DDrawCall> mDrawCalls;

    /// GPU resources
    rh::engine::IBuffer *       mIndexBuffer;
    rh::engine::IBuffer *       mVertexBuffer;
    rh::engine::IDescriptorSet *mDescriptorSet;
    std::vector<rh::engine::IDescriptorSet*> mTextureDescriptorSets;
    rh::engine::IDescriptorSetAllocator *mDescriptorSetAlloc;

    // Pipeline resources

    rh::engine::IPipeline *           m2DPipeline                 = nullptr;
    rh::engine::IPipelineLayout *     m2DPipelineLayout           = nullptr;
    rh::engine::IDescriptorSetLayout *mTextureDescriptorSetLayout = nullptr;

    // Dependencies
    rh::engine::ISampler *mTextureSampler;
    uint32_t              mFrameCounter = 0;
    uint32_t              mCmdBufferCount = 1;

    rh::engine::IDeviceState *mDeviceState = nullptr;
};