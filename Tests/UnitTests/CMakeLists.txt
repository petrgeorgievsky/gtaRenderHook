cmake_minimum_required(VERSION 3.12)
project(UnitTests)

include("${CMAKE_SOURCE_DIR}/cmake/resource_install.cmake")

file(GLOB UNIT_TEST_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp")

add_executable(rh_unit_tests main.cpp ${UNIT_TEST_SOURCES})

target_include_directories(rh_unit_tests PRIVATE
        "${CMAKE_SOURCE_DIR}/Tests/ThirdParty"
        "${CMAKE_SOURCE_DIR}/Tests/Common"
        "${CMAKE_SOURCE_DIR}/rw_rh_engine_lib")

target_link_libraries(rh_unit_tests PRIVATE rh_engine_lib)

target_compile_definitions(rh_unit_tests PRIVATE USE_VULKAN_API VULKAN_HPP_NO_EXCEPTIONS)

set_target_properties(rh_unit_tests PROPERTIES CXX_STANDARD 20)

# doctest 2.4.11 specializes std::tuple for its String interop; MSVC 14.51 warns.
target_compile_options(rh_unit_tests PRIVATE /wd5285)

copy_resources_to_build_dir(rh_unit_tests "${CMAKE_SOURCE_DIR}/shaders" "shaders")

# GPU test kernels are compiled by rh_unit_tests too, so a broken kernel is a
# readable CTest failure instead of a std::terminate inside rh_gpu_tests.
add_custom_target(rh_unit_test_shaders
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Tests/GpuTests/shaders"
        "${CMAKE_CURRENT_BINARY_DIR}/shaders/tests"
        COMMENT "Copying GPU test kernels")
add_dependencies(rh_unit_tests rh_unit_test_shaders)

add_custom_command(TARGET rh_unit_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        $ENV{VULKAN_SDK}\\Bin\\glslangValidator.exe $<TARGET_FILE_DIR:rh_unit_tests>/)

add_test(NAME unit COMMAND rh_unit_tests -tce=known_bug/*,shader/all_*)
