cmake_minimum_required(VERSION 3.12)
set (CMAKE_CXX_STANDARD 11)

project (RenderHookEngineTests)

add_definitions( -DUNICODE -D_UNICODE )

# Include plugin sdk
#add_library($ENV{PLUGIN_SDK_DIR}/output/lib STATIC IMPORTED)
include_directories($ENV{PLUGIN_SDK_DIR}\\plugin_sa\\)

include_directories(../RHEngineLib/)

add_executable( RHEngineTests RHEngineTests.cpp )
