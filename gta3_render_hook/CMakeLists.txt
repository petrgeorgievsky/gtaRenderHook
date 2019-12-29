set (CMAKE_CXX_STANDARD 17)

project (gta3_render_hook)
option (USE_VULKAN_API "Add RenderHook Vulkan API implementation" ON)
set(GTA_3_FOLDER CACHE PATH "GTA 3 folder")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DLL")

include_directories(
    ../rw_rh_engine_lib
    ../RHEngineLib
    ../rw_ray_tracing_lib
    ../rw_render_algorithms_lib
    ${DEPENDENCY_INCLUDE_LIST}
    )

set(SOURCES
    dllmain.cpp
    forward_pbr_pipeline.cpp
    gta3_default_rendering_pipeline.cpp
    gta3_geometry_proxy.cpp
    )

#set ( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON )

add_library( gta3_render_hook SHARED ${SOURCES})
set_target_properties( gta3_render_hook PROPERTIES SUFFIX ".asi" )
set_target_properties( gta3_render_hook PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY_DEBUG ${GTA_3_FOLDER}/
                      RUNTIME_OUTPUT_DIRECTORY_RELEASE ${GTA_3_FOLDER}/
)
set_target_properties( gta3_render_hook PROPERTIES
                        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_link_libraries( gta3_render_hook ${DEPENDENCY_LIB_LIST}
    RHEngineLib rw_rh_engine_lib rw_ray_tracing_lib
    rw_render_algorithms_lib)

add_custom_command( TARGET gta3_render_hook
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                     ${CMAKE_SOURCE_DIR}/rw_ray_tracing_lib/shaders ${GTA_3_FOLDER}/shaders)

add_custom_command( TARGET gta3_render_hook
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/rw_ray_tracing_lib/rh_resources ${GTA_3_FOLDER}/rh_resources)

add_custom_command(TARGET gta3_render_hook
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/rw_render_algorithms_lib/shaders ${GTA_3_FOLDER}/shaders)
