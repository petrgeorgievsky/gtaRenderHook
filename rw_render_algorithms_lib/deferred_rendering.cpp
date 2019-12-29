#include "deferred_rendering.h"
#include <Engine/Common/types/shader_info.h>
#include <Engine/Common/types/shader_stage.h>
#include <Engine/IRenderer.h>

DeferredRendering::DeferredRendering()
{
    rh::engine::IGPUAllocator *allocator = rh::engine::g_pRHRenderer->GetGPUAllocator();
    allocator->AllocateShader( {TEXT( "shaders/d3d11/deferred_rendering.hlsl" ),
                                TEXT( "CompositePS" ),
                                rh::engine::ShaderStage::Pixel},
                               m_pCompositePS );
}
