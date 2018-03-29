#ifndef GTA_SA_PTRS_H
#define GTA_SA_PTRS_H
//#include <game_sa\rw\rwcore.h>
//#include <game_sa\rw\rpworld.h>
//#include <game_sa\rw\rpskin.h>
#include <game_sa\RenderWare.h>

/*
*	RW MACROS(plugin sdk lacks them for some reason)
*/

#define RwCameraSetRasterMacro(_camera, _raster)                \
    (((_camera)->frameBuffer = (_raster)), (_camera))

#define RwCameraSetZRasterMacro(_camera, _raster)               \
    (((_camera)->zBuffer = (_raster)), (_camera))

#define RwCameraSetZRaster(_camera, _raster)                    \
    RwCameraSetZRasterMacro(_camera, _raster)

#define RwCameraSetRaster(_camera, _raster)                     \
    RwCameraSetRasterMacro(_camera, _raster)

#define RwCameraGetFrameMacro(_camera)                          \
    ((RwFrame *)rwObjectGetParent((_camera)))

#define RwCameraGetFrame(_camera)                               \
    RwCameraGetFrameMacro(_camera)

#define RwRasterGetTypeMacro(_raster) \
    (((_raster)->cType) & rwRASTERTYPEMASK)

#define RwRasterGetType(_raster)                  \
    RwRasterGetTypeMacro(_raster)


#define RwV3dScaleMacro(o, a, s)                                \
MACRO_START                                                     \
{                                                               \
    (o)->x = (((a)->x) * ( (s)));                               \
    (o)->y = (((a)->y) * ( (s)));                               \
    (o)->z = (((a)->z) * ( (s)));                               \
}                                                               \
MACRO_STOP

#define RwV3dScale(o, a, s)             RwV3dScaleMacro(o, a, s)

class CCamera;
class CD3D1XTexture;
class CD3D1XVertexDeclaration;

/*
*	RenderWare structures
*/

struct TextureFormat
{
	unsigned int	platformId;
	unsigned int	filterMode	: 8;
	unsigned int	uAddressing	: 4;
	unsigned int	vAddressing	: 4;
	char			name[32];
	char			maskName[32];
};

struct RasterFormat
{
	unsigned int rasterFormat;
	union {
		D3DFORMAT	d3dFormat; // SA, see D3DFORMAT on MSDN
		unsigned int hasAlpha; // GTA3 & VC
	};
	unsigned short	width;
	unsigned short	height;
	unsigned char	depth;
	unsigned char	numLevels;
	unsigned char	rasterType;
	union {
		unsigned char compression; // GTA3 & VC
		struct { // SA
			unsigned char alpha : 1;
			unsigned char cubeTexture : 1;
			unsigned char autoMipMaps : 1;
			unsigned char compressed : 1;
			unsigned char pad : 4;
		};
	};
};

/*struct RsGlobalType
{
	char*	AppName;
	int		MaximumWidth;
	int		MaximumHeight;
	int		frameLimit;
	int		quit;
	int		ps;
};*/

struct RpSkin
{
	RwUInt32			numBones;
	RwUInt32			numBoneIds;
	RwUInt8*			boneIds;
	RwMatrix*			skinToBoneMatrices;
	RwUInt32			maxNumWeightsForVertex;
	RwUInt32*			vertexBoneIndices;
	RwMatrixWeights*	vertexBoneWeights;
	RwUInt32			boneCount;
	RwUInt32			useVS;
	RwUInt32			boneLimit;
	RwUInt32			numMeshes;
	RwUInt32			numRLE;
	RwUInt8*			meshBoneRemapIndices;
	RwUInt32			meshBoneRLECount;
	void*				meshBoneRLE;
	void*				field_3C;
};

struct RwMatrix4x3 {
	RwV4d a[3];
};

struct RpSkinGlobals
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

struct sScene {
	RpWorld*	curWorld;
	RwCamera*	curCamera;             /* Current camera */
};

/* 
*	RenderWare D3D9 structures
*/

struct RwD3D9Raster
{
	union
	{
		IDirect3DTexture9* texture;
		IDirect3DSurface9* surface;
	};
	unsigned char*       palette;
	unsigned char        alpha;
	unsigned char        cubeTextureFlags; /* 0x01 IS_CUBEMAP_TEX */
	unsigned char        textureFlags;     /* 0x01 HAS_MIP_MAPS
										   0x10 IS_COMPRESSED */
	unsigned char        lockFlags;
	IDirect3DSurface9*   lockedSurface;
	D3DLOCKED_RECT       lockedRect;
	D3DFORMAT            format;
	IDirect3DSwapChain9* swapChain;
	HWND*                hwnd;
};

struct _rwD3DRaster {
	RwRaster		r;
	RwD3D9Raster	dr;
};

struct RxInstanceData : RwResEntry
{
	RxD3D9ResEntryHeader	header;
	RxD3D9InstanceData		models[];
};

struct RpD3DMeshHeader : RpMeshHeader
{
	RpMesh meshes[];
};

struct RxD3D9Pipelines
{
	RxD3D9AllInOneInstanceCallBack		instance;
	RxD3D9AllInOneReinstanceCallBack	reinstance;
	RxD3D9AllInOneLightingCallBack		lighting;
	RxD3D9AllInOneRenderCallBack		render;
};

struct RpSkinPipeCB
{
	RxD3D9AllInOneRenderCallBack					render;
	RxD3D9AllInOneLightingCallBack					lighting;
	_rxD3D9VertexShaderBeginCallBack				VSBegin;
	_rxD3D9VertexShaderLightingCallBack				VSLighting;
	_rxD3D9VertexShaderGetMaterialShaderCallBack	VSGetMatShader;
	_rxD3D9VertexShaderMeshRenderCallBack			VSRender;
	_rxD3D9VertexShaderEndCallBack					VSEnd;
};

/*
*	Custom D3D11 structures for renderware
*/

struct RwD3D1XRaster
{
	CD3D1XTexture*		resourse;
	unsigned char*		palette;
	unsigned char       alpha;
	unsigned char       cubeTextureFlags; /* 0x01 IS_CUBEMAP_TEX */
	unsigned char       textureFlags;     /* 0x01 HAS_MIP_MAPS
										   0x10 IS_COMPRESSED */
	unsigned char        lockFlags;
	ID3D11Resource*		 lockedSurface;
	D3DLOCKED_RECT       lockedRect;
	DXGI_FORMAT          format;
	IDXGISwapChain*		 swapChain;
	HWND*                hwnd;
};

struct _rwD3D1XRaster {
	RwRaster r;
	RwD3D1XRaster dr;
};

struct SimpleVertex
{
	RwV3d		pos, normal;
	RwTexCoords uv;
	RwRGBA		color;
};

struct SimpleVertexSkin
{
	RwV3d			pos, normal;
	RwTexCoords		uv;
	RwRGBA			color;
	RwMatrixWeights weights;
	UINT			indices;
};

struct AlphaMesh {
	RxInstanceData* entryptr;
	RwMatrix*		worldMatrix;
	int				meshID;
};

#ifdef RENDER_VK_ENGINE
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
struct _rwVKRaster {
	RwRaster r;
	RwVulkanRaster dr;
};
#endif

/*
*	Custom gta structs
*/
/*struct CBaseModelInfo;
typedef CBaseModelInfo* (*AsAtomicModelInfoPtrCB)(CBaseModelInfo*);
typedef char(*getModelTypeCB)(void*);
typedef void*(*getTimeInfoCB)(void*);
struct vmt_CBaseModelInfo {
	void* Destructor;
	AsAtomicModelInfoPtrCB AsAtomicModelInfoPtr;
	void* AsDamageAtomicModelInfoPtr;
	void* AsLodAtomicModelInfoPtr;
	getModelTypeCB GetModelType;
	getTimeInfoCB GetTimeInfo;
};
struct CBoundingBox
{
	RwBBox m_BBox;
	RwSphere m_BSphere;
};
struct CColModel
{
	CBoundingBox m_BBox;
	int a;
	void *m_pColData;
};
struct CBaseModelInfo {
	vmt_CBaseModelInfo*	vmt;
	unsigned int   m_dwKey;
	unsigned short m_wUsageCount;
	signed short   m_wTxdIndex;
	unsigned char  m_nAlpha; // 0 - 255
	unsigned char  m_n2dfxCount;
	short          m_w2dfxIndex;
	short          m_wObjectInfoIndex;
	union {
		unsigned short m_wFlags;
		struct {
			unsigned char m_bHasBeenPreRendered : 1; // we use this because we need to apply changes only once
			unsigned char m_bAlphaTransparency : 1;
			unsigned char m_bIsLod : 1;
			unsigned char m_bDontCastShadowsOn : 1;
			unsigned char m_bDontWriteZBuffer : 1;
			unsigned char m_bDrawAdditive : 1;
			unsigned char m_bDrawLast : 1;
			unsigned char m_bDoWeOwnTheColModel : 1;
			union {
				struct {
					unsigned char m_bCarmodIsWheel : 1;
					unsigned char bUnknownFlag9 : 1;
					unsigned char bUnknownFlag10 : 1;
					unsigned char m_bSwaysInWind : 1;
					unsigned char m_bCollisionWasStreamedWithModel : 1;
					unsigned char m_bDontCollideWithFlyer : 1;
					unsigned char m_bHasComplexHierarchy : 1;
					unsigned char m_bWetRoadReflection : 1;
				};
				struct {
					unsigned char pad0 : 2;
					unsigned char m_nCarmodId : 5;
					unsigned char pad1 : 1;
				};
			};

		};
	};
	CColModel*    		m_pColModel;
	float             m_fDrawDistance;
	struct RwObject*  m_pRwObject;
};*/
/*struct CPtrList {
	void* element;
	CPtrList* next;
};*/

#if (GTA_SA)

/*
*	Global Variables
*/

//#define Scene				(*(sScene *)		0xC17038)
#define RsGlobal			(*(RsGlobalType *)  0xC17040)
#define RwEngineInstance	((void *)			0xC97B24)
#define RwD3D9D3D9ViewTransform			(*(RwMatrix*)	0xC9BC80)
#define RwD3D9D3D9ProjTransform			(*(RwMatrix*)	0x8E2458)
#define RwD3D9ActiveViewProjTransform	(*(RwMatrix**)	0xC97C64)
#define rwD3D9ImmPool		(*(rwIm3DPool **)	0xC9AB30)
#define RpSkinGlobals		(*(RpSkinGlobals *)	0xC978A0)
#define RpResModule			(*(RwModuleInfo *)	0xC9BC58)
#define gColourTop			((RwRGBA *)			0xB72CA0)
#define dgGGlobals			(*(RwCamera**)		0xC9BCC0)
#define CTimer__ms_fTimeStep	(*(float *)0xB7CB5C)
//#define TheCamera				((CCamera *)0xB6F028)
#define CGame__currArea			(*(UINT *)0xB72914) // Current area 0 - inside interiors
#define CWeather__WetRoads	(*(float *)0xC81308)
//A9B0C8     ; CBaseModelInfo *CModelInfo::ms_modelInfoPtrs
#define CModelInfo__ms_modelInfoPtrs ((CBaseModelInfo **)0xA9B0C8)

/*
*	RenderWare methods
*/

// Standard methods(Rw)
#define RwD3DSystem(a1, a2, a3, a4)((bool (__cdecl *)(int, void*, void*,int))0x7F5F70)(a1, a2, a3, a4)
#define _rwD3D9VSSetActiveWorldMatrix(matrix)((RwBool (__cdecl*)(RwMatrix * ))0x764650)(matrix) 
#define rwD3D9ConvertToTriList(to,from,nIndices,minVert) ((int (__cdecl*)(RxVertexIndex *,RxVertexIndex *,int,int))0x756830)(to,from,nIndices,minVert)
#define rwD3D9SortTriangles(a,b) ((int (__cdecl*)(const void *, const void *))0x7567A0)(a,b)

#define _RwResourcesFreeResEntry(entry)((RwBool (__cdecl*)(RwResEntry * ))0x807DE0)(entry) 
#define _RwResourcesAllocateResEntry(owner, ownerRef, size, destroyNotify)((RwResEntry* (__cdecl*)(void *,RwResEntry **,RwInt32,RwResEntryDestroyNotify))0x807ED0)(owner, ownerRef, size, destroyNotify) 

#define _RwMatrixMultiply(matrixOut,MatrixIn1,MatrixIn2)((RwMatrix * (__cdecl*)(RwMatrix *,const RwMatrix *,const RwMatrix * ))	0x7F18B0)(matrixOut,MatrixIn1,MatrixIn2) 
#define _RwMatrixTranslate(matrixOut,MatrixIn1,ct)		((RwMatrix * (__cdecl*)(RwMatrix *,const RwV3d *,RwOpCombineType ))		0x7F2450)(matrixOut,MatrixIn1,ct) 
#define _RwMatrixScale(matrixOut,MatrixIn1,ct)			((RwMatrix * (__cdecl*)(RwMatrix *,const RwV3d *,RwOpCombineType ))		0x7F22C0)(matrixOut,MatrixIn1,ct) 
#define RwMatrixRotate(matrix, axis, angle, combine)	((void (__cdecl *)(RwMatrix *, const RwV3d *, float, RwOpCombineType))	0x7F1FD0)(matrix, axis, angle, combine)
#define RwMatrixInvert(out,in) ((RwMatrix* (__cdecl*)(RwMatrix *, RwMatrix *))0x7F2070)(out,in)

#define RwCameraBeginUpdate(camera) ((RwCamera *(__cdecl *)(RwCamera *))	0x7EE190)(camera)
#define RwCameraEndUpdate(camera)	((RwCamera *(__cdecl *)(RwCamera *))	0x7EE180)(camera)
#define RwCameraSync(camera)		((RwCamera *(__cdecl *)(RwCamera *))	0x7EE5A0)(camera)
#define RwCameraCreate()			((RwCamera *(__cdecl *)())				0x7EE4F0)()
#define RwCameraDestroy(camera)		((void (__cdecl *)(RwCamera *))			0x7EE4B0)(camera)
#define RwCameraClear(camera, color, clearMode)		((RwCamera *(__cdecl *)(RwCamera *, RwRGBA *, int))	0x7EE340)(camera, color, clearMode)
#define RwCameraSetProjection(camera, projection)	((void (__cdecl *)(RwCamera *, unsigned int))		0x7EE3A0)(camera, projection)
#define RwCameraSetViewWindow(camera, viewWindow)	((void (__cdecl *)(RwCamera *, RwV2d *))			0x7EE410)(camera, viewWindow)
#define RwCameraSetFarClipPlane(camera, farClip)	((RwCamera *(__cdecl *)(RwCamera *, float))			0x7EE2A0)(camera, farClip)
#define RwCameraSetNearClipPlane(camera, nearClip)	((RwCamera *(__cdecl *)(RwCamera *, float))			0x7EE1D0)(camera, nearClip)

#define RwObjectHasFrameSetFrame(object, frame) ((void (__cdecl *)(void *, RwFrame *))0x804EF0)(object, frame)
#define RwFrameCreate()			((RwFrame *(__cdecl *)())		0x7F0410)()
#define RwFrameDestroy(frame)	((void (__cdecl *)(RwFrame *))	0x7F05A0)(frame)
#define RwFrameTranslate(frame, pos, combine) ((void(__cdecl *)(RwFrame *, RwV3d *, int))0x7F0E30)(frame, pos, combine)
#define RwFrameTransform(frame, matrix, combine) ((RwFrame * (__cdecl *)(RwFrame *, const RwMatrix *, int)) 0x7F0F70)(frame, matrix, combine)
#define RwFrameGetLTM(frame) ((RwMatrix* (__cdecl*)(RwFrame*))0x7F0990)(frame)

#define RwStreamRead(str,buffer,length) ((RwUInt32 (__cdecl*)(RwStream * ,void *,RwUInt32 ))0x7EC9D0)(str,buffer,length)
#define RwStreamFindChunk(str,type,lengthOut,versionOut) ((RwBool (__cdecl*)(RwStream * , RwUInt32, RwUInt32 *, RwUInt32 *))0x7ED2D0)(str,type,lengthOut,versionOut)

#define RwRasterCreate(width, height, depth, flags) ((RwRaster* (__cdecl*)(int, int, int, int))0x7FB230)(width, height, depth, flags)
#define RwRasterDestroy(raster) ((bool (__cdecl*)(RwRaster *))0x7FB020)(raster)
#define RwRasterLock(raster,level,lockMode) ((RwUInt8* (__cdecl*)(RwRaster * , RwUInt8 , RwInt32 ))0x7FB2D0)(raster,level,lockMode)
#define RwRasterUnlock(raster) ((RwRaster* (__cdecl*)(RwRaster * ))0x7FAEC0)(raster)

#define RwTextureCreate(raster) ((RwTexture* (__cdecl*)(RwRaster * ))0x7F37C0)(raster)
#define RwTextureSetName(texture,name) ((RwTexture* (__cdecl*)(RwTexture * , const RwChar * ))0x7F38A0)(texture,name)
#define RwTextureSetMaskName(texture,maskName) ((RwTexture* (__cdecl*)(RwTexture * , const RwChar * ))0x7F3910)(texture,maskName)
#define _RwTexDictionaryFindNamedTexture(texDir,texture) ((RwTexture* (__cdecl*)(RwTexDictionary * , RwChar * ))0x7F39F0)(texDir,texture)

#define RwIm3DTransform(pVerts,numVerts,ltm,flags) ((void* (__cdecl*)(RwIm3DVertex *, RwUInt32 ,RwMatrix *, RwUInt32))0x7EF450)(pVerts,numVerts,ltm,flags)
#define RwIm3DRenderPrimitive(primType) ((RwBool (__cdecl*)(RwPrimitiveType))0x7EF6B0)(primType)
#define RwIm3DEnd() ((RwBool (__cdecl*)())0x7EF520)()
#define RwIm3DRenderIndexedPrimitive(primType,indices,numIndices) ((RwBool (__cdecl*)(RwPrimitiveType,RwImVertexIndex*,RwInt32))0x7EF550)(primType,indices,numIndices)

#define RwV3dTransformPoints(pointsOut,pointsIn,numPoints,matrix) ((RwV3d * (__cdecl*)(RwV3d * ,const RwV3d *,RwInt32,const RwMatrix *))0x7EDD90)(pointsOut,pointsIn,numPoints,matrix)

#define RwProcessorForceSinglePrecision() ((void (__cdecl*)())0x857432)()

// Simple platform-abstraction layer(Rs)
#define RsCameraShowRaster(camera)	((int (__cdecl *)(RwCamera *))			0x619440)(camera)
#define RsCameraBeginUpdate(camera)	((signed int (__cdecl *)(RwCamera *))	0x619450)(camera)

// Plugins(Rp)
#define RpMaterialGetFxEnvShininess(material) ((float (__cdecl *)(RpMaterial *)) 0x5D70D0)(material)
#define RpWorldAddCamera(world,cam)	((RpWorld* (__cdecl*) (RpWorld *, RwCamera *)) 0x750F20)(world,cam)
#define rpD3D9SkinVertexShaderMatrixUpdate(mat,atomic,skin) ((RpAtomic* (__cdecl*)(RwMatrix * , RpAtomic* , RpSkin* ))0x7C78A0)(mat,atomic,skin)

/*
*	GTA Class methods
*/

#define CCustomCarEnvMapPipeline__GetFxSpecSpecularity(material) ((float (__cdecl *)(RpMaterial *)) 0x5D8B90)(material)

/*
*	GTA Global methods
*/

#define Im2DRenderQuad(x_offset, y_offset, width, height, a5, a6, a7) ((RwCamera *(__cdecl *)(float , float , int , int , int, int, int))0x705A20)(x_offset, y_offset,width,height,a5,a6,a7)
#define CameraSize(camera, rect, viewWindow, aspectRatio) ((void (__cdecl *)(RwCamera *, void *, float, float))0x72FC70)\
	(camera, rect, viewWindow, aspectRatio)

#elif (GTA_III)
/*
*	Global Variables
*/

#define RwEngineInstance((void *)			0x661228)
#define RpSkinGlobals	(*(rpSkinGlobals *)	0x663C8C)
#define RpResModule		(*(RwModuleInfo *)	0x732A18)
#define dgGGlobals		(*(RwCamera**)		0x6F1D08)

/*
*	RenderWare methods
*/

// Standard methods(Rw)
#define RwD3DSystem(a1, a2, a3, a4)((bool (__cdecl *)(int, void*, void*,int))0x5B71E0)(a1, a2, a3, a4)
#define RwD3D9SetTransform(state,matrix) ((bool (__cdecl*)(int, RwMatrix *))0x5BB1D0)(state,matrix)
#define rwD3D9SortTriangles(a,b) ((int (__cdecl*)(const void *, const void *))0x5DF5D0)(a,b)

#define _RwResourcesFreeResEntry(entry)((RwBool (__cdecl*)(RwResEntry * ))0x5C3080)(entry) 
#define _RwResourcesAllocateResEntry(owner, ownerRef, size, destroyNotify)((RwResEntry* (__cdecl*)(void *,RwResEntry **,RwInt32,RwResEntryDestroyNotify))0x5C3170)(owner, ownerRef, size, destroyNotify) 

#define _RwMatrixMultiply(matrixOut,MatrixIn1,MatrixIn2)((RwMatrix * (__cdecl*)(RwMatrix *,const RwMatrix *,const RwMatrix * ))0x5A28F0)(matrixOut,MatrixIn1,MatrixIn2) 
#define RwMatrixInvert(out,in) ((RwMatrix* (__cdecl*)(RwMatrix *, RwMatrix *))0x5A2C90)(out,in)

#define RwProcessorForceSinglePrecision() ((void (__cdecl*)())0x5E435E)()

#define RwStreamRead(str,buffer,length) ((RwUInt32 (__cdecl*)(RwStream * ,void *,RwUInt32 ))0x5A3AD0)(str,buffer,length)
#define RwStreamFindChunk(str,type,lengthOut,versionOut) ((RwBool (__cdecl*)(RwStream * , RwUInt32, RwUInt32 *, RwUInt32 *))0x5AA540)(str,type,lengthOut,versionOut)

#define RwRasterCreate(width, height, depth, flags) ((RwRaster* (__cdecl*)(int, int, int, int))0x5AD930)(width, height, depth, flags)
#define RwRasterDestroy(raster) ((bool (__cdecl*)(RwRaster *))0x5AD780)(raster)
#define RwRasterLock(raster,level,lockMode) ((RwUInt8* (__cdecl*)(RwRaster * , RwUInt8 , RwInt32 ))0x5AD9D0)(raster,level,lockMode)
#define RwRasterUnlock(raster) ((RwRaster* (__cdecl*)(RwRaster * ))0x5AD6F0)(raster)

#define RwTextureCreate(raster) ((RwTexture* (__cdecl*)(RwRaster * ))0x5A72D0)(raster)
#define RwTextureSetName(texture,name) ((RwTexture* (__cdecl*)(RwTexture * , const RwChar * ))0x5A73B0)(texture,name)
#define RwTextureSetMaskName(texture,maskName) ((RwTexture* (__cdecl*)(RwTexture * , const RwChar * ))0x5A7420)(texture,maskName)

#define RwFrameGetLTM(frame) ((RwMatrix* (__cdecl*)(RwFrame*))0x5A1CE0)(frame)


#elif (GTA_VC)
/*
*	RenderWare methods
*/

// Standard methods(Rw)
#define RwD3DSystem(a1, a2, a3, a4)((bool (__cdecl *)(int, void*, void*,int))0x65B620)(a1, a2, a3, a4)
#endif



/*
*	RenderWare macros defines
*/
#define BYTEn(x, n)   (*((BYTE*)&(x)+n))
#define LOBYTE(x)   (*((BYTE*)&(x)))
#define BYTE1(x)   BYTEn(x,  1)         // byte 1 (counting from 0)
#define BYTE2(x)   BYTEn(x,  2)
#define BYTE3(x)   BYTEn(x,  3)
#define BYTE4(x)   BYTEn(x,  4)

#define _RWPLUGINOFFSET(_type,_base,_offset) \
	*(_type**)((UINT)_base + (_offset))

#define _RWRESOURCESGLOBAL(var) (_RWPLUGINOFFSET(rwResourcesGlobals,  \
    RwEngineInstance, RpResModule.globalsOffset)->var)

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

#define GetD3D9Raster(r)\
        (&((_rwD3DRaster*)r)->dr)
#define GetVKRaster(r)\
        (&((_rwVKRaster*)r)->dr)
#define GetD3D1XRaster(r)\
        (&((_rwD3D1XRaster*)r)->dr)

#endif