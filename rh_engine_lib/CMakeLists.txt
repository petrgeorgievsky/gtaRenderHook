cmake_minimum_required(VERSION 3.10.2)
set(CMAKE_CXX_STANDARD 20)

project(rh_engine_lib)

include_directories(
        /
        ${DEPENDENCY_INCLUDE_LIST}
)

find_package(imgui CONFIG REQUIRED)

set(VULKAN_SOURCES

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
        Engine/VulkanImpl/VulkanPipelineLayout.cpp
        Engine/VulkanImpl/VulkanDescriptorSetLayout.cpp
        Engine/VulkanImpl/VulkanBuffer.cpp
        Engine/VulkanImpl/VulkanMemoryAllocator.cpp
        Engine/VulkanImpl/VulkanDescriptorSetAllocator.cpp
        Engine/VulkanImpl/VulkanDescriptorSet.cpp
        Engine/VulkanImpl/VulkanImage.cpp
        Engine/VulkanImpl/VulkanSampler.cpp
        Engine/VulkanImpl/SyncPrimitives/VulkanGPUSyncPrimitive.cpp
        Engine/VulkanImpl/SyncPrimitives/VulkanCPUSyncPrimitive.cpp
        Engine/VulkanImpl/VulkanBottomLevelAccelerationStructure.cpp
        Engine/VulkanImpl/VulkanTopLevelAccelerationStructure.cpp
        Engine/VulkanImpl/VulkanRayTracingPipeline.cpp
        Engine/VulkanImpl/VulkanComputePipeline.cpp
        Engine/VulkanImpl/VulkanImGUI.cpp
        Engine/VulkanImpl/VulkanGPUInfo.cpp)

set(D3D11_SOURCES
        Engine/D3D11Impl/D3D11DeviceState.cpp
        Engine/D3D11Impl/D3D11CommandBuffer.cpp
        Engine/D3D11Impl/D3D11Convert.cpp
        Engine/D3D11Impl/D3D11InputLayout.cpp
        Engine/D3D11Impl/D3D11DeviceOutputView.cpp
        Engine/D3D11Impl/D3D11PrimitiveBatch.cpp
        Engine/D3D11Impl/D3D11Window.cpp
        Engine/D3D11Impl/D3D11Swapchain.cpp
        Engine/D3D11Impl/D3D11ImageView.cpp
        Engine/D3D11Impl/D3D11ImageBuffer.cpp
        Engine/D3D11Impl/D3D11Framebuffer.cpp
        Engine/D3D11Impl/D3D11RenderPass.cpp
        Engine/D3D11Impl/D3D11Shader.cpp
        Engine/D3D11Impl/D3D11Sampler.cpp
        Engine/D3D11Impl/D3D11DescriptorSetAllocator.cpp
        Engine/D3D11Impl/D3D11DescriptorSetLayout.cpp
        Engine/D3D11Impl/D3D11DescriptorSet.cpp
        Engine/D3D11Impl/D3D11Buffer.cpp
        Engine/D3D11Impl/D3D11Pipeline.cpp

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
        )

# android support for future
if (ANDROID)
    set(SOURCES ${SOURCES} ${VULKAN_SOURCES})
else ()
    set(SOURCES ${SOURCES}
            ${D3D11_SOURCES}
            Engine/Common/IStateCacheObject.cpp
            Engine/Win32NativeWindow.cpp
            TestUtils/WindowsSampleWrapper.cpp
            )
    if (USE_VULKAN_API)
        set(SOURCES ${SOURCES} ${VULKAN_SOURCES}
                #[[ImGUI/imgui_impl_vulkan.cpp]])
    endif ()
endif ()

set(SOURCES ${SOURCES}
        TestUtils/TestSample.cpp
        DebugUtils/DebugLogger.cpp
        Engine/VulkanImpl/VulkanDebugUtils.cpp
        ConfigUtils/ConfigurationManager.cpp
        Engine/EngineConfigBlock.cpp
        TestUtils/BitmapLoader.cpp ConfigUtils/ConfigBlock.h ConfigUtils/Serializable.cpp ConfigUtils/Serializable.h Engine/FastPimpl.h DebugUtils/Win32UncaughtExceptionHandler.cpp DebugUtils/Win32UncaughtExceptionHandler.h)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

if (ANDROID)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DANDROID_BUILD -DVK_USE_PLATFORM_ANDROID_KHR)
else ()
    #set_property(TARGET ${PROJECT_NAME} PROPERTY
    #        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()

if (USE_VULKAN_API)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DUSE_VULKAN_API)
endif ()
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

#target_compile_options(${PROJECT_NAME} PRIVATE -fno-omit-frame-pointer -fsanitize=address)
#target_link_options(${PROJECT_NAME} PRIVATE -fno-omit-frame-pointer -fsanitize=address)

target_link_libraries(${PROJECT_NAME} ${DEPENDENCY_LIB_LIST}
        imgui::imgui
        unofficial::vulkan-memory-allocator::vulkan-memory-allocator
        nlohmann_json::nlohmann_json
        )

target_compile_options(${PROJECT_NAME} PRIVATE -ftime-trace)
if (NOT (CLANG_ANALYSER_PATH STREQUAL ""))
    add_custom_command(TARGET ${PROJECT_NAME}
            PRE_BUILD
            COMMAND "${CLANG_ANALYSER_PATH}/ClangBuildAnalyzer.exe" --start "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND "${CLANG_ANALYSER_PATH}/ClangBuildAnalyzer.exe" --stop "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/clang_analysis_result.json"
            COMMAND "${CLANG_ANALYSER_PATH}/ClangBuildAnalyzer.exe" --analyze "$<TARGET_FILE_DIR:${PROJECT_NAME}>/clang_analysis_result.json")
endif ()