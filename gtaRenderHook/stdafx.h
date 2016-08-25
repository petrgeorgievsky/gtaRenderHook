
#ifndef stdafx_h__
#define stdafx_h__

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define VK_USE_PLATFORM_WIN32_KHR
#define NOASM
// Windows Header Files:
#include <windows.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <thread>
#include <vector>
#include <set>
#include <algorithm>
#include <functional>
#include <limits>
// Render API Header Files:
#include <d3d9.h>
#include <d3d11_1.h>
//#define DebuggingShaders
#pragma comment( lib, "dxguid.lib")
#ifndef DebuggingShaders
#include <D3DX11.h>
#else
#include <d3dcompiler.h>
#endif // !1

//
#include <gl\GL.h>
#include "vulkan/vulkan.h"
#define VK_RETURN_ON_ERR(r) if (r!=VK_SUCCESS) return false;
static const char *vk_layers[] = { "VK_LAYER_LUNARG_standard_validation" };
static const char *vk_extensions[] = { "VK_KHR_surface", "VK_KHR_win32_surface", "VK_EXT_debug_report" };
#include "gta_sa_ptrs.h"
#include "MemoryFuncs.h"
template<UINT TNameLength>
inline void SetDebugObjectName(_In_ ID3D11DeviceChild* resource,
	_In_z_ const char(&name)[TNameLength])
{
#if defined(_DEBUG) || defined(PROFILE)
	resource->SetPrivateData(WKPDID_D3DDebugObjectName, TNameLength - 1, name);
#endif
}
#endif // stdafx_h__
