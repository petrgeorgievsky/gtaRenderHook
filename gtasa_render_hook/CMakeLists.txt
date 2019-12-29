cmake_minimum_required(VERSION 3.12)
set (CMAKE_CXX_STANDARD 17)

project (gtasa_render_hook)
set(GTA_SA_FOLDER CACHE PATH "GTA SA folder")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE")

include_directories(
    ../rw_ray_tracing_lib
    ../rw_render_algorithms_lib
    ../rw_rh_engine_lib
    ../RHEngineLib
    ${DEPENDENCY_INCLUDE_LIST}
    )

set(SOURCES
    dllmain.cpp
    idle_hook.cpp
    renderloop.cpp
    gta_sa_internal_classes/game.cpp
    gta_sa_internal_classes/renderer.cpp
    gta_sa_internal_classes/timer.cpp
    gta_sa_internal_classes/scene.cpp
    gta_sa_internal_classes/sprite2d.cpp
    gta_sa_internal_classes/font.cpp
    gta_sa_internal_classes/audioengine.cpp
    gta_sa_internal_classes/hud.cpp
    gta_sa_internal_classes/menumanager.cpp
    gta_sa_internal_classes/rw_func_ptrs.cpp
    )

set ( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON )

add_library( gtasa_render_hook SHARED ${SOURCES})

set_target_properties( gtasa_render_hook PROPERTIES SUFFIX ".asi" )
set_target_properties( gtasa_render_hook PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY_DEBUG ${GTA_SA_FOLDER}/
                      RUNTIME_OUTPUT_DIRECTORY_RELEASE ${GTA_SA_FOLDER}/
)

set_property(TARGET gtasa_render_hook PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_link_libraries( gtasa_render_hook ${DEPENDENCY_LIB_LIST} RHEngineLib rw_rh_engine_lib rw_render_algorithms_lib rw_ray_tracing_lib )

add_custom_command(TARGET gtasa_render_hook
                 COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/rw_render_algorithms_lib/shaders $<TARGET_FILE_DIR:gtasa_render_hook>/shaders)

add_custom_command( TARGET gtasa_render_hook
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                     ${CMAKE_SOURCE_DIR}/rw_ray_tracing_lib/shaders $<TARGET_FILE_DIR:gtasa_render_hook>/shaders)

add_custom_command( TARGET gtasa_render_hook
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/rw_ray_tracing_lib/rh_resources $<TARGET_FILE_DIR:gtasa_render_hook>/rh_resources)
