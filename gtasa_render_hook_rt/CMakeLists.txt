cmake_minimum_required(VERSION 3.12)
set (CMAKE_CXX_STANDARD 17)

project (gtasa_render_hook_rt)
option (USE_VULKAN_API "Add RenderHook Vulkan API implementation" ON)
set(GTA_SA_FOLDER CACHE PATH "GTA SA folder")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DLL")

include_directories(
    ../rw_ray_tracing_lib
    ../rw_rh_engine_lib
    ../RHEngineLib
    ${DEPENDENCY_INCLUDE_LIST}
    )

set(SOURCES
    dllmain.cpp
    forward_pbr_pipeline.cpp
    gbuffer_pipeline.cpp
    ray_tracer.cpp
    bilateralblurpass.cpp
    )

set ( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON )

add_library( gtasa_render_hook_rt SHARED ${SOURCES})
set_target_properties( gtasa_render_hook_rt PROPERTIES SUFFIX ".asi" )
set_target_properties( gtasa_render_hook_rt PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY_DEBUG ${GTA_SA_FOLDER}/
                      RUNTIME_OUTPUT_DIRECTORY_RELEASE ${GTA_SA_FOLDER}/
)

target_link_libraries( gtasa_render_hook_rt ${DEPENDENCY_LIB_LIST} RHEngineLib rw_rh_engine_lib rw_ray_tracing_lib)

#add_custom_command(TARGET gtasa_render_hook_rt
#                   POST_BUILD
#                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gtasa_render_hook_rt> ${GTA_SA_FOLDER}/)
