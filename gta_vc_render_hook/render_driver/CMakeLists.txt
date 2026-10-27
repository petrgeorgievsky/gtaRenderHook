cmake_minimum_required(VERSION 3.12)

project(gta_vc_render_driver)

set(SOURCES
        main.cpp
        )

add_executable(${PROJECT_NAME} WIN32 ${SOURCES})
#set_property(TARGET ${PROJECT_NAME} PROPERTY
#        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 20
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${GTA_VC_FOLDER}/
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${GTA_VC_FOLDER}/
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${GTA_VC_FOLDER}/
        )
target_compile_definitions(${PROJECT_NAME} PRIVATE -DUSE_VULKAN_API)

target_link_libraries(${PROJECT_NAME} $ENV{VULKAN_SDK}/Lib/vulkan-1.lib rh_engine_lib rw_rh_engine_lib)
target_compile_options(${PROJECT_NAME} PRIVATE)

include("${CMAKE_SOURCE_DIR}/cmake/resource_install.cmake")

# This process compiles shaders at runtime, so it installs the shader tree and
# the validator next to itself instead of relying on the 32-bit hook build.
copy_resources_to_build_dir(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/shaders" "shaders")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        $ENV{VULKAN_SDK}\\Bin\\glslangValidator.exe $<TARGET_FILE_DIR:${PROJECT_NAME}>/)
