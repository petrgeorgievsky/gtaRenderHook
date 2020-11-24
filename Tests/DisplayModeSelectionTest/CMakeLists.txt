cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)

project(DisplayModeSelectionTest)

set(SOURCES
        main.cpp
        Im2DRenderer.cpp)

include_directories(. ../../rh_engine_lib ${DEPENDENCY_INCLUDE_LIST})

add_executable(${PROJECT_NAME} WIN32 ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${DEPENDENCY_LIB_LIST} rh_engine_lib)
#set_property(TARGET ${PROJECT_NAME} PROPERTY
#             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_custom_command(TARGET ${PROJECT_NAME}
        COMMAND ${CMAKE_COMMAND} -E copy
        $ENV{VULKAN_SDK}\\Bin32\\glslangValidator.exe $<TARGET_FILE_DIR:${PROJECT_NAME}>/)

if (USE_VULKAN_API)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DUSE_VULKAN_API)
endif ()


message("ShaderPath: ${CMAKE_SOURCE_DIR}\\shaders")
# Copy shaders to build destination
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}\\shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders)