cmake_minimum_required(VERSION 3.12)
set (CMAKE_CXX_STANDARD 17)

project (GTAModelRayTracingTest)

set(SOURCES
    gta_map_loader.cpp
    forward_pbr_pipeline.cpp
    ModelLoadingTest.cpp
    main.cpp
    )
#SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE")

include_directories(
    ../../rw_ray_tracing_lib
    ../../rw_render_algorithms_lib
    ../../rw_rh_engine_lib
    ../../RHEngineLib
    $ENV{PLUGIN_SDK_DIR}/plugin_sa
    ${DEPENDENCY_INCLUDE_LIST} )

add_executable( GTAModelRayTracingTest WIN32 ${SOURCES} )
target_link_libraries( GTAModelRayTracingTest ${DEPENDENCY_LIB_LIST}
    RHEngineLib rw_rh_engine_lib rw_ray_tracing_lib
    rw_render_algorithms_lib)
set_property(TARGET GTAModelRayTracingTest PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Copy shaders to build destination
add_custom_command(TARGET GTAModelRayTracingTest
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/Tests/GTAModelRayTracingTest/shaders $<TARGET_FILE_DIR:GTAModelRayTracingTest>/shaders)

add_custom_command( TARGET GTAModelRayTracingTest
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/rw_ray_tracing_lib/shaders $<TARGET_FILE_DIR:GTAModelRayTracingTest>/shaders)
add_custom_command( TARGET GTAModelRayTracingTest
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/rw_ray_tracing_lib/rh_resources $<TARGET_FILE_DIR:GTAModelRayTracingTest>/rh_resources)

add_custom_command(TARGET GTAModelRayTracingTest
                 COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/rw_render_algorithms_lib/shaders $<TARGET_FILE_DIR:GTAModelRayTracingTest>/shaders)

# Copy test models to build destination
add_custom_command( TARGET GTAModelRayTracingTest POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/Tests/GTAModelRayTracingTest/models $<TARGET_FILE_DIR:GTAModelRayTracingTest>/models)
