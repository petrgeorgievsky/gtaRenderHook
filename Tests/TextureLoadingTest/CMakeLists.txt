cmake_minimum_required(VERSION 3.12)

project(TextureLoadingTest)

set(SOURCES
        SimpleSample.cpp
        main.cpp
        )

add_executable(TextureLoadingTest WIN32 ${SOURCES})
target_link_libraries(TextureLoadingTest ${DEPENDENCY_LIB_LIST} rh_engine_lib rw_rh_engine_lib)

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 20
        )

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        $ENV{VULKAN_SDK}\\Bin\\glslangValidator.exe $<TARGET_FILE_DIR:${PROJECT_NAME}>/)

install_rw_library()

# Copy shaders to build destination
copy_resources_to_build_dir(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}\\shaders" "shaders" )
