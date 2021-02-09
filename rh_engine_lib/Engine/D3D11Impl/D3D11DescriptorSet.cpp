#include "D3D11DescriptorSet.h"
#include "D3D11DescriptorSetLayout.h"
#include <DebugUtils/DebugLogger.h>
#include <d3d11.h>
#include <functional>

namespace rh::engine
{

D3D11DescriptorSet::D3D11DescriptorSet(
    const D3D11DescriptorSetCreateParams &desc )
    : mLayout( dynamic_cast<D3D11DescriptorSetLayout *>( desc.mLayout ) )
{
    auto bindings = mLayout->mDescription;

    for ( auto binding : bindings )
    {
        D3D11ShaderResourceBinding current_cb_binding{};
        switch ( binding.mDescriptorType )
        {
        case rh::engine::DescriptorType::Sampler:
            current_cb_binding.mResourceType = D3DDescriptorType::Sampler;
            break;
        case rh::engine::DescriptorType::ROBuffer:
            current_cb_binding.mResourceType =
                D3DDescriptorType::ConstantBuffer;
            break;
        case rh::engine::DescriptorType::ROTexture:
        case rh::engine::DescriptorType::RWTexture:
        case rh::engine::DescriptorType::RWBuffer:
            current_cb_binding.mResourceType =
                D3DDescriptorType::ShaderResource;
            break;
        default:
            rh::debug::DebugLogger::Error(
                "Unsupported descriptor type bound to dx11 descriptor set" );
            std::terminate();
        }

        current_cb_binding.mDescriptorType = binding.mDescriptorType;
        current_cb_binding.mStartId        = binding.mRegisterId;
        current_cb_binding.mShaderStages |= binding.mShaderStages;
        current_cb_binding.mResources.push_back( nullptr );

        mBindings.push_back( current_cb_binding );
        mBindingMap[binding.mBindingId] = std::make_pair<int, int>(
            (int)mBindings.size() - 1,
            (int)current_cb_binding.mResources.size() - 1 );
    }
}

D3D11DescriptorSet::~D3D11DescriptorSet() = default;

void D3D11DescriptorSet::BindToContext( ID3D11DeviceContext *context )
{
    using bind_resources_cmd =
        std::function<void( ID3D11DeviceContext * ctx, uint32_t start_id,
                            uint32_t count, void *const *data_ptr )>;

    static std::array<bind_resources_cmd, 3> vs_bind_cmds{
        []( ID3D11DeviceContext *ctx, uint32_t start_id, uint32_t count,
            void *const *data_ptr ) {
            ctx->VSSetConstantBuffers(
                start_id, count,
                reinterpret_cast<ID3D11Buffer *const *>( data_ptr ) );
        },
        []( ID3D11DeviceContext *ctx, uint32_t start_id, uint32_t count,
            void *const *data_ptr ) {
            ctx->VSSetShaderResources(
                start_id, count,
                reinterpret_cast<ID3D11ShaderResourceView *const *>(
                    data_ptr ) );
        },
        []( ID3D11DeviceContext *ctx, uint32_t start_id, uint32_t count,
            void *const *data_ptr ) {
            ctx->VSSetSamplers(
                start_id, count,
                reinterpret_cast<ID3D11SamplerState *const *>( data_ptr ) );
        } };

    static std::array<bind_resources_cmd, 3> ps_bind_cmds{
        []( ID3D11DeviceContext *ctx, uint32_t start_id, uint32_t count,
            void *const *data_ptr ) {
            ctx->PSSetConstantBuffers(
                start_id, count,
                reinterpret_cast<ID3D11Buffer *const *>( data_ptr ) );
        },
        []( ID3D11DeviceContext *ctx, uint32_t start_id, uint32_t count,
            void *const *data_ptr ) {
            ctx->PSSetShaderResources(
                start_id, count,
                reinterpret_cast<ID3D11ShaderResourceView *const *>(
                    data_ptr ) );
        },
        []( ID3D11DeviceContext *ctx, uint32_t start_id, uint32_t count,
            void *const *data_ptr ) {
            ctx->PSSetSamplers(
                start_id, count,
                reinterpret_cast<ID3D11SamplerState *const *>( data_ptr ) );
        } };

    // yay finally I can use this forbidden C++ pointer to member stuff!
    for ( auto binding : mBindings )
    {
        if ( binding.mShaderStages & ShaderStage::Vertex )
        {
            vs_bind_cmds[static_cast<int32_t>( binding.mResourceType )](
                context, binding.mStartId,
                static_cast<uint32_t>( binding.mResources.size() ),
                binding.mResources.data() );
        }
        if ( binding.mShaderStages & ShaderStage::Pixel )
        {
            ps_bind_cmds[static_cast<int32_t>( binding.mResourceType )](
                context, binding.mStartId,
                static_cast<uint32_t>( binding.mResources.size() ),
                binding.mResources.data() );
        }
    }
}

void D3D11DescriptorSet::UpdateDescriptorBinding(
    uint32_t id, ID3D11DeviceChild *descriptor )
{
    auto [cb, buff_id]                = mBindingMap[id];
    mBindings[cb].mResources[buff_id] = descriptor;
}
DescriptorType D3D11DescriptorSet::GetType( uint32_t binding_id )
{
    auto [cb, resource] = mBindingMap[binding_id];
    return mBindings[cb].mDescriptorType;
}
} // namespace rh::engine