cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)

project(gta_sa_render_driver)

include_directories(
        /
        ../../rh_engine_lib/
        ../../rw_rh_engine_lib/
        ${DEPENDENCY_INCLUDE_LIST}
)
set(SOURCES
        main.cpp
        )

add_executable(${PROJECT_NAME} WIN32 ${SOURCES})
#set_property(TARGET ${PROJECT_NAME} PROPERTY
#        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${GTA_SA_FOLDER}/
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${GTA_SA_FOLDER}/
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${GTA_SA_FOLDER}/
        )
target_compile_definitions(${PROJECT_NAME} PRIVATE -DUSE_VULKAN_API)

target_link_libraries(${PROJECT_NAME} $ENV{VULKAN_SDK}/Lib/vulkan-1.lib rh_engine_lib rw_rh_engine_lib)
target_compile_options(${PROJECT_NAME} PRIVATE)

add_custom_command(TARGET ${PROJECT_NAME}
        COMMAND ${CMAKE_COMMAND} -E copy
        $ENV{VULKAN_SDK}\\Bin\\glslangValidator.exe $<TARGET_FILE_DIR:${PROJECT_NAME}>/)