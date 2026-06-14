cmake_minimum_required(VERSION 3.10.2)

project(rh_engine_lib)

find_package(imgui CONFIG REQUIRED)

set(VULKAN_SOURCES

        Engine/VulkanImpl/VulkanDeviceState.cpp
        Engine/VulkanImpl/VulkanRenderPass.cpp
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
        Engine/VulkanImpl/VulkanGPUInfo.cpp
        Engine/VulkanImpl/VulkanDebugUtils.cpp
        )

set(D3D11_SOURCES

        Engine/D3D11Impl/D3D11DeviceState.cpp
        Engine/D3D11Impl/D3D11CommandBuffer.cpp
        Engine/D3D11Impl/D3D11Convert.cpp
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
        )


add_library(${PROJECT_NAME} STATIC
        Engine/EngineConfigBlock.cpp
        TestUtils/TestSample.cpp
        TestUtils/BitmapLoader.cpp
        TestUtils/test_dump_util.cpp
        DebugUtils/DebugLogger.cpp
        DebugUtils/Win32UncaughtExceptionHandler.cpp
        ConfigUtils/ConfigurationManager.cpp
        ConfigUtils/Serializable.cpp)

if (NOT ANDROID)
    target_sources(${PROJECT_NAME} PRIVATE ${D3D11_SOURCES}
            Engine/Common/IStateCacheObject.cpp
            Engine/Win32NativeWindow.cpp
            TestUtils/WindowsSampleWrapper.cpp
    )
endif ()

if (USE_VULKAN_API)
    target_sources(${PROJECT_NAME} PRIVATE ${VULKAN_SOURCES})
endif ()

target_include_directories(${PROJECT_NAME} PUBLIC ./)

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 20
        UNITY_BUILD TRUE
        )

if (ANDROID)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DANDROID_BUILD -DVK_USE_PLATFORM_ANDROID_KHR)
endif ()

if (USE_VULKAN_API)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DUSE_VULKAN_API -DVULKAN_HPP_NO_EXCEPTIONS)
endif ()

set(RH_COMPILE_OPTIONS "")
set(RH_LINK_OPTIONS "")

if (USE_SANITIZERS)
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
    target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)

    if (ARCH_64BIT)
        target_link_libraries(${PROJECT_NAME}
                clang_rt.asan_dbg_dynamic-x86_64
                clang_rt.asan_dbg_dynamic_runtime_thunk-x86_64
                )
    else ()
        target_link_libraries(${PROJECT_NAME}
                clang_rt.asan_dbg_dynamic-i386
                clang_rt.asan_dbg_dynamic_runtime_thunk-i386
                )
    endif ()
endif ()

target_compile_options(${PROJECT_NAME} PRIVATE "$<$<CONFIG:DEBUG>:-D_DEBUG -ftime-trace>")

target_link_libraries(${PROJECT_NAME} PRIVATE
        imgui::imgui
        nlohmann_json::nlohmann_json
        )

if (USE_VULKAN_API)
    target_link_libraries(${PROJECT_NAME} PUBLIC
            Vulkan::Vulkan
            ${VMA_CMAKE_TARGET}
    )
endif ()

if (NOT ANDROID)
    target_link_libraries(${PROJECT_NAME} PRIVATE
            dxgi
            dbghelp
    )
endif ()

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