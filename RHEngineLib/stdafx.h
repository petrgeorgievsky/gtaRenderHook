#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR

#pragma warning(push, 0)
#include <windows.h>
#define NOASM
#include <game_sa/RenderWare.h>
#include <string>
#include <vector>
#include <ostream>
#include <fstream>
#include <sstream>
#include <locale>
#include <codecvt>
#include <functional>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <d3d11_3.h>
#include <d3d12.h>
#include <vulkan\vulkan.h>
#pragma warning(pop)