#include "D3D11GPUAllocator.h"
#include "Buffers/D3D11ConstantBuffer.h"
#include "Buffers/D3D11DynamicIndexBuffer.h"
#include "Buffers/D3D11DynamicVertexBuffer.h"
#include "Buffers/D3D11IndexBuffer.h"
#include "Buffers/D3D11StructuredBuffer.h"
#include "Buffers/D3D11VertexBuffer.h"
#include "D3D11Convert.h"
#include "D3D11Common.h"
#include "D3D11DeviceOutputView.h"
#include "D3D11InputLayout.h"
#include "D3D11RenderingContext.h"
#include "ImageBuffers/D3D11BackBuffer.h"
#include "ImageBuffers/D3D11DepthStencilBuffer.h"
#include "ImageBuffers/D3D11Texture2D.h"
#include "ImageBuffers/D3D11Texture2DArray.h"
#include "Shaders/D3D11ComputeShader.h"
#include "Shaders/D3D11PixelShader.h"
#include "Shaders/D3D11VertexShader.h"
#include <Engine/Common/types/image_buffer_info.h>
#include <Engine/Common/types/image_buffer_type.h>
#include <Engine/Common/types/index_buffer_info.h>
#include <Engine/Common/types/input_layout_info.h>
#include <Engine/Common/types/shader_info.h>
#include <Engine/Common/types/shader_stage.h>
#include <Engine/Common/types/vertex_buffer_info.h>
#include <d3d11_3.h>

void rh::engine::D3D11GPUAllocator::Init( ID3D11Device *device, D3D11DeviceOutputView *output )
{
    m_pDevice = device;
    m_pDeviceOutput = output;
}

bool rh::engine::D3D11GPUAllocator::AllocateImageBuffer( const ImageBufferInfo &info,
                                                         IGPUResource *&buffer_ptr )
{
    buffer_ptr = nullptr;
    switch ( info.type ) {
    case ImageBufferType::BackBuffer:
        // Default backbuffer, with swap-chain bound to main window
        buffer_ptr = m_pDeviceOutput->GetBackBufferView( m_pDevice );
        break;
    case ImageBufferType::DepthBuffer:
        buffer_ptr = new D3D11DepthStencilBuffer( m_pDevice,
                                                  {info.width,
                                                   info.height,
                                                   DXGI_FORMAT_D24_UNORM_S8_UINT} );
        break;
    case ImageBufferType::RenderTargetBuffer: {
        D3D11Texture2DCreateInfo &&create_info = {info.width,
                                                  info.height,
                                                  info.mipLevels,
                                                  GetDXGIResourceFormat( info.format ),
                                                  true,
                                                  true,
                                                  true};

        std::vector<D3D11_SUBRESOURCE_DATA> impl_data;
        for ( auto init_data : info.initialDataVec ) {
            D3D11_SUBRESOURCE_DATA data{};
            data.pSysMem = init_data.data;
            data.SysMemPitch = init_data.stride;
            impl_data.push_back( data );
        }

        buffer_ptr = new D3D11Texture2D( m_pDevice, create_info, std::move( impl_data ) );
        break;
    }
    case ImageBufferType::TextureBuffer: {
        D3D11Texture2DCreateInfo &&create_info = {info.width,
                                                  info.height,
                                                  info.mipLevels,
                                                  GetDXGIResourceFormat( info.format ),
                                                  false,
                                                  true,
                                                  false};

        std::vector<D3D11_SUBRESOURCE_DATA> impl_data;
        for ( auto init_data : info.initialDataVec ) {
            D3D11_SUBRESOURCE_DATA data{};
            data.pSysMem = init_data.data;
            data.SysMemPitch = init_data.stride;
            impl_data.push_back( data );
        }

        buffer_ptr = new D3D11Texture2D( m_pDevice, create_info, std::move( impl_data ) );
        break;
    }
    case ImageBufferType::DynamicTextureArrayBuffer: {
        D3D11Texture2DArrayCreateInfo &&create_info = {info.width,
                                                       info.height,
                                                       info.mipLevels,
                                                       info.depth,
                                                       GetDXGIResourceFormat( info.format ),
                                                       false,
                                                       true,
                                                       false};

        buffer_ptr = new D3D11Texture2DArray( m_pDevice, create_info );
        break;
    }
    default:
        break;
    }
    return buffer_ptr != nullptr;
}

bool rh::engine::D3D11GPUAllocator::FreeImageBuffer( void *buffer, ImageBufferType type )
{
    switch ( type ) {
    case ImageBufferType::Unknown:
        break;
    case ImageBufferType::BackBuffer:
        break;
    case ImageBufferType::TextureBuffer:
        delete static_cast<D3D11Texture2D *>( buffer );
        break;
    case ImageBufferType::DynamicTextureArrayBuffer:
        delete static_cast<D3D11Texture2DArray *>( buffer );
        break;
    case ImageBufferType::DepthBuffer:
        delete static_cast<D3D11DepthStencilBuffer *>( buffer );
        break;
    case ImageBufferType::RenderTargetBuffer:
        break;
    }
    return true;
}

bool rh::engine::D3D11GPUAllocator::AllocateVertexBuffer( const VertexBufferInfo &info,
                                                          IGPUResource *&buffer_ptr )
{
    buffer_ptr = nullptr;

    if ( info.isDynamic ) {
        buffer_ptr = new D3D11DynamicVertexBuffer( m_pDevice, info.vertexSize, info.vertexCount );
    } else {
        buffer_ptr = new D3D11VertexBuffer( m_pDevice,
                                            info.vertexSize * info.vertexCount,
                                            static_cast<const D3D11_SUBRESOURCE_DATA *>(
                                                info.initialData ) );
    }
    return buffer_ptr != nullptr;
}

bool rh::engine::D3D11GPUAllocator::AllocateIndexBuffer( const IndexBufferInfo &info,
                                                         IGPUResource *&buffer_ptr )
{
    buffer_ptr = nullptr;

    if ( info.isDynamic ) {
        buffer_ptr = new D3D11DynamicIndexBuffer( m_pDevice, info.indexCount );
    } else {
        buffer_ptr = new D3D11IndexBuffer( m_pDevice,
                                           info.indexCount,
                                           reinterpret_cast<const D3D11_SUBRESOURCE_DATA *>(
                                               info.initialData ) );
    }
    return buffer_ptr != nullptr;
}

bool rh::engine::D3D11GPUAllocator::AllocateInputLayout( const InputLayoutInfo &info,
                                                         IGPUResource *&buffer_ptr )
{
    buffer_ptr = nullptr;
    std::vector<D3D11_INPUT_ELEMENT_DESC> internal_input_layout;
    internal_input_layout.reserve( info.inputElements.size() );
    uint32_t stride = 0;
    for ( auto &&input_element : info.inputElements ) {
        auto &&[format, size] = GetVertexFormat( input_element.type );

        D3D11_INPUT_ELEMENT_DESC &&el
            = {input_element.semantic.c_str(), 0, format, 0, stride, D3D11_INPUT_PER_VERTEX_DATA, 0};

        internal_input_layout.push_back( el );

        stride += size;
    }
    buffer_ptr = new D3D11InputLayout( m_pDevice,
                                       internal_input_layout,
                                       reinterpret_cast<D3D11VertexShader *>( info.shaderPtr ) );
    return buffer_ptr != nullptr;
}

bool rh::engine::D3D11GPUAllocator::AllocateSampler( const Sampler & /*info*/, void *&buffer_ptr )
{
    // TODO: ADD ACTUAL CONVERSION
    D3D11_SAMPLER_DESC sampler_desc{};
    sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC;
    sampler_desc.MaxAnisotropy = 16;
    sampler_desc.BorderColor[0] = 1.0F;
    sampler_desc.BorderColor[1] = 1.0F;
    sampler_desc.BorderColor[2] = 1.0F;
    sampler_desc.BorderColor[3] = 1.0F;
    sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampler_desc.MinLOD = 0;
    sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

    return CALL_D3D_API( m_pDevice->CreateSamplerState( &sampler_desc,
                                                        reinterpret_cast<ID3D11SamplerState **>(
                                                            &buffer_ptr ) ),
                         TEXT( "Failed to create sampler state!" ) );
}

bool rh::engine::D3D11GPUAllocator::AllocateShader( const ShaderInfo &info,
                                                    IGPUResource *&buffer_ptr )
{
    buffer_ptr = nullptr;

    switch ( info.shaderType ) {
    case ShaderStage::Compute:
        buffer_ptr = new D3D11ComputeShader( m_pDevice, info.filePath, info.entrypoint );
        break;
    case ShaderStage::Pixel:
        buffer_ptr = new D3D11PixelShader( m_pDevice, info.filePath, info.entrypoint );
        break;
    case ShaderStage::Vertex:
        buffer_ptr = new D3D11VertexShader( m_pDevice, info.filePath, info.entrypoint );
        break;
    default:
        break;
    }
    return buffer_ptr != nullptr;
}

bool rh::engine::D3D11GPUAllocator::AllocateDepthStencilState( const DepthStencilState &info,
                                                               void *&buffer_ptr )
{
    buffer_ptr = nullptr;
    D3D11_DEPTH_STENCIL_DESC desc = GetD3D11DepthStencilState( info );
    return CALL_D3D_API( m_pDevice->CreateDepthStencilState(
                             &desc, reinterpret_cast<ID3D11DepthStencilState **>( &buffer_ptr ) ),
                         TEXT( "Failed to create depth stencil state!" ) );
}

bool rh::engine::D3D11GPUAllocator::AllocateBlendState( const BlendState &info, void *&buffer_ptr )
{
    buffer_ptr = nullptr;
    D3D11_BLEND_DESC desc = GetD3D11BlendState( info );

    return CALL_D3D_API( m_pDevice->CreateBlendState( &desc,
                                                      reinterpret_cast<ID3D11BlendState **>(
                                                          &buffer_ptr ) ),
                         TEXT( "Failed to create blend state!" ) );
}

bool rh::engine::D3D11GPUAllocator::AllocateRasterizerState( const RasterizerState &info,
                                                             void *&buffer_ptr )
{
    buffer_ptr = nullptr;
    D3D11_RASTERIZER_DESC desc = GetD3D11RasterizerState( info );

    return CALL_D3D_API( m_pDevice->CreateRasterizerState( &desc,
                                                           reinterpret_cast<ID3D11RasterizerState **>(
                                                               &buffer_ptr ) ),
                         TEXT( "Failed to create rasterizer state!" ) );
}

bool rh::engine::D3D11GPUAllocator::AllocateConstantBuffer( const ConstantBufferInfo &info,
                                                            IGPUResource *&buffer_ptr )
{
    buffer_ptr = nullptr;
    buffer_ptr = new D3D11ConstantBuffer( m_pDevice, info );
    return buffer_ptr != nullptr;
}

bool rh::engine::D3D11GPUAllocator::AllocateDeferredContext( IGPUResource *&context_ptr )
{
    auto *renderingContext = new D3D11RenderingContext();
    ID3D11DeviceContext *d3dContext = nullptr;
    if ( !CALL_D3D_API( m_pDevice->CreateDeferredContext( 0, &d3dContext ),
                        TEXT( "Failed to create deferred context!" ) ) )
        return false;
    renderingContext->Init( d3dContext, this );

    context_ptr = renderingContext;
    return context_ptr != nullptr;
}

bool rh::engine::D3D11GPUAllocator::AllocateStructuredBuffer(
    const rh::engine::StructuredBufferInfo &info, IGPUResource *&buffer_ptr )
{
    buffer_ptr = nullptr;
    buffer_ptr = new D3D11StructuredBuffer( m_pDevice, info );
    return buffer_ptr != nullptr;
}

ID3D11Device *rh::engine::D3D11GPUAllocator::GetDevice()
{
    return m_pDevice;
}

rh::engine::D3D11GPUAllocator::~D3D11GPUAllocator() = default;
