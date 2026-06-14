cmake_minimum_required(VERSION 3.12)

project(gta_sa_render_hook)
set(GTA_SA_FOLDER CACHE PATH "GTA SA folder")

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
        gta_sa_internal_classes/CColorSet.cpp
        game_patches/material_system_patches.cpp
        game/TxdStore.cpp
        )

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".asi")
set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 20
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${GTA_SA_FOLDER}/
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${GTA_SA_FOLDER}/
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${GTA_SA_FOLDER}/
        )

#set_property(TARGET ${PROJECT_NAME} PROPERTY
#        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_link_libraries(${PROJECT_NAME} rh_engine_lib)


install_rw_library()

# Copy shaders to build destination
copy_resources_to_build_dir(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}\\shaders" "shaders" )