cmake_minimum_required(VERSION 3.12)
set (CMAKE_CXX_STANDARD 17)

project (GTAModelLoadingTest)

set(SOURCES
    forward_pbr_pipeline.cpp
    ModelLoadingTest.cpp
    main.cpp
    )

include_directories( ../../rw_rh_engine_lib ../../RHEngineLib
    $ENV{PLUGIN_SDK_DIR}/plugin_sa ${DEPENDENCY_INCLUDE_LIST} )

add_executable( GTAModelLoadingTest WIN32 ${SOURCES} )
target_link_libraries( GTAModelLoadingTest ${DEPENDENCY_LIB_LIST} RHEngineLib rw_rh_engine_lib)
set_property(TARGET GTAModelLoadingTest PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
# Copy shaders to build destination
add_custom_command(TARGET GTAModelLoadingTest PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/Tests/GTAModelLoadingTest/shaders $<TARGET_FILE_DIR:GTAModelLoadingTest>/shaders)

# Copy test models to build destination
add_custom_command( TARGET GTAModelLoadingTest POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/Tests/GTAModelLoadingTest/models $<TARGET_FILE_DIR:GTAModelLoadingTest>/models)
