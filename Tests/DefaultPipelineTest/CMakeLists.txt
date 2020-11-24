cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)

project(DefaultPipelineTest)

set(SOURCES
        SimpleSample.cpp
        forward_pbr_pipeline.cpp

        clump_read_funcs.cpp

        rp_atomic_funcs.cpp
        rp_clump_funcs.cpp

        rp_geometry_binmesh_read_funcs.cpp

        rp_geometry_funcs.cpp
        rp_geometry_read_funcs.cpp

        rp_material_funcs.cpp
        rp_material_read_funcs.cpp

        rp_matlist_funcs.cpp
        rp_matlist_read_funcs.cpp

        rw_frame_funcs.cpp
        rw_framelist_read_funcs.cpp

        rw_geometrylist_read_funcs.cpp

        main.cpp
        )

include_directories(../../RHEngineLib ${DEPENDENCY_INCLUDE_LIST})

add_executable(DefaultPipelineTest WIN32 ${SOURCES})
target_link_libraries(DefaultPipelineTest ${DEPENDENCY_LIB_LIST} RHEngineLib)

# Copy shaders to build destination
add_custom_command(TARGET DefaultPipelineTest PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:DefaultPipelineTest>/shaders)

add_custom_command(TARGET DefaultPipelineTest
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:DefaultPipelineTest>
        "C:/Users/peter/Documents/Visual Studio 2015/Projects/gtaRenderHook/Win32Build/Release")
#
