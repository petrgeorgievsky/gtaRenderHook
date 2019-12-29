cmake_minimum_required(VERSION 3.12)
set (CMAKE_CXX_STANDARD 20)

project (DisplayModeSelectionTest)

set(SOURCES
main.cpp
    )

include_directories( ../../RHEngineLib
    $ENV{PLUGIN_SDK_DIR}/plugin_sa ${DEPENDENCY_INCLUDE_LIST} )

add_executable( DisplayModeSelectionTest WIN32 ${SOURCES})
target_link_libraries( DisplayModeSelectionTest ${DEPENDENCY_LIB_LIST} RHEngineLib )
set_property(TARGET DisplayModeSelectionTest PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_custom_command(TARGET DisplayModeSelectionTest
    COMMAND ${CMAKE_COMMAND} -E copy
    $ENV{VULKAN_SDK}\\Bin32\\glslangValidator.exe $<TARGET_FILE_DIR:DisplayModeSelectionTest>/)