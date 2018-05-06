// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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

SHORT* CWaterLevel::m_BlocksToBeRenderedOutsideWorldX = (SHORT*)0xC21560;
SHORT* CWaterLevel::m_BlocksToBeRenderedOutsideWorldY = (SHORT*)0xC214D0;

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

	CurrentTextureShiftU += waterSpeedX / WATER_UPDATE_FRAMERATE;
	CurrentTextureShiftV += waterSpeedY / WATER_UPDATE_FRAMERATE;
	CurrentTextureShiftSecondU += waterSpeedX/(2*WATER_UPDATE_FRAMERATE);
	CurrentTextureShiftSecondU += waterSpeedY/(2*WATER_UPDATE_FRAMERATE);
	
	if (CurrentTextureShiftU > 1.0)
		CurrentTextureShiftU -= 1.0;
	if (CurrentTextureShiftV > 1.0)
		CurrentTextureShiftV -= 1.0;
	if (CurrentTextureShiftSecondU > 1.0)
		CurrentTextureShiftSecondU -= 1.0;
	if (CurrentTextureShiftSecondV > 1.0)
		CurrentTextureShiftSecondV -= 1.0;

	// R* MAGIC
	float magic0 = (CTimer::m_snTimeInMilliseconds & 0xFFF) * 0.00015339808f;
	float magic1 = (CTimer::m_snTimeInMilliseconds & 0x1FFF) * 0.000076699042f;
	float magic2 = rand() * 0.000030518509f / WATER_UPDATE_FRAMERATE;

	TextureShiftU = sin(magic0) * CWeather::Wavyness / WATER_UPDATE_FRAMERATE * 2.0f + CurrentTextureShiftU;
	TextureShiftV = cos(magic0) * CWeather::Wavyness / WATER_UPDATE_FRAMERATE * 2.0f + CurrentTextureShiftV;
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
		for(UINT i=0;i<m_nNumOfWaterQuads;i++){
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
				m_aQuads[i].m_flags &= ((byte)~1);
			}
		}
	}
	// water rectangles
	for (UINT k = 0; k < m_NumBlocksOutsideWorldToBeRendered; ++k)
	{

		if (m_BlocksToBeRenderedOutsideWorldX[k] >= 0 && m_BlocksToBeRenderedOutsideWorldX[k] < 12) {
			if(m_BlocksToBeRenderedOutsideWorldY[k] >= 0 && m_BlocksToBeRenderedOutsideWorldY[k] < 12)
				continue;
		}

		SHORT x = m_BlocksToBeRenderedOutsideWorldX[k];
		SHORT y = m_BlocksToBeRenderedOutsideWorldY[k];

		RenderWaterRectangle(
			(x * 500 - 3000), ((x + 1) * 500 - 3000), (y * 500 - 3000), ((y + 1) * 500 - 3000),
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

	CameraRangeMinX = 2 * (int)floorf((camPos.x - DETAILEDWATERDIST) * 0.5f);
	CameraRangeMaxX = 2 * (int)ceilf((DETAILEDWATERDIST + camPos.x) * 0.5f);
	CameraRangeMinY = 2 * (int)floorf((camPos.y - DETAILEDWATERDIST) * 0.5f);
	CameraRangeMaxY = 2 * (int)ceilf((DETAILEDWATERDIST + camPos.y) * 0.5f);
}

void CWaterLevel::RenderBoatWakes()
{
	CWaterLevel_RenderBoatWakes();
}



CRenPar * __cdecl CRenPar_Lerp(CRenPar *a1, const CRenPar &a, const CRenPar &b, float t)
{
	float invT = 1.0f - t;
	
	a1->baseHeight = a.baseHeight * invT + b.baseHeight * t;
	a1->wave0Height = a.wave0Height * invT + b.wave0Height * t;
	a1->wave1Height = a.wave1Height * invT + b.wave1Height * t;
	a1->flowX= a1->flowY = 0;
	a1->pad = 0;
	return a1;
}

void CWaterLevel::RenderWaterRectangle(int x0, int x1, int y0, int y1, const CRenPar &a, const CRenPar &b, const CRenPar &c, const CRenPar &d)
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
		/*int xSize = x1 - x0;
		int halfYSize = (y1 - y0) / 2;
		int halfXSize = xSize / 2;
		int xToYHalfRatio = xSize * halfYSize;
		int a23 = (halfXSize + 1) * (halfYSize + 1);
		RenderHighDetailWaterRectangle_OneLayer(x0, x1, y0, y1, a, b, c, d, 0, xToYHalfRatio, a23, halfXSize, halfYSize);*/
		//RenderDetailedWaterRectangle_OneLayer(x0, x1, y0, y1, a, b, c, d, 1);
	}
}

void CWaterLevel::RenderFlatWaterRectangle(int x, int y, int z, int w, const CRenPar & a, const CRenPar & b, const CRenPar & c, const CRenPar & d)
{
	RenderFlatWaterRectangle_OneLayer(x, y, z, w, a, b, c, d, 0);
	//RenderFlatWaterRectangle_OneLayer(x, y, z, w, a, b, c, d, 1);
	//CWaterLevel_RenderFlatWaterRectangle(x, y, z, w, a, b, c, d);
}

void CWaterLevel::SplitWaterRectangleAlongYLine(int m, int x0, int x1, int y0, int y1, const CRenPar & a, const CRenPar & b, const CRenPar & c, const CRenPar & d)
{
	CRenPar b_d, a_c;
	float blendFactor = (float)(m - y0) / (y1 - y0);

	// First split part
	CRenPar_Lerp(&b_d, b, d, blendFactor);
	CRenPar_Lerp(&a_c, a, c, blendFactor);

	RenderWaterRectangle(x0, x1, y0, m, a, b, a_c, b_d);

	// Second split part
	CRenPar_Lerp(&b_d, b, d, blendFactor);
	CRenPar_Lerp(&a_c, a, c, blendFactor);

	RenderWaterRectangle(x0, x1, m, y1, a_c, b_d, c, d);
}

void CWaterLevel::SplitWaterRectangleAlongXLine(int m, int x0, int x1, int y0, int y1, const CRenPar & a, const CRenPar & b, const CRenPar & c, const CRenPar & d)
{
	CRenPar c_d, a_b;
	float blendFactor = (float)(m - x0) / (x1 - x0);

	// First split part
	CRenPar_Lerp(&c_d, c, d, blendFactor);
	CRenPar_Lerp(&a_b, a, b, blendFactor);

	RenderWaterRectangle(x0, m, y0, y1, a, a_b, c, c_d);

	// Second split part
	CRenPar_Lerp(&c_d, c, d, blendFactor);
	CRenPar_Lerp(&a_b, a, b, blendFactor);

	RenderWaterRectangle(m, x1, y0, y1, a_b, b, c_d, d);
}
float BilinearInterp(float a, float b, float c, float d, float x, float y, float x1, float x2, float y1, float y2) {
	float xl1 = (x2 - x) / (x2 - x1);
	float xl2 = (x - x1) / (x2 - x1);
	float xLerp1 = a * xl1 + b * xl2;
	float xLerp2 = c * xl1 + d * xl2;
	float yl1 = (y2 - y) / (y2 - y1);
	float yl2 = (y - y1) / (y2 - y1);

	return yl1 * xLerp1 + yl2 * xLerp2;
}

void CWaterLevel::RenderDetailedSeaBedSegment(int x, int y, float a, float b, float c, float d)
{
	float height = -70.0f;
	float UVScale = 32.0;
	UINT  color = 0xFF505050;

	float uvDist = b - a;
	float v7 = (uvDist * 4.0f);
	int v8 = 1;
	float v39 = 1;
	if (v7 >= 1)
		v39 = v7;

	float uvDist2 = d - c;
	float v10 = (uvDist2 * 4.0f);
	if (v10 >= 1)
		v8 = (int)v10;

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
				
				TempVertexBuffer[vertexId].objVertex.x = (x + v18) * 500.0f - 3000.0f;
				TempVertexBuffer[vertexId].objVertex.y = (y + v26) * 500.0f - 3000.0f;
				TempVertexBuffer[vertexId].objVertex.z = height;
				TempVertexBuffer[vertexId].color = color;
				TempVertexBuffer[vertexId].v = v26 * UVScale;
				TempVertexBuffer[vertexId].u = v18 * UVScale;
				
				TempVertexBuffer[vertexId + 1].objVertex.x = (x + v18) * 500.0f - 3000.0f;
				TempVertexBuffer[vertexId + 1].objVertex.y = (y + v27) * 500.0f - 3000.0f;
				TempVertexBuffer[vertexId + 1].objVertex.z = height;
				TempVertexBuffer[vertexId + 1].color = color;
				TempVertexBuffer[vertexId + 1].u = v18 * UVScale;
				TempVertexBuffer[vertexId + 1].v = v27 * UVScale;

				TempVertexBuffer[vertexId + 2].u = v19 * UVScale;
				TempVertexBuffer[vertexId + 2].v = v26 * UVScale;
				TempVertexBuffer[vertexId + 2].objVertex.x = (x + v19) * 500.0f - 3000.0f;
				TempVertexBuffer[vertexId + 2].objVertex.y = (y + v26) * 500.0f - 3000.0f;
				TempVertexBuffer[vertexId + 2].objVertex.z = height;
				TempVertexBuffer[vertexId + 2].color	   = color;

				TempVertexBuffer[vertexId + 3].u = v19 * UVScale;
				TempVertexBuffer[vertexId + 3].v = v27 * UVScale;
				TempVertexBuffer[vertexId + 3].objVertex.x = (x + v19) * 500.0f - 3000.0f;
				TempVertexBuffer[vertexId + 3].objVertex.y = (y + v27) * 500.0f - 3000.0f;
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
	TempVertexBuffer[vertexId].objVertex.x = (x + xOffset0) * 500.0f - 3000.0f;
	TempVertexBuffer[vertexId].objVertex.y = (y + yOffset0) * 500.0f - 3000.0f;
	TempVertexBuffer[vertexId].objVertex.z = height;
	TempVertexBuffer[vertexId].objNormal.z = 1.0;
	TempVertexBuffer[vertexId].color = color;

	TempVertexBuffer[vertexId + 1].u = xOffset0 * UVScale;
	TempVertexBuffer[vertexId + 1].v = yOffset1 * UVScale;
	TempVertexBuffer[vertexId + 1].objVertex.x = (x + xOffset0) * 500.0f - 3000.0f;
	TempVertexBuffer[vertexId + 1].objVertex.y = (y + yOffset1) * 500.0f - 3000.0f;
	TempVertexBuffer[vertexId + 1].objVertex.z = height;
	TempVertexBuffer[vertexId + 1].objNormal.z = 1.0;
	TempVertexBuffer[vertexId + 1].color = color;

	TempVertexBuffer[vertexId + 2].u = xOffset1 * UVScale;
	TempVertexBuffer[vertexId + 2].v = yOffset0 * UVScale;
	TempVertexBuffer[vertexId + 2].objVertex.x = (x + xOffset1) * 500.0f - 3000.0f;
	TempVertexBuffer[vertexId + 2].objVertex.y = (y + yOffset0) * 500.0f - 3000.0f;
	TempVertexBuffer[vertexId + 2].objVertex.z = height;
	TempVertexBuffer[vertexId + 2].objNormal.z = 1.0;
	TempVertexBuffer[vertexId + 2].color = color;

	TempVertexBuffer[vertexId + 3].u = xOffset1 * UVScale;
	TempVertexBuffer[vertexId + 3].v = yOffset1 * UVScale;
	TempVertexBuffer[vertexId + 3].objVertex.x = (x + xOffset1) * 500.0f - 3000.0f;
	TempVertexBuffer[vertexId + 3].objVertex.y = (y + yOffset1) * 500.0f - 3000.0f;
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

	for (UINT i = 0; i < m_NumBlocksOutsideWorldToBeRendered; ++i)
	{
		SHORT x = m_BlocksToBeRenderedOutsideWorldX[i];
		SHORT y = m_BlocksToBeRenderedOutsideWorldY[i];

		CVector camPos= TheCamera.GetPosition();

		float xDistToCam = camPos.x - ((x + 0.5f) * 500.0f - 3000.0f);
		float yDistToCam = camPos.y - ((y + 0.5f) * 500.0f - 3000.0f);

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
				uvA = 0.96f;
				uvB = 1.0f;
				v8 = 1;
			}
		}
		else
		{
			uvB = 0.04f;
			v8 = 1;
		}
		if (m_BlocksToBeRenderedOutsideWorldY[i])
		{
			if (y == 11) {
				uvC = 0.96f;
				uvD = 1.0f;
				v43 = 1;
			}
		}
		else
		{
			uvD = 0.04f;
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

void CWaterLevel::RenderWaterTriangle(const CWaterVertice &a, const CWaterVertice & b, const CWaterVertice & c)
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
	for (UINT i = 0; i < TempBufferIndicesStored /3; i++)
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


void CWaterLevel::RenderFlatWaterRectangle_OneLayer(int x0, int x1, int y0, int y1, const CRenPar & a, const CRenPar & b, const CRenPar & c, const CRenPar & d, int e)
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


		v27 = 0.08f;
		//LOBYTE(a21) = dword_8D3808;
	}
	else if (e == 1)
	{
		float tcU = /*x * 0.039999999 +*/ TextureShiftSecondU;
		float tcV = /*z * 0.039999999 +*/ TextureShiftSecondV;
		tU = tcU;//- floor(tcU);
		tV = tcV;//- floor(tcV);
		v27 = 0.04f;
		//LOBYTE(a21) = dword_8D380C;
	}
	else
	{
		v27 = 0.08f;
		//v13 = a21;
	}
	tU = tU - 7.0f;
	//v28 = a4 - a3;
	if (y1 - y0 <= 0)
		tV = tV + 7.0f;
	else
		tV = tV - 7.0f;
	float diffuse, sunGlare;
	height = a.baseHeight;
	CalculateWavesForCoordinate(x0, y0, a.wave0Height, a.wave1Height, &height, &diffuse, &sunGlare, (CVector*)&TempVertexBuffer[startVertex].objNormal);
	TempVertexBuffer[startVertex].u = 0;
	TempVertexBuffer[startVertex].v = 0;
	TempVertexBuffer[startVertex].objVertex.x = (float)x0;
	TempVertexBuffer[startVertex].objVertex.y = (float)y0;
	TempVertexBuffer[startVertex].objVertex.z = height;
	//TempVertexBuffer[startVertex].objNormal.z = 1.0;
	TempVertexBuffer[startVertex].color = 0xFFFFFFFF;
	float tU2 = (x1 - x0) * v27 + tU;
	float tV2 = (y1 - y0) * v27 + tV;
	height = b.baseHeight;
	CalculateWavesForCoordinate(x1, y0, b.wave0Height, b.wave1Height, &height, &diffuse, &sunGlare, (CVector*)&TempVertexBuffer[startVertex+1].objNormal);
	TempVertexBuffer[startVertex + 1].u = 1;
	TempVertexBuffer[startVertex + 1].v = 0;
	TempVertexBuffer[startVertex + 1].objVertex.x = (float)x1;
	TempVertexBuffer[startVertex + 1].objVertex.y = (float)y0;
	TempVertexBuffer[startVertex + 1].objVertex.z = height;
	//TempVertexBuffer[startVertex + 1].objNormal.z = 1.0;
	TempVertexBuffer[startVertex + 1].color = 0xFFFFFFFF;

	height = c.baseHeight;
	CalculateWavesForCoordinate(x0, y1, c.wave0Height, c.wave1Height, &height, &diffuse, &sunGlare, (CVector*)&TempVertexBuffer[startVertex+2].objNormal);
	TempVertexBuffer[startVertex + 2].u = 0;
	TempVertexBuffer[startVertex + 2].v = 1;
	TempVertexBuffer[startVertex + 2].objVertex.x = (float)x0;
	TempVertexBuffer[startVertex + 2].objVertex.y = (float)y1;
	TempVertexBuffer[startVertex + 2].objVertex.z = height;
	//TempVertexBuffer[startVertex + 2].objNormal.z = 1.0;
	TempVertexBuffer[startVertex + 2].color = 0xFFFFFFFF;

	height = d.baseHeight;
	CalculateWavesForCoordinate(x1, y1, d.wave0Height, d.wave1Height, &height, &diffuse, &sunGlare, (CVector*)&TempVertexBuffer[startVertex+3].objNormal);
	TempVertexBuffer[startVertex + 3].u = 1;
	TempVertexBuffer[startVertex + 3].v = 1;
	TempVertexBuffer[startVertex + 3].objVertex.x = (float)x1;
	TempVertexBuffer[startVertex + 3].objVertex.y = (float)y1;
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

	
	float sin0 = (float)sin(wave0Phi*M_PI);//CMaths::ms_SinTable[lookUpID0 + 1];
	float cos0 = (float)cos(wave0Phi*M_PI);//CMaths::ms_SinTable[(lookUpID0 + 64) + 1];
	float sin1 = (float)sin(wave1Phi*M_PI);//CMaths::ms_SinTable[lookUpID1 + 1];
	float cos1 = (float)cos(wave1Phi*M_PI);//CMaths::ms_SinTable[(lookUpID1 + 64) + 1];
	float sin2 = (float)sin(wave2Phi*M_PI);//CMaths::ms_SinTable[lookUpID2 + 1];
	float cos2 = (float)cos(wave2Phi*M_PI);//CMaths::ms_SinTable[(lookUpID2 + 64) + 1];

	*resHeight = sin0 * 2.0f * waveIntensity * a3 + *resHeight;
	float wave0 = -(cos0 * 2.0f * waveIntensity * a3 * (float)M_PI / 32.0f);
	resNormal->x = wave0;
	resNormal->y = wave0;
	resNormal->z = 1.0f;

	*resHeight = sin1 * waveIntensity * a4 + *resHeight;
	float wave1 = cos1 * waveIntensity * a4 * (float)M_PI / 13.0f;
	resNormal->x += wave1;
	resNormal->y += wave1;

	*resHeight = sin2 * 0.5f * waveIntensity * a4 + *resHeight;
	resNormal->x += waveIntensity * (cos2 * 0.5f) * a4 * (float)M_PI / 10.0f;

	resNormal->Normalise();


	float cosA = resNormal->x*lightDir.x + resNormal->y*lightDir.y + resNormal->z*lightDir.z;

	float diffuseCoeff = max(cosA, 0.0f);
	*diffuse = diffuseCoeff * g_fWaterDiffuseIntensity + g_fWaterAmbientIntensity;

	*sunGlare = cosA;
	float sunGlareCoeff = cosA * 8.0f - 5.0f;

	if (sunGlareCoeff >= 0.0f)
	{
		sunGlareCoeff = min(sunGlareCoeff, 0.99f);
		*sunGlare = sunGlareCoeff * CWeather::SunGlare;
	}
	else
	{
		*sunGlare = 0.0f;
	}
}


void CWaterLevel::RenderDetailedWaterRectangle_OneLayer(int x0, int x1, int y0, int y1, const CRenPar & a, const CRenPar & b, const CRenPar & c, const CRenPar & d, int layer)
{
	
	int startIndex, startVertex;
	if (TempBufferIndicesStored >= 4090 || TempBufferVerticesStored >= 2044)
	{
		RenderAndEmptyRenderBuffer();
	}
	float tU = 0, tV = 0, layerTCScale, height;
	float tcShiftU, tcShiftV;
	height = 0.0;
	if (layer == 0)
	{
		tcShiftU =TextureShiftU;
		tcShiftV =TextureShiftV;
		layerTCScale = 1.0f / (WATER_UPDATE_FRAMERATE*2.0f);
		
	}
	else
	{
		tcShiftU = TextureShiftSecondU;
		tcShiftV = TextureShiftSecondV;
		layerTCScale = 1.0f / WATER_UPDATE_FRAMERATE;
	}
	float uBaseOffset = tcShiftU + x0 * layerTCScale;
	float vBaseOffset = tcShiftV + y0 * layerTCScale;
	// clamp offsets
	//uBaseOffset = uBaseOffset - floorf(uBaseOffset);
	//vBaseOffset = vBaseOffset - floorf(vBaseOffset);

	UINT tessFactor = 16;
	float xSize = (x1 - x0)/ (float)tessFactor;
	float ySize = (y1 - y0)/ (float)tessFactor;
	float wave0avgHeight = (a.wave0Height + b.wave0Height + c.wave0Height + d.wave0Height) / 4.0f;
	float wave1avgHeight = (a.wave1Height + b.wave1Height + c.wave1Height + d.wave1Height) / 4.0f;
	auto camPos = TheCamera.GetPosition();
	startVertex = TempBufferVerticesStored;
	startIndex = TempBufferIndicesStored;
	int vertCount = 0;
	for (uint x_offset = 0; x_offset <= tessFactor; x_offset++)
	{
		for (uint y_offset = 0; y_offset <= tessFactor; y_offset++)
		{
			int currVertID = startVertex + x_offset * (tessFactor+1) + y_offset;
			float diffuse;
			float sunGlare;
			int x = x0 + (int)(xSize * x_offset);
			int y = y0 + (int)(ySize * y_offset);
			float detailScale = sqrtf(((camPos.y - y) * (camPos.y - y)) + ((camPos.x - x) * (camPos.x - x))) / DETAILEDWATERDIST;

			float waterHeightScale = 1.0f;
			if (detailScale <= 1.0f)
			{
				if (detailScale > 0.75f)
					waterHeightScale = (1.0f - detailScale) * 4.0f;
			}
			else
				waterHeightScale = 0.0f;
			
			height = BilinearInterp(a.baseHeight, b.baseHeight, c.baseHeight, d.baseHeight, (float)x, (float)y, (float)x0, (float)x1, (float)y0, (float)y1);//wave0avgHeight;
			float wave0Height = BilinearInterp(a.wave0Height, b.wave0Height, c.wave0Height, d.wave0Height, (float)x, (float)y, (float)x0, (float)x1, (float)y0, (float)y1)*waterHeightScale;
			float wave1Height = BilinearInterp(a.wave1Height, b.wave1Height, c.wave1Height, d.wave1Height, (float)x, (float)y, (float)x0, (float)x1, (float)y0, (float)y1)*waterHeightScale;
			CalculateWavesForCoordinate(x, y, wave0Height, wave1Height, &height, &diffuse, &sunGlare, (CVector*)&TempVertexBuffer[currVertID].objNormal);
			float u = x_offset * layerTCScale + uBaseOffset;
			float v = y_offset * layerTCScale + vBaseOffset;
			TempVertexBuffer[currVertID].u = u-floorf(u);
			TempVertexBuffer[currVertID].v = v-floorf(v);
			TempVertexBuffer[currVertID].objVertex.x = (float)x;
			TempVertexBuffer[currVertID].objVertex.y = (float)y;
			TempVertexBuffer[currVertID].objVertex.z = height;
			TempVertexBuffer[currVertID].color = 0xFFFFFFFF;
			vertCount++;
		}
	}
	TempBufferVerticesStored = startVertex + vertCount;
	for (uint x_offset = 0; x_offset < tessFactor; x_offset++)
	{
		for (uint y_offset = 0; y_offset < tessFactor; y_offset++)
		{
			startIndex = TempBufferIndicesStored;
			TempBufferRenderIndexList[startIndex]		= startVertex + x_offset* (tessFactor + 1) + y_offset;
			TempBufferRenderIndexList[startIndex + 1]	= startVertex + (x_offset + 1) * (tessFactor + 1) + y_offset;
			TempBufferRenderIndexList[startIndex + 2]	= startVertex + x_offset * (tessFactor + 1) + y_offset + 1;
			TempBufferRenderIndexList[startIndex + 3]	= startVertex + (x_offset + 1) * (tessFactor + 1) + y_offset + 1;
			//TempBufferRenderIndexList[startIndex + 4] = startVertex + 1;
			//TempBufferRenderIndexList[startIndex + 5] = startVertex + 2;
			TempBufferIndicesStored = startIndex+4;
			//6;
		}
	}
	//TempBufferIndicesStored = startIndex + 4 * (tessFactor - 1) * (tessFactor - 1);
}

void CWaterLevel::RenderHighDetailWaterRectangle_OneLayer(int x0, int x1, int y0, int y1, const CRenPar & a, const CRenPar & b, const CRenPar & c, const CRenPar & d, int layer, int xToYHalfRatio, int a23, signed int halfXSize, signed int halfYSize)
{
	CWaterLevel::RenderAndEmptyRenderBuffer();
	float xScale = 1.0f / (float)halfXSize;
	float yScale = 1.0f / (float)halfYSize;

	float xDecr = (float)(x1 - x0) / (float)halfXSize;
	float yDecr = (float)(y1 - y0) / (float)halfYSize;
	float yBaseHeightIncr = (float)(c.baseHeight - a.baseHeight) * yScale;
	float yWave0Incr = (float)(c.wave0Height - a.wave0Height) * yScale;
	float yWave1Incr = (float)(c.wave1Height - a.wave1Height) * yScale;

	float xIncr = (float)(x0 - x1) / (float)halfXSize;
	float yIncr = (float)(y0 - y1) / (float)halfYSize;
	float yBaseHeightDecr = (float)(b.baseHeight - d.baseHeight) * yScale;
	float yWave0Decr = (float)(b.wave0Height - d.wave0Height) * yScale;
	float yWave1Decr = (float)(b.wave1Height - d.wave1Height) * yScale;
	float texScale, texShiftU, texShiftV;
	if (layer)
	{
		texScale = 0.04f;
		texShiftU = TextureShiftSecondU;
		texShiftV = TextureShiftSecondV;
	}
	else
	{
		texScale = 0.08f;
		texShiftU = TextureShiftU;
		texShiftV = TextureShiftV;
	}
	float uBaseOffset = texShiftU + x0 * texScale;
	float vBaseOffset = texShiftV + y0 * texScale;
	// clamp offsets
	uBaseOffset = uBaseOffset - floorf(uBaseOffset);
	vBaseOffset = vBaseOffset - floorf(vBaseOffset);

	auto camPos = TheCamera.GetPosition();
	float camPosX = camPos.x;
	float camPosY = camPos.y;
	int x, y;
	float baseHeight, wave0Height, wave1Height;
	if (halfYSize >= 0)
	{
		int yd = 0;
		int yc = 0;

		int yi = y1 + (int)(halfYSize * yIncr);
		do
		{
			if (halfXSize >= 0)
			{
				int xc = 0;
				int xd = 0;
				int xi = x1 + (int)(halfXSize * xIncr);
				int ycinv = halfYSize - yc;
				do
				{
					int xcinv = halfXSize - xc;
					if (yc * yScale + xc * xScale < 1.0)
					{
						x = xd + x0;
						y = yd + y0;
						baseHeight = (a.baseHeight + (b.baseHeight - a.baseHeight) * xScale * xc) + yBaseHeightIncr * yc;
						wave0Height = (a.wave0Height + (b.wave0Height - a.wave0Height) * xScale * xc) + yWave0Incr * yc;
						wave1Height = (a.wave1Height + (b.wave1Height - a.wave1Height) * xScale * xc) + yWave1Incr * yc;
					}
					else
					{
						x = xi;
						y = yi;
						
						baseHeight = d.baseHeight + (c.baseHeight - d.baseHeight) * xScale * xcinv + yBaseHeightDecr * ycinv;
						wave0Height = d.wave0Height + (c.wave0Height - d.wave0Height) * xScale * xcinv + yWave0Decr * ycinv;
						wave1Height = d.wave1Height + (c.wave1Height - d.wave1Height) * xScale * xcinv + yWave1Decr * ycinv;
					}
					float detailScale = sqrtf(((camPosY - y) * (camPosY - y)) + ((camPosX - x) * (camPosX - x))) / DETAILEDWATERDIST;

					float waterHeightScale = 1.0f;
					if (detailScale <= 1.0f)
					{
						if (detailScale > 0.75f)
							waterHeightScale = (1.0f - detailScale) * 4.0f;
					}
					else
					{
						waterHeightScale = 0.0f;
					}
					if (layer == 1)
					{
					}
					else
					{
						float diffuse, sunGlare;
						CWaterLevel::CalculateWavesForCoordinate(x, y,
							wave0Height * waterHeightScale, wave1Height * waterHeightScale, &baseHeight, &diffuse, &sunGlare, (CVector*)&TempVertexBuffer[TempBufferVerticesStored].objNormal);
						TempVertexBuffer[TempBufferVerticesStored].u = xd * 0.08f + uBaseOffset;
						TempVertexBuffer[TempBufferVerticesStored].v = xd * 0.08f + vBaseOffset;
						TempVertexBuffer[TempBufferVerticesStored].objVertex.x = (float)x;
						TempVertexBuffer[TempBufferVerticesStored].objVertex.y = (float)y;
						TempVertexBuffer[TempBufferVerticesStored].objVertex.z = baseHeight;
						TempVertexBuffer[TempBufferVerticesStored].color = 0xFFFFFFFF;
					}
					if (xc & 1 && yc & 1)
					{
						TempBufferRenderIndexList[TempBufferIndicesStored] = TempBufferVerticesStored - halfXSize - 2;
						TempBufferRenderIndexList[TempBufferIndicesStored + 1] = TempBufferVerticesStored - halfXSize - 1;
						TempBufferRenderIndexList[TempBufferIndicesStored + 2] = TempBufferVerticesStored - 1;
						TempBufferRenderIndexList[TempBufferIndicesStored + 3] = TempBufferVerticesStored;
						TempBufferRenderIndexList[TempBufferIndicesStored + 4] = TempBufferVerticesStored - halfXSize - 1;
						TempBufferRenderIndexList[TempBufferIndicesStored + 5] = TempBufferVerticesStored - 1;
						TempBufferIndicesStored = TempBufferIndicesStored + 6;
					}
					++xc;
					xd += (int)xDecr;
					xi -= (int)xIncr;
					TempBufferVerticesStored++;
				} while (halfXSize >= xc);
			}
			++yc;
			yd += (int)yDecr;
			yi -= (int)yIncr;
		} while (halfYSize >= yc);
	}
}
