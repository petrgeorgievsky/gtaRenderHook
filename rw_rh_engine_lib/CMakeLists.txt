cmake_minimum_required(VERSION 3.12)
set (CMAKE_CXX_STANDARD 20)

project (RW_RH_WrapperLibrary)

include_directories(
    /
    ../RHEngineLib/
    $ENV{PLUGIN_SDK_DIR}/plugin_sa
    ${DEPENDENCY_INCLUDE_LIST}
    )
set(SOURCES
    RwRenderEngine.cpp
    rwtestsample.cpp
    rw_engine/global_definitions.cpp
    rw_engine/rw_api_injectors.cpp
    rw_engine/rw_rh_pipeline.cpp
    rw_engine/rw_macro_constexpr.cpp
    rw_engine/rp_atomic/rp_atomic.cpp
    rw_engine/rw_frame/rw_frame.cpp
    rw_engine/rw_frame/rw_framelist.cpp
    rw_engine/rp_geometry/rp_geometry.cpp
    rw_engine/rp_matlist/rp_matlist.cpp
    rw_engine/rp_material/rp_material.cpp
    rw_engine/rp_mesh/rp_mesh.cpp
    rw_engine/rw_camera/rw_camera.cpp
    rw_engine/rw_raster/rw_raster.cpp
    rw_engine/rw_stream/rw_stream.cpp
    rw_engine/rw_texture/rw_texture.cpp
    rw_engine/rw_tex_dict/rw_tex_dict.cpp
    rw_engine/rw_im2d/rw_im2d.cpp
    rw_engine/rp_clump/rp_clump.cpp
    rw_engine/rw_standard_render_commands/camerabeginupdatecmd.cpp
    rw_engine/rw_standard_render_commands/cameraendupdatecmd.cpp
    rw_engine/rw_standard_render_commands/cameraclearcmd.cpp
    rw_engine/rw_standard_render_commands/rastershowrastercmd.cpp
    rw_engine/rw_standard_render_commands/rastersetimagecmd.cpp
    rw_engine/rw_standard_render_commands/rastercreatecmd.cpp
    rw_engine/rw_standard_render_commands/rasterdestroycmd.cpp
    rw_engine/rw_standard_render_commands/imagefindrasterformat.cpp
    rw_engine/rw_standard_render_commands/nativetexturereadcmd.cpp
    rw_engine/rw_standard_render_commands/rasterlockcmd.cpp
    rw_engine/rw_standard_render_commands/rasterunlockcmd.cpp
    rw_game_hooks.cpp
    rw_engine/rw_rh_convert_funcs.cpp
    rw_engine/rw_image/rw_image_funcs.cpp
    )
add_library( rw_rh_engine_lib STATIC ${SOURCES} )
set_property(TARGET rw_rh_engine_lib PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_link_libraries( rw_rh_engine_lib ${DEPENDENCY_LIB_LIST} RHEngineLib )
target_compile_options( rw_rh_engine_lib PRIVATE )
