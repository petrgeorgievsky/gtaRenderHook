#pragma once
#include "../Common/IRenderingContext.h"
#include "RenderStateCache/D3D11RenderStateCache.h"

namespace rh::engine {
class D3D11RenderingContext final : public IRenderingContext
{
public:
    ~D3D11RenderingContext() override;
    void Init( ID3D11DeviceContext *context, IGPUAllocator *allocator );
    void BindViewPorts( const std::vector<ViewPort> &viewports ) override;
    bool BindImageBuffers( ImageBindType bindType,
                           const std::vector<IndexPtrPair> &buffers ) override;
    bool BindConstantBuffers( uint8_t bindStages,
                              const std::vector<IndexPtrPair> &buffers ) override;
    bool ClearImageBuffer( ImageClearType clearType,
                           void *buffer,
                           const std::array<float, 4> &clearColor ) override;
    void BindVertexBuffers( const std::vector<void *> &buffers,
                            uint32_t *strides,
                            uint32_t *offsets ) override;
    bool BindIndexBuffer( void *buffer ) override;
    bool BindInputLayout( void *layout ) override;
    void BindShader( void *shader ) override;
    void BindSamplers( const std::vector<IndexPtrPair> &sampler_states, ShaderStage stage ) override;
    void UpdateBuffer( void *buffer, const void *data, int32_t size ) override;
    void SetPrimitiveTopology( PrimitiveType type ) override;
    void FlushCache() override;
    void RecordDrawCall( uint32_t vertexCount, uint32_t startVertex ) override;
    void RecordDrawCallIndexed( uint32_t indexCount,
                                uint32_t startIndex,
                                uint32_t baseVertex ) override;
    void ReleaseResources() override;
    void FinishCmdList( void *&cmdlist ) override;
    void ReplayCmdList( void *cmdlist ) override;
    void DispatchThreads( uint32_t x, uint32_t y, uint32_t z ) override;

    D3D11RenderStateCache *GetStateCache();
    ID3D11DeviceContext *GetContextImpl() { return m_pContextImpl; }

private:
    ID3D11DeviceContext *m_pContextImpl = nullptr;
    D3D11RenderStateCache m_renderStateCache;

    // IRenderingContext interface
public:
    void ResetShader( void *shader ) override;

    // IRenderingContext interface
public:
    void CopyImageBuffer( void *dest_buffer,
                          void *src_buffer,
                          uint32_t dest_subres_id,
                          uint32_t dest_pos_x,
                          uint32_t dest_pos_y,
                          uint32_t dest_pos_z ) override;
};

}; // namespace rh::engine
