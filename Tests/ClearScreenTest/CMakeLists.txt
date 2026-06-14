cmake_minimum_required(VERSION 3.12)

project(ClearScreenTest)

set(SOURCES
        ClearScreenSample.cpp
        main.cpp
        )

include(../../rw_rh_engine_lib/install_rw_library.cmake)

add_executable(ClearScreenTest WIN32 ${SOURCES})
target_link_libraries(ClearScreenTest rh_engine_lib)
install_rw_library()

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 20
        )

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        $ENV{VULKAN_SDK}\\Bin\\glslangValidator.exe $<TARGET_FILE_DIR:${PROJECT_NAME}>/)

# Copy shaders to build destination
copy_resources_to_build_dir( ${PROJECT_NAME} "${CMAKE_SOURCE_DIR}\\shaders" "shaders" )