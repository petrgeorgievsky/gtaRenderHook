cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)

project(TextureLoadingTest)

set(SOURCES
        SimpleSample.cpp
        main.cpp
        )

include_directories(../../rw_rh_engine_lib ../../rh_engine_lib ${DEPENDENCY_INCLUDE_LIST})

add_executable(TextureLoadingTest WIN32 ${SOURCES})
target_link_libraries(TextureLoadingTest ${DEPENDENCY_LIB_LIST} rh_engine_lib rw_rh_engine_lib)
#set_property(TARGET TextureLoadingTest PROPERTY
#        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_custom_command(TARGET ${PROJECT_NAME}
        COMMAND ${CMAKE_COMMAND} -E copy
        $ENV{VULKAN_SDK}\\Bin32\\glslangValidator.exe $<TARGET_FILE_DIR:${PROJECT_NAME}>/)
# Copy shaders to build destination
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}\\shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders)

# Copy shaders to build destination
# todo: create test textures
#add_custom_command(TARGET TextureLoadingTest POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_directory
#        ${CMAKE_SOURCE_DIR}/Tests/TextureLoadingTest/textures $<TARGET_FILE_DIR:TextureLoadingTest>/textures)

# Copy shaders to build destination
add_custom_command(TARGET TextureLoadingTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/rw_rh_engine_lib/resources $<TARGET_FILE_DIR:TextureLoadingTest>/resources)
