cmake_minimum_required(VERSION 3.12)
set (CMAKE_CXX_STANDARD 20)

project (RenderHookEngineLibrary)

include_directories(
    /
    ${DEPENDENCY_INCLUDE_LIST}
    )

set(SOURCES
    Engine/RendererFactory.cpp

    Engine/Common/IStateCacheObject.cpp

    #Engine/D3D12Impl/D3D12Renderer.cpp
    #Engine/D3D12Impl/d3d12_device_state.cpp

    #Engine/VulkanImpl/VulkanRenderer.cpp
    #Engine/VulkanImpl/ImageBuffers/VulkanBackBuffer.cpp

    Engine/VulkanImpl/VulkanDeviceState.cpp
    Engine/VulkanImpl/VulkanRenderPass.cpp
    Engine/VulkanImpl/VulkanDeviceOutputView.cpp
    Engine/VulkanImpl/VulkanImageView.cpp
    Engine/VulkanImpl/VulkanCommandBuffer.cpp
    Engine/VulkanImpl/VulkanFrameBuffer.cpp
    Engine/VulkanImpl/VulkanWin32Window.cpp
    Engine/VulkanImpl/VulkanSwapchain.cpp
    Engine/VulkanImpl/VulkanShader.cpp
    Engine/VulkanImpl/VulkanPipeline.cpp
    Engine/VulkanImpl/VulkanBuffer.cpp
    Engine/VulkanImpl/SyncPrimitives/VulkanGPUSyncPrimitive.cpp
    Engine/VulkanImpl/SyncPrimitives/VulkanCPUSyncPrimitive.cpp

    Engine/D3D11Impl/D3D11Renderer.cpp
    Engine/D3D11Impl/D3D11DeviceState.cpp
    Engine/D3D11Impl/D3D11RenderingContext.cpp
    Engine/D3D11Impl/D3D11CommandBuffer.cpp
    Engine/D3D11Impl/D3D11GPUAllocator.cpp
    Engine/D3D11Impl/D3D11Convert.cpp
    Engine/D3D11Impl/D3D11InputLayout.cpp
    Engine/D3D11Impl/D3D11DeviceOutputView.cpp
    Engine/D3D11Impl/D3D11Im2DPipeline.cpp
    Engine/D3D11Impl/D3D11PrimitiveBatch.cpp
    Engine/D3D11Impl/D3D11Window.cpp
    Engine/D3D11Impl/D3D11Swapchain.cpp
    Engine/D3D11Impl/D3D11ImageView.cpp
    Engine/D3D11Impl/D3D11Framebuffer.cpp
    Engine/D3D11Impl/D3D11RenderPass.cpp

    Engine/D3D11Impl/RenderStateCache/D3D11RenderStateCache.cpp
    Engine/D3D11Impl/RenderStateCache/D3D11RasterizerStateCache.cpp
    Engine/D3D11Impl/RenderStateCache/D3D11SampleStateCache.cpp
    Engine/D3D11Impl/RenderStateCache/D3D11ShaderResourceCache.cpp
    Engine/D3D11Impl/RenderStateCache/D3D11BlendStateCache.cpp
    Engine/D3D11Impl/RenderStateCache/D3D11DepthStencilStateCache.cpp
    Engine/D3D11Impl/RenderStateCache/D3D11RenderTargetCache.cpp

    Engine/D3D11Impl/Buffers/D3D11Buffer.cpp
    Engine/D3D11Impl/Buffers/D3D11IndexBuffer.cpp
    Engine/D3D11Impl/Buffers/D3D11VertexBuffer.cpp
    Engine/D3D11Impl/Buffers/D3D11DynamicIndexBuffer.cpp
    Engine/D3D11Impl/Buffers/D3D11DynamicVertexBuffer.cpp
    Engine/D3D11Impl/Buffers/D3D11ConstantBuffer.cpp
    Engine/D3D11Impl/Buffers/D3D11StructuredBuffer.cpp

    Engine/D3D11Impl/ImageBuffers/D3D11Texture2D.cpp
    Engine/D3D11Impl/ImageBuffers/D3D11DepthStencilBuffer.cpp
    Engine/D3D11Impl/ImageBuffers/D3D11BackBuffer.cpp
    Engine/D3D11Impl/ImageBuffers/D3D11Texture2DArray.cpp
    #Engine/D3D11Impl/ImageBuffers/D3D11RenderTarget.cpp

    Engine/D3D11Impl/Shaders/D3D11Shader.cpp
    Engine/D3D11Impl/Shaders/D3D11VertexShader.cpp
    Engine/D3D11Impl/Shaders/D3D11PixelShader.cpp
    Engine/D3D11Impl/Shaders/D3D11ComputeShader.cpp

    TestUtils/TestSample.cpp
    TestUtils/WindowsSampleWrapper.cpp

    DebugUtils/DebugLogger.cpp

    ImGUI/imgui.cpp
    ImGUI/imgui_draw.cpp
    ImGUI/imgui_widgets.cpp
    ImGUI/imgui_impl_dx11.cpp
    ImGUI/imgui_impl_win32.cpp
    ImGUI/imgui_impl_vulkan.cpp
    )

add_library( RHEngineLib STATIC ${SOURCES} )

set_property(TARGET RHEngineLib PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_link_libraries( RHEngineLib ${DEPENDENCY_LIB_LIST} )
target_compile_options( RHEngineLib PRIVATE -ftime-trace )
