cmake_minimum_required(VERSION 3.12)
include(../../rw_rh_engine_lib/install_rw_library.cmake)
project(GTAModelLoadingTest)

set(SOURCES
        forward_pbr_pipeline.cpp
        ModelLoadingTest.cpp
        main.cpp
        )

add_executable(${PROJECT_NAME} WIN32 ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${DEPENDENCY_LIB_LIST} rh_engine_lib imgui::imgui)
install_rw_library()

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 20
        )

#set_property(TARGET ${PROJECT_NAME} PROPERTY
#        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
# Copy shaders to build destination

copy_resources_to_build_dir( ${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/shaders" "shaders")

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/PROFILE")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        $ENV{VULKAN_SDK}\\Bin\\glslangValidator.exe $<TARGET_FILE_DIR:${PROJECT_NAME}>/)
# Copy test models to build destination
copy_resources_to_build_dir( ${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/Tests/GTAModelLoadingTest/models" "models")