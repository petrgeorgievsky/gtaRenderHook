#include "stdafx.h"
#include "CWaterLevel.h"
#include "CustomWaterPipeline.h"
#include "CustomSeabedPipeline.h"

#include <game_sa\CCamera.h>
#include <game_sa\CGame.h>
#include <game_sa\CTimer.h>
#include <game_sa\CWeather.h>
#include <game_sa\CVector.h>

#define _USE_MATH_DEFINES
#include <math.h>

UINT &CWaterLevel::m_NumBlocksOutsideWorldToBeRendered = *(UINT*)0xC215EC;
UINT &CWaterLevel::m_nNumOfWaterTriangles = *(UINT*)0xC22884;
UINT &CWaterLevel::m_nNumOfWaterQuads = *(UINT*)0xC22888;

USHORT* CWaterLevel::m_BlocksToBeRenderedOutsideWorldX = (USHORT*)0xC21560;
USHORT* CWaterLevel::m_BlocksToBeRenderedOutsideWorldY = (USHORT*)0xC214D0;

CWaterVertice* CWaterLevel::m_aVertices = (CWaterVertice*)0xC22910;
CWaterTriangle* CWaterLevel::m_aTriangles = (CWaterTriangle*)0xC22854;
CWaterQuad* CWaterLevel::m_aQuads = (CWaterQuad*)0xC21C90;
float &CWaterLevel::m_CurrentFlowX= *(float*)0xC22890;
float &CWaterLevel::m_CurrentFlowY= *(float*)0xC22894;

float *CMaths::ms_SinTable = (float*)0xBB3DFC;
float *CWaterLevel::g_aFlowIntensity = (float*)0x8D38C8;
unsigned int &CWaterLevel::m_nWaterTimeOffset = *(unsigned int*)0xC228A4;

//     ; CWaterVertice CWaterLevel::m_aVertices[1021]
//
float TextureShiftU = 0;
float TextureShiftV = 0;
float TextureShiftSecondU = 0;
float TextureShiftSecondV = 0;
float TextureShiftHighLightU = 0;
float TextureShiftHighLightV = 0;
float CurrentTextureShiftU = 0.5;
float CurrentTextureShiftV = 0.5;
float CurrentTextureShiftSecondU = 0.5;
float CurrentTextureShiftSecondV = 0.5;
// GTA Water has fixed speed depending on framerate(or perhaps I'm wrong), but R* didn't account for unlocked framerate
// TODO: Fix this issue
float WATER_UPDATE_FRAMERATE = 25.0;
void CWaterLevel::RenderWater()
{
	if (!CGame::CanSeeWaterFromCurrArea())
		return;
	SetCameraRange();

	TempBufferIndicesStored = 0;
	TempBufferVerticesStored = 0;
	WaterColor = { 255,255,255,255 };
	float waterSpeedX = (CTimer::ms_fTimeStep * m_CurrentFlowX) / WATER_UPDATE_FRAMERATE;
	float waterSpeedY = (CTimer::ms_fTimeStep * m_CurrentFlowY) / WATER_UPDATE_FRAMERATE;

	CurrentTextureShiftU += 0.08f * waterSpeedX;
	CurrentTextureShiftV += 0.08f * waterSpeedY; 
	CurrentTextureShiftSecondU += 0.04f * waterSpeedX;
	CurrentTextureShiftSecondU += 0.04f * waterSpeedY;
	
	if (CurrentTextureShiftU > 1.0)
		CurrentTextureShiftU -= 1.0;
	if (CurrentTextureShiftV > 1.0)
		CurrentTextureShiftV -= 1.0;
	if (CurrentTextureShiftSecondU > 1.0)
		CurrentTextureShiftSecondU -= 1.0;
	if (CurrentTextureShiftSecondV > 1.0)
		CurrentTextureShiftSecondV -= 1.0;

	// R* MAGIC
	float magic0 = (CTimer::m_snTimeInMilliseconds & 0xFFF) * 0.0015339808f;
	float magic1 = (CTimer::m_snTimeInMilliseconds & 0x1FFF) * 0.00076699042f;
	float magic2 = rand() * 0.000030518509f / WATER_UPDATE_FRAMERATE;

	TextureShiftU = sin(magic0) * CWeather::Wavyness * 0.08f + CurrentTextureShiftU;
	TextureShiftV = cos(magic0) * CWeather::Wavyness * 0.08f + CurrentTextureShiftV;
	// R* copypast? or is it intended this way?
	TextureShiftSecondU = CurrentTextureShiftSecondU;
	TextureShiftSecondV = cos(magic1) * 0.024f + CurrentTextureShiftSecondV;
	
	TextureShiftHighLightU = sin(magic1) * 0.1f + magic2;
	TextureShiftHighLightV = cos(magic1) * 0.1f + magic2;

	// Water triangles
	/*for (int j = 0; j < m_nNumOfWaterTriangles; ++j)
	{
		if (m_aTriangles[j].d & 1)
		{
			int a = m_aTriangles[j].a;
			int b = m_aTriangles[j].b;
			int c = m_aTriangles[j].c;
			RenderWaterTriangle(m_aVertices[a], m_aVertices[b], m_aVertices[c]);
			m_aTriangles[j].d &= 0xFEu;
		}
	}*/
	// water quads
	if (m_nNumOfWaterQuads > 0)
	{
		for(int i=0;i<m_nNumOfWaterQuads;i++){
			if (m_aQuads[i].m_flags & 1)
			{
				/*if (byte_C228DC != v0)
				{
					WaterColor.red = ((v25 & 0xF) * 0.0625 * 255.0);
					WaterColor.green = ((v25 / 16 & 0xF) * 0.0625 * 255.0);
					WaterColor.blue = ((v25 / 256 & 0xF) * 0.0625 * 255.0);
					WaterColor.alpha = flt_B7C514;
					WaterColorDebug = WaterColor;
				}*/
				int a = m_aQuads[i].a;
				int b = m_aQuads[i].b;
				int c = m_aQuads[i].c;
				int d = m_aQuads[i].d;
				int minX = min(min(min(m_aVertices[a].posX, m_aVertices[b].posX), m_aVertices[c].posX), m_aVertices[d].posX);
				int maxX = max(max(max(m_aVertices[a].posX, m_aVertices[b].posX), m_aVertices[c].posX), m_aVertices[d].posX);
				int minY = min(min(min(m_aVertices[a].posY, m_aVertices[b].posY), m_aVertices[c].posY), m_aVertices[d].posY);
				int maxY = max(max(max(m_aVertices[a].posY, m_aVertices[b].posY), m_aVertices[c].posY), m_aVertices[d].posY);

				RenderWaterRectangle(minX, maxX, minY, maxY,
					m_aVertices[a].renPar,
					m_aVertices[b].renPar,
					m_aVertices[c].renPar,
					m_aVertices[d].renPar);
				m_aQuads[i].m_flags &= 0xFEu;
			}
		}
	}
	// water rectangles
	for (int k = 0; k < m_NumBlocksOutsideWorldToBeRendered; ++k)
	{

		if (m_BlocksToBeRenderedOutsideWorldX[k] >= 0 && m_BlocksToBeRenderedOutsideWorldX[k] < 12) {
			if(m_BlocksToBeRenderedOutsideWorldY[k] >= 0 && m_BlocksToBeRenderedOutsideWorldY[k] < 12)
				continue;
		}

		float x = m_BlocksToBeRenderedOutsideWorldX[k];
		float y = m_BlocksToBeRenderedOutsideWorldY[k];

		RenderWaterRectangle(
			(x * 500.0f - 3000.0f), ((x + 1) * 500.0f - 3000.0f), (y * 500.0f - 3000.0), ((y + 1) * 500.0f - 3000.0),
			{ 0.0,	1.0,	0.0,	0,0,0 }, { 0.0,	1.0,	0.0,	0,0,0 },
			{ 0.0,	1.0,	0.0,	0,0,0 }, { 0.0,	1.0,	0.0,	0,0,0 });

	}
	
	RenderAndEmptyRenderBuffer();
	
	/*
	if (TempBufferVerticesStored)
	{
		if (RwIm3DTransform(TempVertexBuffer, TempBufferVerticesStored, 0, 1u))
		{
			RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, TempBufferRenderIndexList, TempBufferIndicesStored);
			RwIm3DEnd();
		}
	}*/
	TempBufferIndicesStored = 0;
	TempBufferVerticesStored = 0;

	
	/*

	if (TempBufferVerticesStored)
	{
		if (RwIm3DTransform(TempVertexBuffer, TempBufferVerticesStored, 0, 1u))
		{
			RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, TempBufferRenderIndexList, TempBufferIndicesStored);
			RwIm3DEnd();
		}
	}
	
	TempBufferVerticesStored = 0;
	TempBufferIndicesStored = 0;
	RenderBoatWakes();*/
}
int CameraRangeMaxX;
int CameraRangeMaxY;
int CameraRangeMinX;
int CameraRangeMinY;
float DETAILEDWATERDIST=100;
void CWaterLevel::SetCameraRange()
{
	CVector camPos = TheCamera.GetPosition();

	CameraRangeMinX = 2 * floor((camPos.x - DETAILEDWATERDIST) * 0.5);
	CameraRangeMaxX = 2 * ceil((DETAILEDWATERDIST + camPos.x) * 0.5);
	CameraRangeMinY = 2 * floor((camPos.y - DETAILEDWATERDIST) * 0.5);
	CameraRangeMaxY = 2 * ceil((DETAILEDWATERDIST + camPos.y) * 0.5);
}

void CWaterLevel::RenderBoatWakes()
{
	CWaterLevel_RenderBoatWakes();
}



CRenPar * __cdecl CRenPar_Lerp(CRenPar *a1, CRenPar a, CRenPar b, float t)
{
	float invT = 1.0f - t;
	
	a1->baseHeight = a.baseHeight * invT + b.baseHeight * t;
	a1->wave0Height = a.wave0Height * invT + b.wave0Height * t;
	a1->wave1Height = a.wave1Height * invT + b.wave1Height * t;
	a1->flowX= a1->flowY = 0;
	a1->pad = 0;
	return a1;
}

void CWaterLevel::RenderWaterRectangle(int x0, int x1, int y0, int y1, CRenPar a, CRenPar b, CRenPar c, CRenPar d)
{
	/*int maxY, minY;
	if (y0 < y1) {
		maxY = y1;
		minY = y0;
	}
	else {
		minY = y1;
		maxY = y0;
	}*/
	if (x0 >= CameraRangeMaxX || x1 <= CameraRangeMinX || y0 >= CameraRangeMaxY || y1 <= CameraRangeMinY)
	{
		RenderFlatWaterRectangle(x0, x1, y0, y1, a, b, c, d);
		//CWaterLevel::SetUpWaterFog(x, w, y, z);
	}
	else if (x1 > CameraRangeMaxX)
	{
		SplitWaterRectangleAlongXLine(CameraRangeMaxX, x0, x1, y0, y1, a, b, c, d);
	}
	else if (x0 < CameraRangeMinX)
	{
		SplitWaterRectangleAlongXLine(CameraRangeMinX, x0, x1, y0, y1, a, b, c, d);
	}
	else if (y1 > CameraRangeMaxY)
	{
		SplitWaterRectangleAlongYLine(CameraRangeMaxY, x0, x1, y0, y1, a, b, c, d);
	}
	else if (y0 < CameraRangeMinY)
	{
		SplitWaterRectangleAlongYLine(CameraRangeMinY, x0, x1, y0, y1, a, b, c, d);
	}
	else {
		RenderDetailedWaterRectangle_OneLayer(x0, x1, y0, y1, a, b, c, d, 0);
		RenderDetailedWaterRectangle_OneLayer(x0, x1, y0, y1, a, b, c, d, 1);
	}
}

void CWaterLevel::RenderFlatWaterRectangle(int x, int y, int z, int w, CRenPar a, CRenPar b, CRenPar c, CRenPar d)
{
	RenderFlatWaterRectangle_OneLayer(x, y, z, w, a, b, c, d, 0);
	RenderFlatWaterRectangle_OneLayer(x, y, z, w, a, b, c, d, 1);
	//CWaterLevel_RenderFlatWaterRectangle(x, y, z, w, a, b, c, d);
}

void CWaterLevel::SplitWaterRectangleAlongYLine(int m, int x0, int x1, int y0, int y1, CRenPar a, CRenPar b, CRenPar c, CRenPar d)
{
	CRenPar b_d, a_c;
	float blendFactor = (m - y0) / (y1 - y0) * 1.0f;

	// First split part
	CRenPar_Lerp(&b_d, b, d, blendFactor);
	CRenPar_Lerp(&a_c, a, c, blendFactor);

	RenderWaterRectangle(x0, x1, y0, m, a, b, a_c, b_d);

	// Second split part
	CRenPar_Lerp(&b_d, b, d, blendFactor);
	CRenPar_Lerp(&a_c, a, c, blendFactor);

	RenderWaterRectangle(x0, x1, m, y1, a_c, b_d, c, d);
}

void CWaterLevel::SplitWaterRectangleAlongXLine(int m, int x0, int x1, int y0, int y1, CRenPar a, CRenPar b, CRenPar c, CRenPar d)
{
	CRenPar c_d, a_b;
	float blendFactor = (m - x0) / (x1 - x0) * 1.0f;

	// First split part
	CRenPar_Lerp(&c_d, c, d, blendFactor);
	CRenPar_Lerp(&a_b, a, b, blendFactor);

	RenderWaterRectangle(x0, m, y0, y1, a, a_b, c, c_d);

	// Second split part
	CRenPar_Lerp(&c_d, c, d, blendFactor);
	CRenPar_Lerp(&a_b, a, b, blendFactor);

	RenderWaterRectangle(m, x1, y0, y1, a_b, b, c_d, d);
}

void CWaterLevel::RenderDetailedSeaBedSegment(int x, int y, float a, float b, float c, float d)
{
	float height = -70.0f;
	float UVScale = 32.0;
	UINT  color = 0xFF505050;

	float uvDist = b - a;
	float v7 = (uvDist * 4.0);
	int v8 = 1;
	float v39 = 1;
	if (v7 >= 1)
		v39 = v7;

	float uvDist2 = d - c;
	float v10 = (uvDist2 * 4.0);
	if (v10 >= 1)
		v8 = v10;

	int v34 = v8;
	int v35 = 0;
	if (v39 <= 0)
		return;
	int vertexId = TempBufferVerticesStored;
	int indexId = TempBufferIndicesStored;
	int i = 0;
	do
	{
		int v14 = 0;
		int v38 = 0;
		if (v8 > 0)
		{
			RwIm3DVertex vertex = TempVertexBuffer[vertexId];
			float v18 = i * uvDist / v39 + a;
			float v19 = (i+1) * uvDist / v39 + a;
			do
			{
				++v14;
				//vertex += 144;
				
				float v26 = v38 * uvDist2 / v34 + c;
				float v27 = v14 * uvDist2 / v34 + c;
				
				TempVertexBuffer[vertexId].objVertex.x = (x + v18) * 500.0 - 3000.0;
				TempVertexBuffer[vertexId].objVertex.y = (y + v26) * 500.0 - 3000.0;
				TempVertexBuffer[vertexId].objVertex.z = height;
				TempVertexBuffer[vertexId].color = color;
				TempVertexBuffer[vertexId].v = v26 * UVScale;
				TempVertexBuffer[vertexId].u = v18 * UVScale;
				
				TempVertexBuffer[vertexId + 1].objVertex.x = (x + v18) * 500.0 - 3000.0;
				TempVertexBuffer[vertexId + 1].objVertex.y = (y + v27) * 500.0 - 3000.0;
				TempVertexBuffer[vertexId + 1].objVertex.z = height;
				TempVertexBuffer[vertexId + 1].color = color;
				TempVertexBuffer[vertexId + 1].u = v18 * UVScale;
				TempVertexBuffer[vertexId + 1].v = v27 * UVScale;

				TempVertexBuffer[vertexId + 2].u = v19 * UVScale;
				TempVertexBuffer[vertexId + 2].v = v26 * UVScale;
				TempVertexBuffer[vertexId + 2].objVertex.x = (x + v19) * 500.0 - 3000.0;
				TempVertexBuffer[vertexId + 2].objVertex.y = (y + v26) * 500.0 - 3000.0;
				TempVertexBuffer[vertexId + 2].objVertex.z = height;
				TempVertexBuffer[vertexId + 2].color	   = color;

				TempVertexBuffer[vertexId + 3].u = v19 * UVScale;
				TempVertexBuffer[vertexId + 3].v = v27 * UVScale;
				TempVertexBuffer[vertexId + 3].objVertex.x = (x + v19) * 500.0 - 3000.0;
				TempVertexBuffer[vertexId + 3].objVertex.y = (y + v27) * 500.0 - 3000.0;
				TempVertexBuffer[vertexId + 3].objVertex.z = height;
				TempVertexBuffer[vertexId + 3].color = color;

				TempBufferRenderIndexList[indexId+0] = vertexId + 2;
				TempBufferRenderIndexList[indexId+1] = vertexId + 1;
				TempBufferRenderIndexList[indexId+2] = vertexId + 0;
				TempBufferRenderIndexList[indexId+3] = vertexId + 1;
				TempBufferRenderIndexList[indexId+4] = vertexId + 2;
				TempBufferRenderIndexList[indexId+5] = vertexId + 3;
				v38 = v14;
				v8 = v34;
				indexId += 6;
				vertexId += 4;
			} while (v14 < v34);
		}
		++i;
	} while (i < v39);
	TempBufferIndicesStored = indexId;
	TempBufferVerticesStored = vertexId;
}

void CWaterLevel::RenderSeaBedSegment(int x, int y, float xOffset0, float xOffset1, float yOffset0, float yOffset1)
{
	int vertexId = TempBufferVerticesStored;
	int indexId = TempBufferIndicesStored;
	float UVScale = 32.0;
	UINT color = 0xFF505050;
	float height = -70.0f;
	TempVertexBuffer[vertexId].u = xOffset0 * UVScale;
	TempVertexBuffer[vertexId].v = yOffset0 * UVScale;
	TempVertexBuffer[vertexId].objVertex.x = (x + xOffset0) * 500.0 - 3000.0;
	TempVertexBuffer[vertexId].objVertex.y = (y + yOffset0) * 500.0 - 3000.0;
	TempVertexBuffer[vertexId].objVertex.z = height;
	TempVertexBuffer[vertexId].objNormal.z = 1.0;
	TempVertexBuffer[vertexId].color = color;

	TempVertexBuffer[vertexId + 1].u = xOffset0 * UVScale;
	TempVertexBuffer[vertexId + 1].v = yOffset1 * UVScale;
	TempVertexBuffer[vertexId + 1].objVertex.x = (x + xOffset0) * 500.0 - 3000.0;
	TempVertexBuffer[vertexId + 1].objVertex.y = (y + yOffset1) * 500.0 - 3000.0;
	TempVertexBuffer[vertexId + 1].objVertex.z = height;
	TempVertexBuffer[vertexId + 1].objNormal.z = 1.0;
	TempVertexBuffer[vertexId + 1].color = color;

	TempVertexBuffer[vertexId + 2].u = xOffset1 * UVScale;
	TempVertexBuffer[vertexId + 2].v = yOffset0 * UVScale;
	TempVertexBuffer[vertexId + 2].objVertex.x = (x + xOffset1) * 500.0 - 3000.0;
	TempVertexBuffer[vertexId + 2].objVertex.y = (y + yOffset0) * 500.0 - 3000.0;
	TempVertexBuffer[vertexId + 2].objVertex.z = height;
	TempVertexBuffer[vertexId + 2].objNormal.z = 1.0;
	TempVertexBuffer[vertexId + 2].color = color;

	TempVertexBuffer[vertexId + 3].u = xOffset1 * UVScale;
	TempVertexBuffer[vertexId + 3].v = yOffset1 * UVScale;
	TempVertexBuffer[vertexId + 3].objVertex.x = (x + xOffset1) * 500.0 - 3000.0;
	TempVertexBuffer[vertexId + 3].objVertex.y = (y + yOffset1) * 500.0 - 3000.0;
	TempVertexBuffer[vertexId + 3].objVertex.z = height;
	TempVertexBuffer[vertexId + 3].objNormal.z = 1.0;
	TempVertexBuffer[vertexId + 3].color = color;

	TempBufferRenderIndexList[indexId]		= vertexId + 2;
	TempBufferRenderIndexList[indexId + 1]	= vertexId + 1;
	TempBufferRenderIndexList[indexId + 2]	= vertexId + 0;
	TempBufferRenderIndexList[indexId + 3]	= vertexId + 1;
	TempBufferRenderIndexList[indexId + 4]	= vertexId + 2;
	TempBufferRenderIndexList[indexId + 5]	= vertexId + 3;

	TempBufferVerticesStored += 4;
	TempBufferIndicesStored += 6;
}

void CWaterLevel::RenderSeaBed()
{
	TempBufferVerticesStored = 0;
	TempBufferIndicesStored = 0;

	for (int i = 0; i < m_NumBlocksOutsideWorldToBeRendered; ++i)
	{
		USHORT x = m_BlocksToBeRenderedOutsideWorldX[i];
		USHORT y = m_BlocksToBeRenderedOutsideWorldY[i];

		CVector camPos= TheCamera.GetPosition();

		float xDistToCam = camPos.x - ((x + 0.5) * 500.0 - 3000.0);
		float yDistToCam = camPos.y - ((y + 0.5) * 500.0 - 3000.0);

		int renderDetailed = 1;
		if (sqrt(yDistToCam * yDistToCam + xDistToCam * xDistToCam) >= 600.0f)
			renderDetailed = 0;

		int v43 = 0;
		float uvA = 0.0;
		float uvB = 1.0;
		float uvC = 0.0;
		float uvD = 1.0;
		if (y < 0 || x < 0 || y >= 12 || x >= 12)
		{
			if (renderDetailed)
				RenderDetailedSeaBedSegment(x, y, uvA, uvB, 0.0, 1.0);
			else
				RenderSeaBedSegment(x, y, uvA, uvB, 0.0, 1.0);
			continue;
		}
		int v8 = 0;
		v43 = 0;

		if (m_BlocksToBeRenderedOutsideWorldX[i])
		{
			if (x == 11) {
				uvA = 0.96;
				uvB = 1.0;
				v8 = 1;
			}
		}
		else
		{
			uvB = 0.04;
			v8 = 1;
		}
		if (m_BlocksToBeRenderedOutsideWorldY[i])
		{
			if (y == 11) {
				uvC = 0.96;
				uvD = 1.0;
				v43 = 1;
			}
		}
		else
		{
			uvD = 0.04;
			v43 = 1;
		}

		if (v8)
		{
			if (renderDetailed)
				RenderDetailedSeaBedSegment(x, y, uvA, uvB, 0.0, 1.0);
			else
				RenderSeaBedSegment(x, y, uvA, uvB, 0.0, 1.0);
		}
		if (v43)
		{
			if (renderDetailed)
				RenderDetailedSeaBedSegment(x, y, 0.0, 1.0, uvC, uvD);
			else
				RenderSeaBedSegment(x, y, 0.0, 1.0, uvC, uvD);
		}
	}
	if (TempBufferVerticesStored)
		g_pCustomSeabedPipe->RenderSeabed(TempVertexBuffer, TempBufferVerticesStored, TempBufferRenderIndexList, TempBufferIndicesStored - TempBufferIndicesStored % 3);
	TempBufferIndicesStored = 0;
	TempBufferVerticesStored = 0;
}

void CWaterLevel::RenderWaterTriangle(CWaterVertice a, CWaterVertice b, CWaterVertice c)
{
	CWaterLevel_RenderWaterTriangle(a.posX, a.posY, a.renPar, b.posX, b.posY, b.renPar, c.posX, c.posY, c.renPar);
}

void CWaterLevel::RenderAndEmptyRenderBuffer()
{
	if (TempBufferVerticesStored) {
		//GenerateNormals();
		if(g_pCustomWaterPipe)
			g_pCustomWaterPipe->RenderWater(TempVertexBuffer, TempBufferVerticesStored, TempBufferRenderIndexList, TempBufferIndicesStored);
	}
	TempBufferIndicesStored = 0;
	TempBufferVerticesStored = 0;
}

void CWaterLevel::GenerateNormals()
{
	for (int i = 0; i < TempBufferIndicesStored /3; i++)
	{
		RwV3d firstvec = {
			TempVertexBuffer[TempBufferRenderIndexList[i * 3 + 1]].objVertex.x - TempVertexBuffer[TempBufferRenderIndexList[i * 3 + 0]].objVertex.x,
			TempVertexBuffer[TempBufferRenderIndexList[i * 3 + 1]].objVertex.y - TempVertexBuffer[TempBufferRenderIndexList[i * 3 + 0]].objVertex.y,
			TempVertexBuffer[TempBufferRenderIndexList[i * 3 + 1]].objVertex.z - TempVertexBuffer[TempBufferRenderIndexList[i * 3 + 0]].objVertex.z
		};
		RwV3d secondvec = {
			TempVertexBuffer[TempBufferRenderIndexList[i * 3 + 0]].objVertex.x - TempVertexBuffer[TempBufferRenderIndexList[i * 3 + 2]].objVertex.x,
			TempVertexBuffer[TempBufferRenderIndexList[i * 3 + 0]].objVertex.y - TempVertexBuffer[TempBufferRenderIndexList[i * 3 + 2]].objVertex.y,
			TempVertexBuffer[TempBufferRenderIndexList[i * 3 + 0]].objVertex.z - TempVertexBuffer[TempBufferRenderIndexList[i * 3 + 2]].objVertex.z
		};
		RwV3d normal = {
			firstvec.y*secondvec.z - firstvec.z*secondvec.y,
			firstvec.z*secondvec.x - firstvec.x*secondvec.z,
			firstvec.x*secondvec.y - firstvec.y*secondvec.x
		};// (firstvec, secondvec);
		RwReal length = sqrt(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);
		normal = { normal.x / length,normal.y / length,normal.z / length };

		TempVertexBuffer[TempBufferRenderIndexList[i * 3 + 0]].objNormal = {
			TempVertexBuffer[TempBufferRenderIndexList[i * 3 + 0]].objNormal.x + normal.x,
			TempVertexBuffer[TempBufferRenderIndexList[i * 3 + 0]].objNormal.y + normal.y,
			TempVertexBuffer[TempBufferRenderIndexList[i * 3 + 0]].objNormal.z + normal.z
		};
		TempVertexBuffer[TempBufferRenderIndexList[i * 3 + 1]].objNormal = {
			TempVertexBuffer[TempBufferRenderIndexList[i * 3 + 1]].objNormal.x + normal.x,
			TempVertexBuffer[TempBufferRenderIndexList[i * 3 + 1]].objNormal.y + normal.y,
			TempVertexBuffer[TempBufferRenderIndexList[i * 3 + 1]].objNormal.z + normal.z
		};
		TempVertexBuffer[TempBufferRenderIndexList[i * 3 + 2]].objNormal = {
			TempVertexBuffer[TempBufferRenderIndexList[i * 3 + 2]].objNormal.x + normal.x,
			TempVertexBuffer[TempBufferRenderIndexList[i * 3 + 2]].objNormal.y + normal.y,
			TempVertexBuffer[TempBufferRenderIndexList[i * 3 + 2]].objNormal.z + normal.z
		};
	}
	/*for (size_t i = 0; i < TempBufferVerticesStored; i++)
	{
		RwReal length = sqrt(TempVertexBuffer[i].objNormal.x*TempVertexBuffer[i].objNormal.x + TempVertexBuffer[i].objNormal.y*TempVertexBuffer[i].objNormal.y + TempVertexBuffer[i].objNormal.z*TempVertexBuffer[i].objNormal.z);
		TempVertexBuffer[i].objNormal = { TempVertexBuffer[i].objNormal.x / length,TempVertexBuffer[i].objNormal.y / length,TempVertexBuffer[i].objNormal.z / length };
	}*/
}


void CWaterLevel::RenderFlatWaterRectangle_OneLayer(int x0, int x1, int y0, int y1, CRenPar a, CRenPar b, CRenPar c, CRenPar d, int e)
{
	int startIndex, startVertex;
	if (TempBufferIndicesStored >= 4090 || TempBufferVerticesStored >= 2044)
	{
		RenderAndEmptyRenderBuffer();
	}
	startIndex = TempBufferIndicesStored;
	startVertex = TempBufferVerticesStored;
	float tU = 0, tV = 0, v27, height;
	if (e == 0)
	{
		float tcU =/* x * 0.079999998 +*/ TextureShiftU;
		float tcV =/* z * 0.079999998 +*/ TextureShiftV;
		tU = tcU;//- floor(tcU);
		tV = tcV;//- floor(tcV);


		v27 = 0.079999998;
		//LOBYTE(a21) = dword_8D3808;
		height = 0.0;
	}
	else if (e == 1)
	{
		float tcU = /*x * 0.039999999 +*/ TextureShiftSecondU;
		float tcV = /*z * 0.039999999 +*/ TextureShiftSecondV;
		tU = tcU;//- floor(tcU);
		tV = tcV;//- floor(tcV);
		v27 = 0.039999999;
		//LOBYTE(a21) = dword_8D380C;
		height = 0.0;
	}
	else
	{
		//v13 = a21;
	}
	tU = tU - 7.0;
	//v28 = a4 - a3;
	if (y1 - y0 <= 0)
		tV = tV + 7.0;
	else
		tV = tV - 7.0;
	float diffuse, sunGlare;
	height = a.baseHeight;
	CalculateWavesForCoordinate(x0, y0, a.wave0Height, a.wave1Height, &height, &diffuse, &sunGlare, (CVector*)&TempVertexBuffer[startVertex].objNormal);
	TempVertexBuffer[startVertex].u = 0;
	TempVertexBuffer[startVertex].v = 0;
	TempVertexBuffer[startVertex].objVertex.x = x0;
	TempVertexBuffer[startVertex].objVertex.y = y0;
	TempVertexBuffer[startVertex].objVertex.z = height;
	//TempVertexBuffer[startVertex].objNormal.z = 1.0;
	TempVertexBuffer[startVertex].color = 0xFFFFFFFF;
	float tU2 = (x1 - x0) * v27 + tU;
	float tV2 = (y1 - y0) * v27 + tV;
	height = b.baseHeight;
	CalculateWavesForCoordinate(x1, y0, b.wave0Height, b.wave1Height, &height, &diffuse, &sunGlare, (CVector*)&TempVertexBuffer[startVertex+1].objNormal);
	TempVertexBuffer[startVertex + 1].u = 1;
	TempVertexBuffer[startVertex + 1].v = 0;
	TempVertexBuffer[startVertex + 1].objVertex.x = x1;
	TempVertexBuffer[startVertex + 1].objVertex.y = y0;
	TempVertexBuffer[startVertex + 1].objVertex.z = height;
	//TempVertexBuffer[startVertex + 1].objNormal.z = 1.0;
	TempVertexBuffer[startVertex + 1].color = 0xFFFFFFFF;

	height = c.baseHeight;
	CalculateWavesForCoordinate(x0, y1, c.wave0Height, c.wave1Height, &height, &diffuse, &sunGlare, (CVector*)&TempVertexBuffer[startVertex+2].objNormal);
	TempVertexBuffer[startVertex + 2].u = 0;
	TempVertexBuffer[startVertex + 2].v = 1;
	TempVertexBuffer[startVertex + 2].objVertex.x = x0;
	TempVertexBuffer[startVertex + 2].objVertex.y = y1;
	TempVertexBuffer[startVertex + 2].objVertex.z = height;
	//TempVertexBuffer[startVertex + 2].objNormal.z = 1.0;
	TempVertexBuffer[startVertex + 2].color = 0xFFFFFFFF;

	height = d.baseHeight;
	CalculateWavesForCoordinate(x1, y1, d.wave0Height, d.wave1Height, &height, &diffuse, &sunGlare, (CVector*)&TempVertexBuffer[startVertex+3].objNormal);
	TempVertexBuffer[startVertex + 3].u = 1;
	TempVertexBuffer[startVertex + 3].v = 1;
	TempVertexBuffer[startVertex + 3].objVertex.x = x1;
	TempVertexBuffer[startVertex + 3].objVertex.y = y1;
	TempVertexBuffer[startVertex + 3].objVertex.z = height;
	//TempVertexBuffer[startVertex + 3].objNormal.z = 1.0;
	TempVertexBuffer[startVertex + 3].color = 0xFFFFFFFF;

	TempBufferRenderIndexList[startIndex] = startVertex;
	TempBufferRenderIndexList[startIndex + 1] = startVertex + 1;
	TempBufferRenderIndexList[startIndex + 2] = startVertex + 2;
	TempBufferRenderIndexList[startIndex + 3] = startVertex + 3;
	//TempBufferRenderIndexList[startIndex + 4] = startVertex + 1;
	//TempBufferRenderIndexList[startIndex + 5] = startVertex + 2;
	TempBufferVerticesStored = startVertex + 4;
	TempBufferIndicesStored = startIndex + 4;//6;
}

void CWaterLevel::CalculateWavesForCoordinate(int x, int y, float a3, float a4, float *resHeight, float *diffuse, float *sunGlare, CVector *resNormal)
{
	float g_fWaterDiffuseIntensity = 0.65f;
	float g_fWaterAmbientIntensity = 0.27f;
	CVector lightDir(1, 1, 1);
	lightDir.Normalise();

	x = abs(x);
	y = abs(y);

	float waveIntensity = g_aFlowIntensity[x / 2 & 7] * g_aFlowIntensity[(y / 2 & 7) + 8] * CWeather::Wavyness;
	int timeDiff = CTimer::m_snTimeInMilliseconds - CWaterLevel::m_nWaterTimeOffset;
	float wave0Freq = (float)(timeDiff % 5000) / 2500.0f;
	float wave1Freq = (float)(timeDiff % 3500) / 1750.0f;
	float wave2Freq = (float)(timeDiff % 3000) / 1500.0f;

	int lookUpID0 = (int)((wave0Freq + (y + x) / 32.0f) * 128) % 127;
	int lookUpID1 = (int)((wave1Freq + y / 26.0f + x / 13.0f) * 128) % 127;
	int lookUpID2 = (int)((wave2Freq + y / 10.0f) * 128) % 127;

	float wave0Phi = (wave0Freq + (y + x) / 32.0f);
	float wave1Phi = (wave1Freq + y / 26.0f + x / 13.0f);
	float wave2Phi = (wave2Freq + y / 10.0f);

	
	float sin0 = sin(wave0Phi*M_PI);//CMaths::ms_SinTable[lookUpID0 + 1];
	float cos0 = cos(wave0Phi*M_PI);//CMaths::ms_SinTable[(lookUpID0 + 64) + 1];
	float sin1 = sin(wave1Phi*M_PI);//CMaths::ms_SinTable[lookUpID1 + 1];
	float cos1 = cos(wave1Phi*M_PI);//CMaths::ms_SinTable[(lookUpID1 + 64) + 1];
	float sin2 = sin(wave2Phi*M_PI);//CMaths::ms_SinTable[lookUpID2 + 1];
	float cos2 = cos(wave2Phi*M_PI);//CMaths::ms_SinTable[(lookUpID2 + 64) + 1];

	*resHeight = sin0 * 2.0 * waveIntensity * a3 + *resHeight;
	float wave0 = -(cos0 * 2.0 * waveIntensity * a3 * M_PI / 32.0f);
	resNormal->x = wave0;
	resNormal->y = wave0;
	resNormal->z = 1.0f;

	*resHeight = sin1 * waveIntensity * a4 + *resHeight;
	float wave1 = cos1 * waveIntensity * a4 * M_PI / 13.0f;
	resNormal->x += wave1;
	resNormal->y += wave1;

	*resHeight = sin2 * 0.5f * waveIntensity * a4 + *resHeight;
	resNormal->x += waveIntensity * (cos2 * 0.5f) * a4 * M_PI / 10.0f;

	resNormal->Normalise();


	float cosA = resNormal->x*lightDir.x + resNormal->y*lightDir.y + resNormal->z*lightDir.z;

	float diffuseCoeff = max(cosA, 0.0f);
	*diffuse = diffuseCoeff * g_fWaterDiffuseIntensity + g_fWaterAmbientIntensity;

	*sunGlare = cosA;
	float sunGlareCoeff = cosA * 8.0 - 5.0;

	if (sunGlareCoeff >= 0.0)
	{
		sunGlareCoeff = min(sunGlareCoeff, 0.99f);
		*sunGlare = sunGlareCoeff * CWeather::SunGlare;
	}
	else
	{
		*sunGlare = 0.0;
	}
}

void CWaterLevel::RenderDetailedWaterRectangle_OneLayer(int x0, int x1, int y0, int y1, CRenPar a, CRenPar b, CRenPar c, CRenPar d, int layer)
{
	
	int startIndex, startVertex;
	if (TempBufferIndicesStored >= 4090 || TempBufferVerticesStored >= 2044)
	{
		RenderAndEmptyRenderBuffer();
	}
	float tU = 0, tV = 0, layerTCScale, height;
	if (layer == 0)
	{
		float tcU =/* x * 0.079999998 +*/ TextureShiftU;
		float tcV =/* z * 0.079999998 +*/ TextureShiftV;
		tU = tcU;//- floor(tcU);
		tV = tcV;//- floor(tcV);
		layerTCScale = 1.0f/(25.0f*2.0f);
		//LOBYTE(a21) = dword_8D3808;
		height = 0.0;
	}
	else if (layer == 1)
	{
		float tcU = /*x * 0.039999999 +*/ TextureShiftSecondU;
		float tcV = /*z * 0.039999999 +*/ TextureShiftSecondV;
		tU = tcU;//- floor(tcU);
		tV = tcV;//- floor(tcV);
		layerTCScale = 1.0f/25.0f;
		//LOBYTE(a21) = dword_8D380C;
		height = 0.0;
	}
	tU = tU - 7.0;
	if (y1 - y0 <= 0)
		tV = tV + 7.0;
	else
		tV = tV - 7.0;
	float tU2 = (x1 - x0) * layerTCScale + tU;
	float tV2 = (y1 - y0) * layerTCScale + tV;
	int tessFactor = 16;
	float xSize = (x1 - x0)/ (float)tessFactor;
	float ySize = (y1 - y0)/ (float)tessFactor;
	float wave0avgHeight = (a.wave0Height + b.wave0Height + c.wave0Height + d.wave0Height) / 4.0f;
	float wave1avgHeight = (a.wave1Height + b.wave1Height + c.wave1Height + d.wave1Height) / 4.0f;
	for (size_t x_offset = 0; x_offset < tessFactor; x_offset++)
	{
		for (size_t y_offset = 0; y_offset < tessFactor; y_offset++)
		{
			startIndex = TempBufferIndicesStored;
			startVertex = TempBufferVerticesStored;
			float diffuse;
			float sunGlare;
			int x = x0 + xSize * x_offset;
			int y = y0 + ySize * y_offset;
			height = a.baseHeight;
			CalculateWavesForCoordinate(x, y, wave0avgHeight, wave1avgHeight, &height, &diffuse, &sunGlare, (CVector*)&TempVertexBuffer[startVertex].objNormal);
			TempVertexBuffer[startVertex].u = x_offset / (float)tessFactor;
			TempVertexBuffer[startVertex].v = y_offset / (float)tessFactor;
			TempVertexBuffer[startVertex].objVertex.x = x;
			TempVertexBuffer[startVertex].objVertex.y = y;
			TempVertexBuffer[startVertex].objVertex.z = height;
			TempVertexBuffer[startVertex].color = 0xFFFFFFFF;

			x = x0 + xSize * (x_offset + 1);
			y = y0 + ySize * y_offset;
			height = b.baseHeight;
			CalculateWavesForCoordinate(x, y, wave0avgHeight, wave1avgHeight, &height, &diffuse, &sunGlare, (CVector*)&TempVertexBuffer[startVertex+1].objNormal);
			TempVertexBuffer[startVertex + 1].u =(x_offset+1) / (float)tessFactor;
			TempVertexBuffer[startVertex + 1].v = y_offset / (float)tessFactor;
			TempVertexBuffer[startVertex + 1].objVertex.x = x;
			TempVertexBuffer[startVertex + 1].objVertex.y = y;
			TempVertexBuffer[startVertex + 1].objVertex.z = height;
			TempVertexBuffer[startVertex + 1].color = 0xFFFFFFFF;

			x = x0 + xSize * x_offset;
			y = y0 + ySize * (y_offset + 1);
			height = c.baseHeight;
			CalculateWavesForCoordinate(x, y, wave0avgHeight, wave1avgHeight, &height, &diffuse, &sunGlare, (CVector*)&TempVertexBuffer[startVertex+2].objNormal);
			TempVertexBuffer[startVertex + 2].u = x_offset / (float)tessFactor;
			TempVertexBuffer[startVertex + 2].v = (y_offset+1) / (float)tessFactor;
			TempVertexBuffer[startVertex + 2].objVertex.x = x;
			TempVertexBuffer[startVertex + 2].objVertex.y = y;
			TempVertexBuffer[startVertex + 2].objVertex.z = height;
			TempVertexBuffer[startVertex + 2].color = 0xFFFFFFFF;

			x = x0 + xSize * (x_offset + 1);
			y = y0 + ySize * (y_offset + 1);
			height = d.baseHeight;
			CalculateWavesForCoordinate(x, y, wave0avgHeight, wave1avgHeight, &height, &diffuse, &sunGlare, (CVector*)&TempVertexBuffer[startVertex + 3].objNormal);
			TempVertexBuffer[startVertex + 3].u = (x_offset+1) / (float)tessFactor;
			TempVertexBuffer[startVertex + 3].v = (y_offset+1) / (float)tessFactor;
			TempVertexBuffer[startVertex + 3].objVertex.x = x;
			TempVertexBuffer[startVertex + 3].objVertex.y = y;
			TempVertexBuffer[startVertex + 3].objVertex.z = height;
			TempVertexBuffer[startVertex + 3].color = 0xFFFFFFFF;

			TempBufferRenderIndexList[startIndex] = startVertex;
			TempBufferRenderIndexList[startIndex + 1] = startVertex + 1;
			TempBufferRenderIndexList[startIndex + 2] = startVertex + 2;
			TempBufferRenderIndexList[startIndex + 3] = startVertex + 3;
			//TempBufferRenderIndexList[startIndex + 4] = startVertex + 1;
			//TempBufferRenderIndexList[startIndex + 5] = startVertex + 2;
			TempBufferVerticesStored = startVertex + 4;
			TempBufferIndicesStored = startIndex + 4;//6;
		}
	}
}
