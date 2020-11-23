cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 17)

project(rw_x64_render_driver)

include_directories(
        /
        ../rh_engine_lib/
        ${DEPENDENCY_INCLUDE_LIST}
)
set(SOURCES
        main.cpp
        )

add_executable(rw_x64_render_driver ${SOURCES})
#set_property(TARGET rw_x64_render_driver PROPERTY
#             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_compile_definitions(${PROJECT_NAME} PRIVATE -DUSE_VULKAN_API)

target_link_libraries(rw_x64_render_driver $ENV{VULKAN_SDK}/Lib32/vulkan-1.lib rh_engine_lib)
target_compile_options(rw_x64_render_driver PRIVATE)
