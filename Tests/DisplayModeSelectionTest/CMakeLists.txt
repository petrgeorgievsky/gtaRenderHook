cmake_minimum_required(VERSION 3.12)

project(DisplayModeSelectionTest)

include(resource_install)

set(SOURCES
        main.cpp
        Im2DRenderer.cpp)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${DEPENDENCY_LIB_LIST} rh_engine_lib)
#set_property(TARGET ${PROJECT_NAME} PROPERTY
#             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 20
        )

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        $ENV{VULKAN_SDK}\\Bin\\glslangValidator.exe $<TARGET_FILE_DIR:${PROJECT_NAME}>/)

if (USE_VULKAN_API)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DUSE_VULKAN_API)
endif ()

# Copy shaders to build destination
copy_resources_to_build_dir(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}\\shaders" "shaders" )