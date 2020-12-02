set(CMAKE_CXX_STANDARD 20)

project(gta_3_render_hook)
option(USE_VULKAN_API "Add RenderHook Vulkan API implementation" ON)
set(GTA_3_FOLDER CACHE PATH "GTA 3 folder")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DLL")

include_directories(
        ../rh_engine_lib
        ../rw_rh_engine_lib
        #[[
         ../rw_ray_tracing_lib
         ../rw_render_algorithms_lib]]
        ${DEPENDENCY_INCLUDE_LIST}
)

set(SOURCES
        dllmain.cpp
        gta3_geometry_proxy.cpp
        game/Renderer.cpp
        game/World.cpp
        game/Entity.cpp
        game/Streaming.cpp
        game/ModelInfo.cpp
        game/PointLights.cpp
        game/Shadows.cpp
        game/Vector.cpp
        game_patches/car_path_bug_fix.cpp
        game_patches/base_model_pipeline.cpp config/GameRendererConfigBlock.cpp config/GameRendererConfigBlock.h)

#set ( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON )

add_library(${PROJECT_NAME} SHARED ${SOURCES})
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xclang -fmodules-ts")

set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".asi")
set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${GTA_3_FOLDER}/
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${GTA_3_FOLDER}/
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${GTA_3_FOLDER}/
        )
#set_target_properties(${PROJECT_NAME} PROPERTIES
#        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_custom_command(TARGET ${PROJECT_NAME}
        COMMAND ${CMAKE_COMMAND} -E copy
        $ENV{VULKAN_SDK}\\Bin32\\glslangValidator.exe $<TARGET_FILE_DIR:${PROJECT_NAME}>/)


if (USE_VULKAN_API)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DUSE_VULKAN_API)
endif ()

message("ShaderPath: ${CMAKE_SOURCE_DIR}\\shaders")
message("ShaderDestPath: $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders")
# Copy shaders to build destination
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}\\shaders ${GTA_3_FOLDER}/shaders)

#target_compile_options(${PROJECT_NAME} PRIVATE -fno-omit-frame-pointer -fsanitize=address)
#target_link_options(${PROJECT_NAME} PRIVATE -fno-omit-frame-pointer -fsanitize=address)

target_link_libraries(${PROJECT_NAME} ${DEPENDENCY_LIB_LIST}
        rh_engine_lib rw_rh_engine_lib)

# Copy shaders to build destination
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/rw_rh_engine_lib/resources ${GTA_3_FOLDER}/resources)