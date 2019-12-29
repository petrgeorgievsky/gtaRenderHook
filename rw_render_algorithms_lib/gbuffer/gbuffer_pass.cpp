#include "gbuffer_pass.h"
#include <Engine/Common/IRenderingContext.h>
#include <Engine/Common/types/image_bind_type.h>
#include <Engine/Common/types/image_buffer_info.h>
#include <Engine/Common/types/image_buffer_type.h>
#include <Engine/Common/types/image_clear_type.h>
#include <Engine/IRenderer.h>
#include <array>
using namespace rh::engine;

GBufferPass::GBufferPass( const GBufferDesc &desc ) : mCurrentGBufferDesc( desc )
{
    IGPUAllocator *allocator = g_pRHRenderer->GetGPUAllocator();

    mGBufferTextures.reserve( mCurrentGBufferDesc.bufferFormats.size() );

    ImageBufferInfo createInfo_{};
    createInfo_.type = ImageBufferType::RenderTargetBuffer;
    createInfo_.width = mCurrentGBufferDesc.width;
    createInfo_.height = mCurrentGBufferDesc.height;
    createInfo_.mipLevels = 1;

    for ( uint32_t format : mCurrentGBufferDesc.bufferFormats ) {
        createInfo_.format = static_cast<ImageBufferFormat>( format );
        IGPUResource *gbuffer_tex = nullptr;
        if ( allocator->AllocateImageBuffer( createInfo_, gbuffer_tex ) && gbuffer_tex != nullptr )
            mGBufferTextures.emplace_back( gbuffer_tex );
    }
}

GBufferPass::~GBufferPass()
{
    //    IGPUAllocator *allocator = g_pRHRenderer->GetGPUAllocator();

    //    for ( void *texture : mGBufferTextures )
    //        allocator->FreeImageBuffer( texture, ImageBufferType::RenderTargetBuffer );
}

void GBufferPass::PrepareFrame( IRenderingContext *context )
{
    std::array<float, 4> clear_color = {0, 0, 0, 0};
    context->BindViewPorts( {{0,
                              0,
                              static_cast<float>( mCurrentGBufferDesc.width ),
                              static_cast<float>( mCurrentGBufferDesc.height ),
                              0.0F,
                              1.0F}} );
    for ( const auto &texture : mGBufferTextures )
        context->ClearImageBuffer( ImageClearType::Color, texture.mPtr, clear_color );

    std::vector<IndexPtrPair> ptr_pair_list;
    ptr_pair_list.reserve( mGBufferTextures.size() );
    uint32_t i = 0;
    for ( const auto &texture : mGBufferTextures )
        ptr_pair_list.push_back( {i++, texture.mPtr} );

    context->BindImageBuffers( ImageBindType::RenderTarget, ptr_pair_list );
}

void GBufferPass::EndFrame( IRenderingContext *context )
{
    std::vector<IndexPtrPair> ptr_pair_list;
    ptr_pair_list.reserve( mGBufferTextures.size() );
    void *empty_ptr = nullptr;
    for ( uint32_t i = 0; i < mGBufferTextures.size(); i++ )
        ptr_pair_list.push_back( {i, empty_ptr} );

    context->BindImageBuffers( ImageBindType::RenderTarget, ptr_pair_list );
}

IGPUResource *GBufferPass::GetGBuffer( uint32_t id )
{
    return mGBufferTextures[id].mPtr;
}

void GBufferPass::UpdateSize( uint32_t w, uint32_t h )
{
    IGPUAllocator *allocator = g_pRHRenderer->GetGPUAllocator();

    mGBufferTextures.clear();

    mCurrentGBufferDesc.width = w;
    mCurrentGBufferDesc.height = h;

    mGBufferTextures.reserve( mCurrentGBufferDesc.bufferFormats.size() );

    ImageBufferInfo createInfo_{};
    createInfo_.type = ImageBufferType::RenderTargetBuffer;
    createInfo_.width = mCurrentGBufferDesc.width;
    createInfo_.height = mCurrentGBufferDesc.height;
    createInfo_.mipLevels = 1;

    for ( uint32_t format : mCurrentGBufferDesc.bufferFormats ) {
        createInfo_.format = static_cast<ImageBufferFormat>( format );
        IGPUResource *gbuffer_tex = nullptr;
        if ( allocator->AllocateImageBuffer( createInfo_, gbuffer_tex ) && gbuffer_tex != nullptr )
            mGBufferTextures.emplace_back( gbuffer_tex );
    }
}
