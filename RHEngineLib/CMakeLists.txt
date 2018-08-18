cmake_minimum_required(VERSION 3.12)
set (CMAKE_CXX_STANDARD 14)

project (RenderHookEngineLibrary)
# Compile with this option OFF if you want full compatibility
# with older videocards
option (USE_VULKAN_API "Add RenderHook Vulkan API implementation" ON)
option (USE_WINDOWS_SDK "Use windows sdk" OFF)

add_definitions( -DUNICODE -D_UNICODE )

# Include plugin sdk
#add_library($ENV{PLUGIN_SDK_DIR}/output/lib STATIC IMPORTED)
include_directories($ENV{PLUGIN_SDK_DIR}\\plugin_sa\\)

include_directories(/ TestUtils)

set(SOURCES TestUtils/TestSample.cpp)

add_library( RHEngineLib STATIC ${SOURCES})
