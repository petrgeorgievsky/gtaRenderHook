cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 17)

project(rw_render_algorithms_lib)

include_directories(
        /
        ../RHEngineLib/
        $ENV{PLUGIN_SDK_DIR}/plugin_sa
        ../rw_rh_engine_lib/
        ${DEPENDENCY_INCLUDE_LIST}
)
set(SOURCES
        gbuffer/gbuffer_pipeline.cpp
        gbuffer/gbuffer_pass.cpp
        deferred_rendering.cpp
        )
add_library(rw_render_algorithms_lib STATIC ${SOURCES})

#set_property(TARGET rw_render_algorithms_lib PROPERTY
#             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_link_libraries(rw_render_algorithms_lib ${DEPENDENCY_LIB_LIST} RHEngineLib rw_rh_engine_lib)
target_compile_options(rw_render_algorithms_lib PRIVATE)
