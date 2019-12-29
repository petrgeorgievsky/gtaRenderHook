cmake_minimum_required(VERSION 3.12)
set (CMAKE_CXX_STANDARD 17)

project (rw_ray_tracing_lib)

include_directories(
    /
    ../RHEngineLib/
    $ENV{PLUGIN_SDK_DIR}/plugin_sa
    ../rw_rh_engine_lib/
    ${DEPENDENCY_INCLUDE_LIST}
    )
set(SOURCES
    bvh_builder.cpp
    ray_tracer.cpp
    ray_tracing_scene_cache.cpp
    ray_tracing_texture_cache.cpp
    ray_traced_shadows/rt_shadows_pass.cpp
    ray_traced_gi/irradiance_fields_gi_pass.cpp
    ray_traced_gi/per_pixel_gi_pass.cpp
    ray_traced_gi/lf_gi_filter_pass.cpp
    )
add_library( rw_ray_tracing_lib STATIC ${SOURCES} )

set_property(TARGET rw_ray_tracing_lib PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_link_libraries( rw_ray_tracing_lib ${DEPENDENCY_LIB_LIST} RHEngineLib rw_rh_engine_lib )
target_compile_options( rw_ray_tracing_lib PRIVATE )
