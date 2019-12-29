cmake_minimum_required(VERSION 3.15)
project (RenderHook)

# Compile with this option OFF if you want full compatibility
# with older videocards
option (USE_VULKAN_API "Add RenderHook Vulkan API implementation" ON)
option (BUILD_TESTS "Compile tests" ON)
option (ARCH_64BIT "Compile tests with x64 architecture" OFF)
option (BUILD_32BIT_LIBS "Compile libs with x86 architecture" OFF)

add_definitions(-DNOASM)

# We store common includes dirs in DEPENDENCY_INCLUDE_LIST macro,
# so we don't need to write it for every new subproject
set(DEPENDENCY_INCLUDE_LIST
    $ENV{PLUGIN_SDK_DIR}/plugin_sa
    )

if(USE_VULKAN_API)
    set(DEPENDENCY_INCLUDE_LIST ${DEPENDENCY_INCLUDE_LIST} $ENV{VULKAN_SDK}/include)
    if(ARCH_64BIT)
        set(DEPENDENCY_LIB_LIST $ENV{VULKAN_SDK}/Lib/vulkan-1.lib)
    else()
        set(DEPENDENCY_LIB_LIST $ENV{VULKAN_SDK}/Lib32/vulkan-1.lib)
    endif()
endif()

# RenderHook engine library - contains rendering backend implementation and interfaces
add_subdirectory(RHEngineLib)

# RenderWare to RenderHook engine wrapper library, contains RW functions, platform-specific
# functions implemented using RenderHook engine and pointer tables used to replace RW engine backends.
add_subdirectory(rw_rh_engine_lib)

add_subdirectory(rw_ray_tracing_lib)

add_subdirectory(rw_render_algorithms_lib)

if(BUILD_TESTS)
    add_subdirectory(Tests)
endif()
if(BUILD_32BIT_LIBS)
    add_subdirectory(gta3_render_hook)
    add_subdirectory(gtasa_render_hook)
    #add_subdirectory(gtasa_render_hook_rt)
endif()
