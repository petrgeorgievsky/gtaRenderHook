cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)

project(gta_sa_render_hook)
set(GTA_SA_FOLDER CACHE PATH "GTA SA folder")

include_directories(
        ../rw_rh_engine_lib
        ../rh_engine_lib
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
        gta_sa_internal_classes/CColorSet.cpp gta_sa_internal_classes/CColorSet.h)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".asi")
set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${GTA_SA_FOLDER}/
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${GTA_SA_FOLDER}/
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${GTA_SA_FOLDER}/
        )

#set_property(TARGET ${PROJECT_NAME} PROPERTY
#        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_link_libraries(${PROJECT_NAME} ${DEPENDENCY_LIB_LIST} rh_engine_lib rw_rh_engine_lib)


add_custom_command(TARGET ${PROJECT_NAME}
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/rw_render_algorithms_lib/shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders)

add_custom_command(TARGET ${PROJECT_NAME}
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/rw_ray_tracing_lib/shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders)

add_custom_command(TARGET ${PROJECT_NAME}
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/rw_ray_tracing_lib/rh_resources $<TARGET_FILE_DIR:${PROJECT_NAME}>/rh_resources)

add_custom_command(TARGET ${PROJECT_NAME}
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders)
# Copy shaders to build destination
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/rw_rh_engine_lib/resources ${GTA_SA_FOLDER}/resources)