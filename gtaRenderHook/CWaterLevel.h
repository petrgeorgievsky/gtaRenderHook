#pragma once
#include "gta_sa_ptrs.h"
struct CRenPar
{
    float baseHeight, wave0Height, wave1Height;
    char flowX;
    char flowY;
    short pad;
};
struct CWaterTriangle
{
    short a, b, c;
    BYTE d, e;
};
struct CWaterQuad
{
    short a, b, c, d;
    short m_flags;
};
struct CWaterVertice
{
    short posX, posY;
    CRenPar renPar;
};
#define CWaterLevel_RenderWater() ((void (__cdecl *)())0x6EF650)()
#define CWaterLevel_SetCameraRange() ((void (__cdecl *)())0x6E9C80)()
#define CWaterLevel_RenderBoatWakes() ((void (__cdecl *)())0x6ED9A0)()
#define CWaterLevel_RenderWaterRectangle(x,y,z,w,a,b,c,d) ((void (__cdecl *)(int,int,int,int,CRenPar,CRenPar,CRenPar,CRenPar))0x6EC5D0)(x,y,z,w,a,b,c,d)
#define CWaterLevel_RenderDetailedSeaBedSegment(x,y,a,b,c,d) ((void (__cdecl *)(int, int, float, float, float, float))0x6E6A10)(x,y,a,b,c,d)
#define CWaterLevel_RenderSeaBedSegment(x,y,a,b,c,d) ((void (__cdecl *)(int, int, float, float, float, float))0x6E6870)(x,y,a,b,c,d)
#define CWaterLevel_RenderWaterTriangle(x0,y0,a,x1,y1,b,x2,y2,c) ((void (__cdecl *)(int, int, CRenPar, int, int, CRenPar, int, int, CRenPar))0x6EE240)(x0,y0,a,x1,y1,b,x2,y2,c)

#define CWaterLevel_RenderFlatWaterRectangle(x,y,z,w,a,b,c,d) ((void (__cdecl *)(int,int,int,int,CRenPar,CRenPar,CRenPar,CRenPar))0x6EBEC0)(x,y,z,w,a,b,c,d)

#define TempBufferVerticesStored (*(UINT *)0xC4B950)
#define TempBufferIndicesStored (*(UINT *)0xC4B954)
#define TempVertexBuffer ((RwIm3DVertex *)0xC4D958)
#define TempBufferRenderIndexList ((USHORT *)0xC4B958)
#define CTimer__m_snTimeInMilliseconds (*(UINT *)0xB7CB84)
#define WaterColor (*(RwRGBA *)0xC2116C)
//C22910     ; CWaterVertice CWaterLevel::m_aVertices[1021]
class CVector;
class CWaterLevel
{
public:
    static void RenderWater();
    static void SetCameraRange();
    static void RenderBoatWakes();
    static void RenderWaterRectangle( int x, int y, int z, int w, const CRenPar & a, const CRenPar & b, const CRenPar & c, const CRenPar & d );
    static void RenderFlatWaterRectangle( int x, int y, int z, int w, const CRenPar & a, const CRenPar & b, const CRenPar & c, const CRenPar & d );
    static void SplitWaterRectangleAlongYLine( int m, int x, int y, int z, int w, const CRenPar & a, const CRenPar & b, const CRenPar & c, const CRenPar & d );
    static void SplitWaterRectangleAlongXLine( int m, int x, int y, int z, int w, const CRenPar & a, const CRenPar & b, const CRenPar & c, const CRenPar & d );

    static void CalculateWavesForCoordinate( int x, int y, float a3, float a4, float *resHeight, float *diffuse, float *sunGlare, CVector *resNormal );

    static void RenderDetailedSeaBedSegment( int x, int y, float a, float b, float c, float d );
    static void RenderSeaBedSegment( int x, int y, float a, float b, float c, float d );
    static void RenderSeaBed();
    static void RenderWaterTriangle( const CWaterVertice & a, const CWaterVertice & b, const CWaterVertice & c );
    static void RenderAndEmptyRenderBuffer();

    static void GenerateNormals();
    static void RenderFlatWaterRectangle_OneLayer( int x, int y, int z, int w, const CRenPar & a, const CRenPar & b, const CRenPar & c, const CRenPar & d, int e );
    static void RenderDetailedWaterRectangle_OneLayer( int x, int y, int z, int w, const CRenPar & a, const CRenPar & b, const CRenPar & c, const CRenPar & d, int e );
    static void RenderHighDetailWaterRectangle_OneLayer( int x, int y, int z, int w, const CRenPar & a, const CRenPar & b, const CRenPar & c, const CRenPar & d, int e, int xToYHalfRatio, int a23, signed int halfXSize, signed int halfYSize );
    static UINT	&m_NumBlocksOutsideWorldToBeRendered;
    static UINT	&m_nNumOfWaterTriangles;
    static UINT	&m_nNumOfWaterQuads;
    static SHORT* m_BlocksToBeRenderedOutsideWorldX;
    static SHORT* m_BlocksToBeRenderedOutsideWorldY;
    static float &m_CurrentFlowX;
    static float &m_CurrentFlowY;
    static float *g_aFlowIntensity;
    static unsigned int &m_nWaterTimeOffset;
    static CWaterTriangle*	m_aTriangles;
    static CWaterVertice*	m_aVertices;
    static CWaterQuad*		m_aQuads;

};

class CMaths
{
public:
    static float* ms_SinTable;
};