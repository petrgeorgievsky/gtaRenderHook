#pragma once
struct RwDisplayMode:D3DDISPLAYMODE
{
	int bFullscreen;
};
struct RwVideoMode {
	int m_iWidth;
	int m_iHeight;
	int m_iDepth;
	int m_iAdapter;
	short m_sRefreshRate;
	short m_sRatioIndex;
	unsigned int m_uiFormat;
};
#define RwD3DSystem(a1, a2, a3, a4)((bool (__cdecl *)(int, void*, void*,int))0x7F5F70)(a1, a2, a3, a4)
#define RwHWnd (*(HWND *)0xC97C1C)
#define pD3D (*(IDirect3D9 **)0xC97C20)
#define RwD3DAdapterIndex (*(UINT *)0xC97C24)
#define RwD3DDevType (*(D3DDEVTYPE *)0x8E2428)
#define RwD3DAdapterModeCount (*(int *)0xC9BEE0)
#define RwD3DDisplayMode (*(D3DDISPLAYMODE *)0xC9BEE4)
#define aRwD3DDisplayMode (*(RwDisplayMode **)0xC97C48)
#define RwD3DDisplayModeCount (*(int *)0xC97C40)
#define RwD3DMaxMultisamplingLevels (*(int *)0x8E242C)
#define RwD3DSelectedMultisamplingLevels (*(int *)0x8E2430)
#define RwD3DMaxMultisamplingLevelsNonMask (*(int *)0x8E2434)
#define RwD3DSelectedMultisamplingLevelsNonMask (*(int *)0x8E2438)
#define RwD3DbFullScreen (*(int *)0xC9BEF8)
#define RwD3D9CurrentModeIndex (*(int *)0xC97C18)
#define RwD3DDepth (*(int *)0xC9BEF4)
#define RwD3DPresentParams (*(D3DPRESENT_PARAMETERS *)0xC9C040)
#define RwD3D9DeviceCaps (*(D3DCAPS9 *)0xC9BF00)
#define pD3DDevice (*(IDirect3DDevice9 **)0xC97C28)
#define RwD3D9DepthStencilSurface (*(IDirect3DSurface9 **)0xC97C2C)
#define RwD3D9RenderSurface (*(IDirect3DSurface9 **)0xC97C30)
#define CurrentDepthStencilSurface (*(int *)0xC9808C)
#define CurrentRenderSurface ((IDirect3DSurface9**)0xC98090)
#define rwD3D9InitLastUsedObjects()((void (__cdecl *)())0x7F6B00)()
#define rwD3D9InitMatrixList()((void (__cdecl *)())0x7F6B40)()
#define MaxNumLights (*(int *)0xC98084)
#define LightsCache (*(void **)0xC98088)
#define _rwD3D9RasterOpen()((void (__cdecl *)())0x4CC150)()
#define _rwD3D9Im2DRenderOpen()((void (__cdecl *)())0x7FB480)()
#define _rwD3D9RenderStateOpen()((void (__cdecl *)())0x7FCAC0)()
#define _rwD3D9VertexBufferManagerOpen()((void (__cdecl *)())0x7F5CB0)()
#define SystemStarted (*(int *)0xC97C38)
#define dword_C97C3C (*(int *)0xC97C3C)
#define dword_C9BEFC (*(int *)0xC9BEFC)
#define EnableMultithreadSafe (*(int *)0xC97C4C)
#define EnableSoftwareVertexProcessing (*(int *)0xC97C50)
#define D3D9SetPresentParameters(dispMode,bFullscreen,fmt)((void (__cdecl *)(D3DDISPLAYMODE*,bool,D3DFORMAT))0x7F6CB0)(dispMode,bFullscreen,fmt)
#define D3D9CalculateMaxMultisamplingLevels() ((void(__cdecl *)())0x7F6BF0)()
