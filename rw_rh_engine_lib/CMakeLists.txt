cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)

project(rw_rh_engine_lib)

include_directories(
        /
        ../rh_engine_lib/
        ${DEPENDENCY_INCLUDE_LIST}
)
set(SOURCES
        rw_engine/rw_stream/rw_stream.cpp
        rw_engine/rw_tex_dict/rw_tex_dict.cpp
        rw_engine/rw_camera/rw_camera.cpp
        rw_engine/rw_raster/rw_raster.cpp
        rw_engine/rw_frame/rw_frame.cpp
        rw_engine/rw_texture/rw_texture.cpp
        rw_engine/rh_backend/raster_backend.cpp
        rw_engine/rh_backend/camera_backend.cpp
        rw_engine/rh_backend/im2d_backend.cpp
        rw_engine/rp_clump/rp_clump.cpp
        rw_engine/rp_atomic/rp_atomic.cpp
        rw_engine/rw_frame/rw_framelist.cpp
        rw_engine/rp_geometry/rp_geometry.cpp
        rw_engine/rp_matlist/rp_matlist.cpp
        rw_engine/rp_material/rp_material.cpp
        rw_engine/rp_mesh/rp_mesh.cpp
        rw_engine/rw_image/rw_image_funcs.cpp
        rw_engine/rw_standard_render_commands/camerabeginupdatecmd.cpp
        rw_engine/rw_standard_render_commands/cameraendupdatecmd.cpp
        rw_engine/rw_standard_render_commands/cameraclearcmd.cpp
        rw_engine/rw_standard_render_commands/rastershowrastercmd.cpp
        rw_engine/rw_standard_render_commands/rastercreatecmd.cpp
        rw_engine/rw_standard_render_commands/rasterdestroycmd.cpp
        rw_engine/rw_standard_render_commands/nativetexturereadcmd.cpp
        rw_engine/rw_standard_render_commands/rastersetimagecmd.cpp
        rw_engine/rw_standard_render_commands/imagefindrasterformat.cpp
        rw_engine/rw_standard_render_commands/rasterlockcmd.cpp
        rw_engine/rw_standard_render_commands/rasterunlockcmd.cpp
        rw_engine/rw_rh_convert_funcs.cpp
        rw_engine/rw_api_injectors.cpp
        rw_engine/rw_rh_pipeline.cpp
        rwtestsample.cpp
        render_loop.cpp
        #[[RwRenderEngine.cpp
        rw_engine/global_definitions.cpp
        rw_engine/rw_macro_constexpr.cpp
        rw_engine/rw_im2d/rw_im2d.cpp
]]
        rw_game_hooks.cpp
        rw_engine/system_funcs/rw_device_system_handler.cpp
        scene_graph.cpp ipc/ipc_utils.cpp ipc/shared_memory_queue_client.cpp
        ipc/shared_memory_queue_client.h rw_engine/rh_backend/mesh_rendering_backend.cpp
        rendering_loop/ray_tracing/RayTracingRenderer.cpp
        rendering_loop/deferred_render/GBufferPass.cpp
        rw_engine/rh_backend/material_backend.cpp
        rw_engine/rw_rh_skin_pipeline.cpp
        rw_engine/rh_backend/skinned_mesh_backend.cpp
        rendering_loop/ray_tracing/scene_description/gpu_texture_pool.cpp
        rendering_loop/ray_tracing/scene_description/gpu_mesh_buffer_pool.cpp
        rendering_loop/compute_skin_animation.cpp
        rw_engine/rh_backend/Im2DRenderer.cpp
        ipc/MemoryWriter.cpp
        ipc/MemoryReader.cpp
        rendering_loop/ray_tracing/RTBlasBuildPass.cpp
        rendering_loop/ray_tracing/RTTlasBuildPass.cpp
        rendering_loop/ray_tracing/RTSceneDescription.cpp
        rendering_loop/ray_tracing/RTPrimaryRaysPass.cpp
        rendering_loop/ray_tracing/RTAOPass.cpp
        rendering_loop/ray_tracing/CameraDescription.cpp
        rendering_loop/DescriptorGenerator.cpp
        rendering_loop/ray_tracing/utils.cpp
        rendering_loop/ray_tracing/BilateralFilterPass.cpp
        rendering_loop/ray_tracing/DeferredCompositionPass.cpp
        rendering_loop/ray_tracing/ComputeVelocityBuffer.cpp
        rendering_loop/ray_tracing/RTShadowsPass.cpp
        rendering_loop/main_renderer.cpp
        rendering_loop/ray_tracing/VarAwareTempAccumFilter.cpp
        rendering_loop/ray_tracing/VarAwareTempAccumFilterColor.cpp
        rendering_loop/ray_tracing/scene_description/gpu_scene_materials_pool.cpp
        rendering_loop/ray_tracing/tiled_light_culling.cpp
        rendering_loop/ray_tracing/RTReflectionRaysPass.cpp
        rendering_loop/ray_tracing/debug_pipeline.cpp
        rw_engine/rh_backend/im3d_backend.cpp
        rw_engine/rh_backend/im3d_renderer.cpp
        rw_engine/system_funcs/raster_lock_cmd.cpp
        rw_engine/system_funcs/start_cmd.cpp
        rw_engine/system_funcs/stop_cmd.cpp
        rw_engine/system_funcs/get_adapter_count_cmd.cpp
        rw_engine/system_funcs/get_adapter_info_cmd.cpp
        rw_engine/system_funcs/set_adapter_cmd.cpp
        rw_engine/system_funcs/get_adapter_cmd.cpp
        rw_engine/system_funcs/get_video_mode_count.cpp
        rw_engine/system_funcs/get_video_mode_cmd.cpp
        rw_engine/system_funcs/set_video_mode_cmd.cpp
        rw_engine/system_funcs/get_video_mode_info_cmd.cpp
        render_driver/render_driver.cpp
        render_client/render_client.cpp
        material_storage.cpp)
add_library(rw_rh_engine_lib STATIC ${SOURCES})
#set_property(TARGET rw_rh_engine_lib PROPERTY
#        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")


#target_compile_options(${PROJECT_NAME} PRIVATE -fno-omit-frame-pointer -fsanitize=address)
#target_link_options(${PROJECT_NAME} PRIVATE -fno-omit-frame-pointer -fsanitize=address)

if (USE_VULKAN_API)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DUSE_VULKAN_API)
endif ()

target_link_libraries(rw_rh_engine_lib ${DEPENDENCY_LIB_LIST} rh_engine_lib imgui::imgui)
target_compile_options(rw_rh_engine_lib PRIVATE)
