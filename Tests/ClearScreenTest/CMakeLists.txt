cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)

project(ClearScreenTest)

set(SOURCES
        ClearScreenSample.cpp
        main.cpp
        )

include_directories(../../rw_rh_engine_lib ../../rh_engine_lib ${DEPENDENCY_INCLUDE_LIST})
include(../../rw_rh_engine_lib/install_rw_library.cmake)

add_executable(ClearScreenTest WIN32 ${SOURCES})
target_link_libraries(ClearScreenTest ${DEPENDENCY_LIB_LIST} rh_engine_lib)
install_rw_library()
#set_property(TARGET ClearScreenTest PROPERTY
#             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_custom_command(TARGET ${PROJECT_NAME}
        COMMAND ${CMAKE_COMMAND} -E copy
        $ENV{VULKAN_SDK}\\Bin32\\glslangValidator.exe $<TARGET_FILE_DIR:${PROJECT_NAME}>/)
# Copy shaders to build destination
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}\\shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders)