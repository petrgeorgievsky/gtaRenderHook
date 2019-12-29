cmake_minimum_required(VERSION 3.12)
set (CMAKE_CXX_STANDARD 17)

project (TextureLoadingTest)

set(SOURCES
    SimpleSample.cpp
    main.cpp
    )

include_directories( ../../rw_rh_engine_lib ../../RHEngineLib
    $ENV{PLUGIN_SDK_DIR}/plugin_sa ${DEPENDENCY_INCLUDE_LIST} )

add_executable( TextureLoadingTest WIN32 ${SOURCES} )
target_link_libraries( TextureLoadingTest ${DEPENDENCY_LIB_LIST} RHEngineLib rw_rh_engine_lib)
set_property(TARGET TextureLoadingTest PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
# Copy shaders to build destination
add_custom_command(TARGET TextureLoadingTest POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:TextureLoadingTest>/shaders)

# Copy shaders to build destination
add_custom_command( TARGET TextureLoadingTest POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/Tests/TextureLoadingTest/textures $<TARGET_FILE_DIR:TextureLoadingTest>/textures)
