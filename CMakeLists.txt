cmake_minimum_required(VERSION 3.10.2)
cmake_policy(SET CMP0091 NEW)
project(RenderHook)

# Compile with this option OFF if you want full compatibility
# with older videocards
option(USE_VULKAN_API "Add RenderHook Vulkan API implementation" ON)
option(BUILD_TESTS "Compile tests" ON)
option(ARCH_64BIT "Compile tests with x64 architecture" OFF)
option(BUILD_32BIT_LIBS "Compile libs with x86 architecture" OFF)

set(CLANG_ANALYSER_PATH CACHE PATH "Clang analyser path")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Dependencies
find_package(imgui CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_definitions(-DNOASM)
if (ARCH_64BIT)
    add_definitions(-DARCH_64BIT)
endif ()
# We store common includes dirs in DEPENDENCY_INCLUDE_LIST macro,
# so we don't need to write it for every new subproject
set(DEPENDENCY_INCLUDE_LIST)

if (USE_VULKAN_API)
    set(DEPENDENCY_INCLUDE_LIST ${DEPENDENCY_INCLUDE_LIST} $ENV{VULKAN_SDK}/include)
    if (ARCH_64BIT)
        set(DEPENDENCY_LIB_LIST $ENV{VULKAN_SDK}/Lib/vulkan-1.lib)
    else ()
        set(DEPENDENCY_LIB_LIST $ENV{VULKAN_SDK}/Lib32/vulkan-1.lib)
    endif ()
endif ()

# RenderHook engine library - contains rendering backend implementation and interfaces
add_subdirectory(rh_engine_lib)

# RenderWare to RenderHook engine wrapper library, contains RW functions, platform-specific
# functions implemented using RenderHook engine and pointer tables used to replace RW engine backends.
add_subdirectory(rw_rh_engine_lib)

#add_subdirectory(rw_ray_tracing_lib)
#add_subdirectory(rw_render_algorithms_lib)

if (BUILD_TESTS)
    add_subdirectory(Tests)
endif ()
if (BUILD_32BIT_LIBS)
    if (ARCH_64BIT)
        add_subdirectory(gta_3_render_hook/render_driver)
        add_subdirectory(gta_vc_render_hook/render_driver)
        add_subdirectory(gta_sa_render_hook/render_driver)
    else ()
        add_subdirectory(gta_3_render_hook)
        add_subdirectory(gta_vc_render_hook)
        add_subdirectory(gta_sa_render_hook)
    endif ()
    #add_subdirectory(gta_sa_render_hook)
    #add_subdirectory(gtasa_render_hook_rt)
endif ()
