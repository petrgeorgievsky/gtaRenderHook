project(gta_vc_render_hook)
option(USE_VULKAN_API "Add RenderHook Vulkan API implementation" ON)
set(GTA_VC_FOLDER CACHE PATH "GTA VC folder")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DLL")

set(SOURCES
        dllmain.cpp
        game/TxdStore.cpp
        game/World.cpp
        game/Renderer.cpp
        game/Entity.cpp
        game/Streaming.cpp
        game/ModelInfo.cpp
        game/Clock.cpp
        game/Vector.cpp
        game/Game.cpp
        game/Shadows.cpp
        game/PointLights.cpp
        game_patches/material_system_patches.cpp
        game_patches/rwd3d8_patches.cpp
        game_patches/base_model_pipeline.cpp
        game_patches/skin_model_pipeline.cpp)

#set ( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON )

add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 20
        SUFFIX ".asi"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${GTA_VC_FOLDER}/
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${GTA_VC_FOLDER}/
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${GTA_VC_FOLDER}/
        )
#set_target_properties(${PROJECT_NAME} PROPERTIES
#        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        $ENV{VULKAN_SDK}\\Bin\\glslangValidator.exe $<TARGET_FILE_DIR:${PROJECT_NAME}>/)

target_link_libraries(${PROJECT_NAME} ${DEPENDENCY_LIB_LIST}
        rh_engine_lib)

install_rw_library()

# Copy shaders to build destination
copy_resources_to_build_dir(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}\\shaders" "shaders" )