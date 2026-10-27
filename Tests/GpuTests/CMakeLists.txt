cmake_minimum_required(VERSION 3.12)
project(GpuTests)

include("${CMAKE_SOURCE_DIR}/cmake/resource_install.cmake")

file(GLOB GPU_TEST_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp")
file(GLOB GPU_HARNESS_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/Harness/*.cpp")

add_executable(rh_gpu_tests main.cpp ${GPU_TEST_SOURCES} ${GPU_HARNESS_SOURCES})

target_include_directories(rh_gpu_tests PRIVATE
        "${CMAKE_SOURCE_DIR}/Tests/ThirdParty"
        "${CMAKE_SOURCE_DIR}/Tests/Common"
        "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(rh_gpu_tests PRIVATE rh_engine_lib rw_rh_engine_lib)

target_compile_definitions(rh_gpu_tests PRIVATE USE_VULKAN_API VULKAN_HPP_NO_EXCEPTIONS)

set_target_properties(rh_gpu_tests PROPERTIES CXX_STANDARD 20)

# doctest 2.4.11 specializes std::tuple for its String interop; MSVC 14.51 warns.
target_compile_options(rh_gpu_tests PRIVATE /wd5285)

copy_resources_to_build_dir(rh_gpu_tests "${CMAKE_SOURCE_DIR}/shaders" "shaders")

# Test shaders are copied on every build so that new .comp files are picked up
# without re-running CMake (file(GLOB) at configure time would miss them).
add_custom_target(rh_gpu_test_shaders
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
        "${CMAKE_CURRENT_BINARY_DIR}/shaders/tests"
        COMMENT "Copying test shaders")
add_dependencies(rh_gpu_tests rh_gpu_test_shaders)

add_custom_command(TARGET rh_gpu_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        $ENV{VULKAN_SDK}\\Bin\\glslangValidator.exe $<TARGET_FILE_DIR:rh_gpu_tests>/)

add_test(NAME gpu COMMAND rh_gpu_tests -tce=known_bug/*)
set_tests_properties(gpu PROPERTIES SKIP_RETURN_CODE 77)
