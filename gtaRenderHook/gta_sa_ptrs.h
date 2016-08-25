#ifndef GTA_SA_PTRS_H
#define GTA_SA_PTRS_H
#include <rwcore.h>
#include <rpworld.h>
#include <rpskin.h>
//#include "stdafx.h"
struct RwDisplayMode :D3DDISPLAYMODE
{
	RwVideoModeFlag flags;
};
struct RwD3D9VertexDecl
{
	IDirect3DVertexDeclaration9* decl;
	int elements_count;
	D3DVERTEXELEMENT9* elements;
};
struct RwD3D9Raster
{
	union
	{
		IDirect3DTexture9 *texture;
		IDirect3DSurface9 *surface;
	};
	unsigned char       *palette;
	unsigned char        alpha;
	unsigned char        cubeTextureFlags; /* 0x01 IS_CUBEMAP_TEX */
	unsigned char        textureFlags;     /* 0x01 HAS_MIP_MAPS
										   0x10 IS_COMPRESSED */
	unsigned char        lockFlags;
	IDirect3DSurface9   *lockedSurface;
	D3DLOCKED_RECT       lockedRect;
	D3DFORMAT            format;
	IDirect3DSwapChain9 *swapChain;
	HWND                *hwnd;
};
class CD3D1XTexture;
struct RwD3D1XRaster
{
	CD3D1XTexture		*resourse;
	unsigned char       *palette;
	unsigned char        alpha;
	unsigned char        cubeTextureFlags; /* 0x01 IS_CUBEMAP_TEX */
	unsigned char        textureFlags;     /* 0x01 HAS_MIP_MAPS
										   0x10 IS_COMPRESSED */
	unsigned char        lockFlags;
	ID3D11Resource		*lockedSurface;
	D3DLOCKED_RECT       lockedRect;
	DXGI_FORMAT          format;
	IDXGISwapChain		*swapChain;
	HWND                *hwnd;
};
struct RwVulkanRasterData
{
	VkImage		vkImg;
	VkImageView vkImgView;
	VkDeviceMemory memory;
	VkSampler	vkSampler;
};

struct RwVulkanRaster
{
	RwVulkanRasterData		*texture;
	unsigned char       *palette;
	unsigned char        alpha;
	unsigned char        cubeTextureFlags; /* 0x01 IS_CUBEMAP_TEX */
	unsigned char        textureFlags;     /* 0x01 HAS_MIP_MAPS
										   0x10 IS_COMPRESSED */
	unsigned char        lockFlags;
	VkImageView   *lockedSurface;
	D3DLOCKED_RECT       lockedRect;
	VkFormat            format;
	VkSwapchainKHR		*swapChain;
	HWND                *hwnd;
};
struct _rwD3DRaster {
	RwRaster r;
	RwD3D9Raster dr;
};
struct _rwD3D1XRaster {
	RwRaster r;
	RwD3D1XRaster dr;
};
struct _rwVKRaster {
	RwRaster r;
	RwVulkanRaster dr;
};
struct RwVideoMemoryRaster
{
	RwRaster* pCurrent;
	RwVideoMemoryRaster* pParent;
};
struct RwD3D9PixelFormat {
	byte alpha, depth, rasterFormat, unk;
};
struct TextureFormat
{
	unsigned int platformId;
	unsigned int filterMode : 8;
	unsigned int uAddressing : 4;
	unsigned int vAddressing : 4;
	char name[32];
	char maskName[32];
};

struct RasterFormat
{
	unsigned int rasterFormat;
	D3DFORMAT d3dFormat;
	unsigned short width;
	unsigned short height;
	unsigned char depth;
	unsigned char numLevels;
	unsigned char rasterType;
	unsigned char alpha : 1;
	unsigned char cubeTexture : 1;
	unsigned char autoMipMaps : 1;
	unsigned char compressed : 1;
};
struct RwD3D9DynamicVertexBuffer
{
	int length;
	int unk;
	IDirect3DVertexBuffer9* pVB;
	IDirect3DVertexBuffer9** ppVB;
	RwD3D9DynamicVertexBuffer* pParent;
};
 struct CreatedVB 
 { 
	 IDirect3DVertexBuffer9 *pVB;
	 CreatedVB* pParent;
 };
struct FreeVB {
	int offset;
	int size;
	IDirect3DVertexBuffer9 *pVB;
	FreeVB *pParent;
	FreeVB *dword10;
};
 struct RwD3D9Stride {
	 int stride;
	 FreeVB *pFreeVB;
	 CreatedVB* pCreatedVB;
	 RwD3D9Stride *pParent;
 };
 struct rxInstanceData:RwResEntry
 {
	 RxD3D9ResEntryHeader header;
	 RxD3D9InstanceData models[];
 };
 struct rpD3DMeshHeader:RpMeshHeader
 {
	 RpMesh meshes[];
 };
 struct pipelineCBs
 {
	 RxD3D9AllInOneInstanceCallBack instance;
	 RxD3D9AllInOneReinstanceCallBack reinstance;
	 RxD3D9AllInOneLightingCallBack lighting;
	 RxD3D9AllInOneRenderCallBack render;
 };
 struct RpSkinPipeCB
 {
	 RxD3D9AllInOneRenderCallBack	render;
	 RxD3D9AllInOneLightingCallBack lighting;
	 _rxD3D9VertexShaderBeginCallBack VSBegin;
	 _rxD3D9VertexShaderLightingCallBack VSLighting;
	 _rxD3D9VertexShaderGetMaterialShaderCallBack VSGetMatShader;
	 _rxD3D9VertexShaderMeshRenderCallBack VSRender;
	 _rxD3D9VertexShaderEndCallBack VSEnd;
 };
 struct RsGlobalType
 {
	 char *AppName;
	 int MaximumWidth;
	 int MaximumHeight;
	 int frameLimit;
	 int quit;
	 int ps;
 };
 struct RpSkin
 {
	 RwUInt32			numBones;
	 RwUInt32			numBoneIds;
	 RwUInt8			*boneIds;
	 RwMatrix			*skinToBoneMatrices;
	 RwUInt32			maxNumWeightsForVertex;
	 RwUInt32			*vertexBoneIndices;
	 RwMatrixWeights	*vertexBoneWeights;
	 RwUInt32			boneCount;
	 RwUInt32			useVS;
	 RwUInt32			boneLimit;
	 RwUInt32			numMeshes;
	 RwUInt32			numRLE;
	 RwUInt8			*meshBoneRemapIndices;
	 RwUInt32			meshBoneRLECount;
	 void				*meshBoneRLE;
	 void				*field_3C;
 };
 struct RwMatrix4x3 {
	 RwV4d a[3];
 };

 struct rpSkinGlobals
 {
	 int globalPluginOffset;
	 int _atomicSkinOffset;
	 int _geometrySkinOffset;
	 RwMatrix *alignedMatrixCache;
	 RpSkin *skinArray;
	 char anonymous_1[4];
	 RwFreeList *alignedMatrixCacheFreeList;
	 char anonymous_2[4];
	 int instanceCount;
	 RxPipeline *m_pPipelines[4];
	 int pCurrentSkin;
	 int anonymous_6;
	 int pCurrentFrame;
	 int supportSystemMemBuffers;
	 int useVertexShader;
	 int nMaxBoneCount;
	 int maxVSConst;
 };
 struct simpleVertex
 {
	 RwV3d pos, normal;
	 RwTexCoords uv;
	 RwRGBA color;
 };
 struct simpleVertexSkin
 {
	 RwV3d pos, normal;
	 RwTexCoords uv;
	 RwRGBA color;
	 RwMatrixWeights weights;
	 UINT	indices;
 };
 class CD3D1XVertexDeclaration;
 struct RxD3D1XResEntryHeader
 {
	 RwUInt32    serialNumber;   /**< The mesh headers serial number */

	 RwUInt32    numMeshes;      /**< The number of meshes */

	 void        *indexBuffer;   /**< Index buffer */

	 RwUInt32    primType;       /**< Primitive type */

	 RxD3D9VertexStream vertexStream[RWD3D9_MAX_VERTEX_STREAMS];   /**< Vertex streams */

	 RwBool      useOffsets;      /**< Use vertex buffer offsets when setting the streams */

	 CD3D1XVertexDeclaration *vertexDeclaration;   /**< Vertex declaration */

	 size_t    totalNumIndex;  /**< Total number of indices. Needed for
								 reinstancing, not for rendering */

	 size_t    totalNumVertex; /**< Total number of vertices. Needed for
								 reinstancing, not for rendering */
 };
#define RsGlobal ((RsGlobalType *)0xC17040)
#define RwEngineInstance ((void *)0xC97B24)
#define RpSkinGlobals (*(rpSkinGlobals *)0xC978A0)
#define rpResModule (*(RwModuleInfo *)0xC9BC58)

#define _RWPLUGINOFFSET(_type,_base,_offset) \
	*(_type**)((UINT)_base + (_offset))

#define _RWRESOURCESGLOBAL(var) (_RWPLUGINOFFSET(rwResourcesGlobals,  \
    RwEngineInstance, rpResModule.globalsOffset)->var)
#define _RwResourcesUseResEntry(_ntry)                               \
    ((((_ntry)->link.next)?                                         \
          (rwLinkListRemoveLLLink(&((_ntry)->link)),                \
           rwLinkListAddLLLink(_RWRESOURCESGLOBAL(res.usedEntries),  \
                               &((_ntry)->link))):                  \
          NULL),                                                    \
     (_ntry))
#define _RWSRCGLOBAL(variable) \
   (((RwGlobals *)RwEngineInstance)->variable)
#define _RwMalloc(_s, _h)                                    \
    ((_RWSRCGLOBAL(memoryFuncs).rwmalloc)((_s), (_h)))
#define _RwFree(_p)                                          \
    ((_RWSRCGLOBAL(memoryFuncs).rwfree)((_p)))
#define GeometryGetSkin(geom) \
	*(RpSkin**)((UINT)geom + RpSkinGlobals._geometrySkinOffset)

#define AtomicGetHAnimHier(atomic) \
	*(RpHAnimHierarchy**)((UINT)atomic + RpSkinGlobals._atomicSkinOffset)

#define rwD3D9ImmPool (*(rwIm3DPool **)0xC9AB30)
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
#define CurrentDepthStencilSurface (*(IDirect3DSurface9 **)0xC9808C)
#define CurrentRenderSurface ((IDirect3DSurface9**)0xC98090)
#define rwD3D9LastFVFUsed (*(DWORD *)0x8E2440)
#define rwD3D9LastVertexDeclarationUsed (*(int *)0x8E2444)
#define rwD3D9LastVertexShaderUsed (*(int *)0x8E2448)
#define rwD3D9LastPixelShaderUsed (*(int *)0x8E244C)
#define rwD3D9LastIndexBufferUsed (*(int *)0x8E2450)
#define LastVertexStreamUsed (*(RxD3D9VertexStream**)0xC97BD8)
#define rwD3D9InitLastUsedObjects()((void (__cdecl *)())0x7F6B00)()
#define _D3D9CreateDisplayModesList()((void (__cdecl *)())0x7F7540)()
#define rwD3D9InitMatrixList()((void (__cdecl *)())0x7F6B40)()
#define MaxNumLights (*(int *)0xC98084)
#define LightsCache (*(void **)0xC98088)
#define _rwD3D9RasterOpen()((void (__cdecl *)())0x4CC150)()
#define _rwD3D9Im2DRenderOpen()((void (__cdecl *)())0x7FB480)()
#define _rwD3D9RenderStateOpen()((void (__cdecl *)())0x7FCAC0)()
#define _rwD3D9VertexBufferManagerOpen()((void (__cdecl *)())0x7F5CB0)()
#define SystemStarted (*(int *)0xC97C38)
#define RwHasStencilBuffer (*(int *)0xC97C3C)
#define RwD3D9ZBufferDepth (*(int *)0xC9BEFC)
#define EnableMultithreadSafe (*(int *)0xC97C4C)
#define EnableSoftwareVertexProcessing (*(int *)0xC97C50)
#define LastWorldMatrixUsedIdentity (*(bool*)0xC97C68)
#define MatrixFreeList (*(RwFreeList**)0xC98080)
#define LastMatrixUsed ((RwMatrix**)0xC97C70)
#define u_C98070 (*(RwMatrix**)0xC98070)
#define IdentityMatrix (*(RwMatrix*)0x8847A0)
#define VertexDeclarations (*(RwD3D9VertexDecl **)0xC97C60)
#define NumVertexDeclarations (*(int*)0xC97C58)
#define MaxVertexDeclarations (*(int*)0xC97C5C)
#define rwD3D9CPUSupportsSSE (*(int*)0xC980A4)
#define RwD3D9RasterExtOffset (*(int*)0xB4E9E0)
#define dgGGlobals (*(RwCamera**)0xC9BCC0)
#define RwD3D9D3D9ViewTransform (*(RwMatrix*)0xC9BC80)
#define RwD3D9D3D9ProjTransform (*(RwMatrix*)0x8E2458)
#define RwD3D9ActiveViewProjTransform (*(RwMatrix**)0xC97C64)
#define InsideScene (*(int*)0xC97C54)
#define VideoMemoryRastersFreeList (*(RwFreeList**)0xB4EA00)
#define VideoMemoryRasters (*(RwVideoMemoryRaster**)0xB4E9FC)
#define rwD3D9PixelFormatInfo (*(RwD3D9PixelFormat**)0xB4E7E0)
#define UseOwnVertexBuffer (*(int*)0xC980E0)
#define CurrentVertexBuffer (*(IDirect3DVertexBuffer9 **)0xC980E8)
#define CurrentBaseIndex (*(int*)0xC980E4)
#define IB2DOffset (*(int*)0xC980EC)
#define IndexBuffer2D (*(IDirect3DIndexBuffer9 **)0xC980F0)
#define VertexDeclIm2D (*(IDirect3DVertexDeclaration9 **)0xC980F4)
#define StrideFreeList (*(RwFreeList**)0xC97B80)
#define FreeVBFreeList (*(RwFreeList**)0xC97B84)
#define CreatedVBFreeList (*(RwFreeList**)0xC97B88)
#define DynamicVertexBufferFreeList (*(RwFreeList**)0xC97B90)
#define CurrentDynamicVertexBufferManager (*(int*)0xC97B94)
#define DynamicVertexBufferManager (*(IDirect3DVertexBuffer9***)0xC97BB8)
#define OffSetDynamicVertexBufferManager (*(int**)0xC97B98)
#define SizeDynamicVertexBufferManager (*(int**)0xC97BA8)
#define DynamicVertexBufferList (*(RwD3D9DynamicVertexBuffer**)0xC97B8C)
#define StrideList (*(RwD3D9Stride**)0xC97B7C)
#define IndexBuffer3D (*(IDirect3DIndexBuffer9 **)0xC9AB40)
#define VertexDeclIm3DNoTex (*(IDirect3DVertexDeclaration9 **)0xC9AB4C)
#define VertexDeclIm3D (*(IDirect3DVertexDeclaration9 **)0xC9AB48)
#define VertexDeclIm3DOld (*(IDirect3DVertexDeclaration9 **)0xC9AB44)

#define _RwResourcesAllocateResEntry(owner, ownerRef, size, destroyNotify)((RwResEntry* (__cdecl*)(void *,RwResEntry **,RwInt32,RwResEntryDestroyNotify))0x807ED0)(owner, ownerRef, size, destroyNotify) 
#define _RwResourcesFreeResEntry(entry)((RwBool (__cdecl*)(RwResEntry * ))0x807DE0)(entry) 
#define _rwD3D9VSSetActiveWorldMatrix(matrix)((RwBool (__cdecl*)(RwMatrix * ))0x764650)(matrix) 
#define _RwMatrixMultiply(matrixOut,MatrixIn1,MatrixIn2)((RwMatrix * (__cdecl*)(RwMatrix *,const RwMatrix *,const RwMatrix * ))0x7F18B0)(matrixOut,MatrixIn1,MatrixIn2) 

//#define _RpSkinAtomicGetHAnimHierarchy(matrix)((RwBool (__cdecl*)(RwMatrix * ))0x764650)(matrix) 

 
#define _rwFreeListAlloc(freeList, hint) ((void *(__cdecl*)(RwFreeList*, UINT))0x801C30)(freeList, hint)
#define _rwFreeListFree(freeList, ptr) ((void (__cdecl*)(RwFreeList*, void*))0x801D50)(freeList, ptr)

#define RwFreeListPurge(freelist) ((int (__cdecl*)(RwFreeList *))0x801980)(freelist)
#define _RwFreeListDestroy(freelist) ((RwBool (__cdecl*)(RwFreeList *))0x801B80)(freelist)
#define RwFreeListPurgeAllFreeLists() ((void (__cdecl*)())0x801F90)()
#define RwProcessorForceSinglePrecision() ((void (__cdecl*)())0x857432)()

#define rwD3D9RasterClose() ((void (__cdecl*)())0x4CC170)()
#define rwD3D9CheckTextureFormat(raster,format) ((bool (__cdecl*)(RwRaster *,int))0x4CC5C0)(raster,format)
#define rwD3D9CreateTextureRaster(d3draster,raster) ((bool (__cdecl*)(RwD3D9Raster*, RwRaster *))0x4CB7C0)(d3draster,raster)
#define rwD3D9CreateCameraTextureRaster(d3draster,raster) ((bool (__cdecl*)(RwD3D9Raster*, RwRaster *))0x4CB9C0)(d3draster,raster)
#define rwD3D9CreateZBufferRaster(raster,d3draster) ((bool (__cdecl*)(RwRaster*,RwD3D9Raster*))0x4CCD70)(raster,d3draster)
#define gtaD3D9CreateTexture(width,height,levels,fmt,tex) ((HRESULT (__cdecl*)(UINT, UINT, UINT, D3DFORMAT, IDirect3DTexture9 **))0x4CB935)(width,height,levels,fmt,tex)

#define RwStreamRead(str,buffer,length) ((RwUInt32 (__cdecl*)(RwStream * ,void *,RwUInt32 ))0x7EC9D0)(str,buffer,length)
#define RwStreamFindChunk(str,type,lengthOut,versionOut) ((RwBool (__cdecl*)(RwStream * , RwUInt32, RwUInt32 *, RwUInt32 *))0x7ED2D0)(str,type,lengthOut,versionOut)

#define RwRasterLock(raster,level,lockMode) ((RwUInt8* (__cdecl*)(RwRaster * , RwUInt8 , RwInt32 ))0x7FB2D0)(raster,level,lockMode)
#define rpD3D9SkinVertexShaderMatrixUpdate(mat,atomic,skin) ((RpAtomic* (__cdecl*)(RwMatrix * , RpAtomic* , RpSkin* ))0x7C78A0)(mat,atomic,skin)
#define RwRasterUnlock(raster) ((RwRaster* (__cdecl*)(RwRaster * ))0x7FAEC0)(raster)

#define RwTextureCreate(raster) ((RwTexture* (__cdecl*)(RwRaster * ))0x7F37C0)(raster)
#define RwTextureSetName(texture,name) ((RwTexture* (__cdecl*)(RwTexture * , const RwChar * ))0x7F38A0)(texture,name)
#define RwTextureSetMaskName(texture,maskName) ((RwTexture* (__cdecl*)(RwTexture * , const RwChar * ))0x7F3910)(texture,maskName)

#define rwD3D9VertexBufferManagerClose() ((void (__cdecl*)())0x7F5D20)()
#define rwD3D9Im2DRenderClose() ((void (__cdecl*)())0x7FB5F0)()
#define rwD3D9Im3DRenderClose() ((void (__cdecl*)())0x80DFB0)()
#define rwD3D9DynamicVertexBufferRelease() ((void (__cdecl*)())0x7F5840)()
#define rxD3D9VideoMemoryRasterListRelease() ((void (__cdecl*)())0x4CB640)()
#define RwMatrixInvert(out,in) ((RwMatrix* (__cdecl*)(RwMatrix *, RwMatrix *))0x7F2070)(out,in)
#define RwD3D9SetTransform(state,matrix) ((bool (__cdecl*)(int, RwMatrix *))0x7FA390)(state,matrix)
#define RwFrameGetLTM(frame) ((RwMatrix* (__cdecl*)(RwFrame*))0x7F0990)(frame)
#define rxD3D9VideoMemoryRasterListRestore() ((bool (__cdecl*)())0x4CC970)()
#define rwD3D9DynamicVertexBufferRestore() ((bool (__cdecl*)())0x7F58D0)()
#define rwD3D9DynamicVertexBufferManagerForceDiscard() ((bool (__cdecl*)())0x7F5800)()
#define rwD3D9DynamicVertexBufferManagerCreate() ((bool (__cdecl*)())0x7F5940)()


#define rwD3D9RenderStateReset() ((bool (__cdecl*)())0x7FD100)()
#define rwD3D9Im2DRenderOpen() ((bool (__cdecl*)())0x7FB480)()
#define rwD3D9Im3DRenderOpen() ((bool (__cdecl*)())0x80E020)()
#define D3D9RestoreDeviceCallback (*(rwD3D9DeviceRestoreCallBack*)0xC980B0)
#define rwD3D9ConvertToTriList(to,from,nIndices,minVert) ((int (__cdecl*)(RxVertexIndex *,RxVertexIndex *,int,int))0x756830)(to,from,nIndices,minVert)
#define rwD3D9SortTriangles(a,b) ((int (__cdecl*)(const void *, const void *))0x7567A0)(a,b)


#define rwProcessorRelease() ((void (__cdecl*)())0x85742B)()
#define D3D9DeviceReleaseVideoMemory() ((void (__cdecl*)())0x7F7F70)()
#define RwD3D9CameraAttachWindow(cam, hWnd) ((bool (__cdecl*)(RwCamera *, HWND*))0x7F8D70)(cam, hWnd)
#define RwRasterDestroy(raster) ((bool (__cdecl*)(RwRaster *))0x7FB020)(raster)
#define RwRasterCreate(width, height, depth, flags) ((RwRaster* (__cdecl*)(int, int, int, int))0x7FB230)(width, height, depth, flags)

#define RwFreeListCreate(entrySize, entriesPerBlock, alignment, hint) ((RwFreeList *(__cdecl*)(int, int, int, UINT))0x801980)(entrySize, entriesPerBlock, alignment, hint)
#define D3D9SetPresentParameters(dispMode,bFullscreen,fmt)((void (__cdecl *)(D3DDISPLAYMODE*,bool,D3DFORMAT))0x7F6CB0)(dispMode,bFullscreen,fmt)
#define D3D9CalculateMaxMultisamplingLevels() ((void(__cdecl *)())0x7F6BF0)()
#define rwD3D9CameraBeginUpdate(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x7F8F20)(a1, a2, a3)
#define rwD3D9CameraEndUpdate(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x7F98D0)(a1, a2, a3)
#define rwD3D9RGBToPixel(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x7FEE20)(a1, a2, a3)
#define rwD3D9PixelToRGB(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x7FF070)(a1, a2, a3)
#define rwD3D9RasterSetFromImage(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x8001E0)(a1, a2, a3)
#define rwD3D9ImageGetFromRaster(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x7FF270)(a1, a2, a3)
#define rwD3D9RasterDestroy(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x4CBB00)(a1, a2, a3)
#define rwD3D9RasterCreate(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x4CCE60)(a1, a2, a3)
#define rwD3D9ImageFindRasterFormat(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x7FFF00)(a1, a2, a3)
#define rwD3D9TextureSetRaster(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x4CBD40)(a1, a2, a3)
#define rwD3D9RasterLock(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x4C9F90)(a1, a2, a3)
#define rwD3D9RasterUnlock(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x4CA290)(a1, a2, a3)
#define rwD3D9RasterLockPalette(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x4CA4E0)(a1, a2, a3)
#define rwD3D9RasterUnlockPalette(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x4CA540)(a1, a2, a3)
#define rwD3D9RasterClear(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x4CB4E0)(a1, a2, a3)
#define rwD3D9RasterClearRect(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x4CB4C0)(a1, a2, a3)
#define rwD3D9RasterRender(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x4CAE40)(a1, a2, a3)
#define rwD3D9RasterRenderScaled(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x4CAE80)(a1, a2, a3)
#define rwD3D9RasterRenderFast(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x4CAE60)(a1, a2, a3)
#define rwD3D9SetRasterContext(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x4CB524)(a1, a2, a3)
#define rwD3D9RasterSubRaster(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x4CBD50)(a1, a2, a3)
#define rwD3D9NativeTextureGetSize(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x4CD360)(a1, a2, a3)
#define rwD3D9NativeTextureWrite(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x4CD4D0)(a1, a2, a3)
#define rwD3D9NativeTextureRead(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x4CD820)(a1, a2, a3)
#define rwD3D9RasterGetMipLevels(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x4CBCB0)(a1, a2, a3)
#define rwD3D9CameraClear(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x7F7730)(a1, a2, a3)
#define rwD3D9RasterShowRaster(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x7F99B0)(a1, a2, a3)
#define D3D9NullStandard(a1, a2, a3)((bool(__cdecl *)(void*, void*, int))0x7F7530)(a1, a2, a3)

#define GetD3D9Raster(r)\
        (&((_rwD3DRaster*)r)->dr)
#define GetVKRaster(r)\
        (&((_rwVKRaster*)r)->dr)
#define GetD3D1XRaster(r)\
        (&((_rwD3D1XRaster*)r)->dr)

#endif