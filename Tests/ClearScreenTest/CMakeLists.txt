cmake_minimum_required(VERSION 3.12)
set (CMAKE_CXX_STANDARD 17)

project (ClearScreenTest)

set(SOURCES
    ClearScreenSample.cpp
    main.cpp
    )

include_directories( ../../rw_rh_engine_lib ../../RHEngineLib
    $ENV{PLUGIN_SDK_DIR}/plugin_sa ${DEPENDENCY_INCLUDE_LIST} )

add_executable(ClearScreenTest WIN32 ${SOURCES})
target_link_libraries( ClearScreenTest ${DEPENDENCY_LIB_LIST} RHEngineLib rw_rh_engine_lib)
set_property(TARGET ClearScreenTest PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
# Copy shaders to build destination
add_custom_command(TARGET ClearScreenTest PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:ClearScreenTest>/shaders)
