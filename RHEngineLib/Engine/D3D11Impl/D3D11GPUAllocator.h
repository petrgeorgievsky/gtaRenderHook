#pragma once
#include "..\Common\IGPUAllocator.h"
#include <d3d11_3.h>
namespace rh::engine {
class D3D11DeviceOutputView;
class IGPUResource;
class D3D11GPUAllocator : public IGPUAllocator
{
public:
    ~D3D11GPUAllocator() override;
    bool AllocateImageBuffer( const ImageBufferInfo &info, IGPUResource *&buffer_ptr ) override;
    bool FreeImageBuffer( void *buffer, ImageBufferType type ) override;
    bool AllocateVertexBuffer( const VertexBufferInfo &info, IGPUResource *&buffer_ptr ) override;
    bool AllocateIndexBuffer( const IndexBufferInfo &info, IGPUResource *&buffer_ptr ) override;
    bool AllocateInputLayout( const InputLayoutInfo &info, IGPUResource *&buffer_ptr ) override;
    bool AllocateSampler( const Sampler &info, void *&buffer_ptr ) override;
    bool AllocateShader( const ShaderInfo &info, IGPUResource *&buffer_ptr ) override;
    bool AllocateDepthStencilState( const DepthStencilState &info, void *&buffer_ptr ) override;
    bool AllocateBlendState( const BlendState &info, void *&buffer_ptr ) override;
    bool AllocateRasterizerState( const RasterizerState &info, void *&buffer_ptr ) override;
    bool AllocateConstantBuffer( const ConstantBufferInfo &info,
                                 IGPUResource *&buffer_ptr ) override;
    bool AllocateDeferredContext( IGPUResource *&context_ptr ) override;
    bool AllocateStructuredBuffer( const StructuredBufferInfo &info,
                                   IGPUResource *&buffer_ptr ) override;

    void Init( ID3D11Device *device, D3D11DeviceOutputView *output );
    ID3D11Device *GetDevice();

private:
    ID3D11Device *m_pDevice = nullptr;
    D3D11DeviceOutputView *m_pDeviceOutput = nullptr;
};

}; // namespace rh::engine
