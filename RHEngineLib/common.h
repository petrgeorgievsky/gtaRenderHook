#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR

#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "vulkan-1.lib" )
#pragma comment( lib, "d3dcompiler.lib" )
#pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "dinput8.lib" )

#pragma warning( push, 0 )
// std lib headers
#include <chrono>
#include <codecvt>
#include <cstring>
#include <fstream>
#include <functional>
#include <locale>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

// Windows and graphics API headers
#include <Windows.h>
#include <comdef.h>
#include <d3d11_3.h>
#include <d3d12.h>
#include <d3d9types.h>
#define VULKAN_HPP_TYPESAFE_CONVERSION
#include <vulkan/vulkan.hpp>
#define _XM_NO_INTRINSICS_
#include <DirectXMath.h>
#include <d3dcompiler.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include <game_sa/RenderWare.h>
#pragma warning( pop )
