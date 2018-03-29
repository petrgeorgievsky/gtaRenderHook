#include "stdafx.h"
#include "CWaterLevel.h"
#include "CustomWaterPipeline.h"
#include "CustomSeabedPipeline.h"

#include <game_sa\CCamera.h>

UINT &CWaterLevel::m_NumBlocksOutsideWorldToBeRendered = *(UINT*)0xC215EC;
UINT &CWaterLevel::m_nNumOfWaterTriangles = *(UINT*)0xC22884;
UINT &CWaterLevel::m_nNumOfWaterQuads = *(UINT*)0xC22888;

USHORT* CWaterLevel::m_BlocksToBeRenderedOutsideWorldX = (USHORT*)0xC21560;
USHORT* CWaterLevel::m_BlocksToBeRenderedOutsideWorldY = (USHORT*)0xC214D0;

CWaterVertice* CWaterLevel::m_aVertices = (CWaterVertice*)0xC22910;
CWaterTriangle* CWaterLevel::m_aTriangles = (CWaterTriangle*)0xC22854;
CWaterQuad* CWaterLevel::m_aQuads = (CWaterQuad*)0xC21C90;
//     ; CWaterVertice CWaterLevel::m_aVertices[1021]
//
float TextureShiftU = 0;
float TextureShiftV = 0;
float TextureShiftSecondU = 0;
float TextureShiftSecondV = 0;
float someShift = 0.5; 
float someShift2 = 0.5;
void CWaterLevel::RenderWater()
{
	if (CGame__currArea && CGame__currArea != 5)
		return;
	//CWaterLevel_RenderWater();
	SetCameraRange();
	//TempBufferVerticesStored = 0;
	//TempBufferIndicesStored = 0;
	//RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEZWRITEENABLE, 0);
	//RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATETEXTUREADDRESS, dword_8D3930);
	/*float v10 = CTimer_ms_fTimeStep * CWaterLevel::m_CurrentFlowX * flt_8D392C;
	flt_8D3824 = 0.079999998 * v10 + flt_8D3824;
	v9 = CTimer::ms_fTimeStep * CWaterLevel::m_CurrentFlowY * flt_8D392C;
	flt_8D3828 = 0.079999998 * v9 + flt_8D3828;
	if (flt_8D3824 > 1.0)
		flt_8D3824 = flt_8D3824 - 1.0;
	if (flt_8D3828 > 1.0)
		flt_8D3828 = flt_8D3828 - 1.0;
	flt_8D382C = v10 * 0.039999999 + flt_8D382C;
	flt_8D3830 = v9 * 0.039999999 + flt_8D3830;
	if (flt_8D382C > 1.0)
		flt_8D382C = flt_8D382C - 1.0;
	if (flt_8D3830 > 1.0)
		flt_8D3830 = flt_8D3830 - 1.0;
	v12 = (CTimer::m_snTimeInMilliseconds & 0xFFF) * 0.0015339808;
	flt_C21184 = sin(v12) * flt_C812E8 * 0.079999998 + flt_8D3824;
	flt_C21180 = cos(v12) * flt_C812E8 * 0.079999998 + flt_8D3828;
	dword_C2117C = LODWORD(flt_8D382C);
	v13 = (CTimer::m_snTimeInMilliseconds & 0x1FFF) * 0.00076699042;
	v14 = *&dword_8D3928;
	v15 = cos(v13);
	v16 = v15;
	flt_C21178 = v15 * 0.024 + flt_8D3830;
	v17 = rand() * 0.000030518509 * v14;
	v18 = *&dword_8D3928;
	flt_C21174 = v17;
	v19 = rand();
	flt_C21174 = sin(v13) * flt_8D3834 + flt_C21174;
	flt_C21170 = v19 * 0.000030518509 * v18 + v16 * flt_8D3834;
	byte_C2116C = flt_B7C508;
	unk_C2116D = flt_B7C50C;
	byte_C2116E = flt_B7C510;
	dword_C21168 = *&byte_C2116C;
	v20 = (flt_B7C514 * 0.5);
	dword_8D380C = v20;
	v11 = (v20 << 8) / (256 - v20);
	dword_8D3808 = 255;
	if (v11 <= 255)
		dword_8D3808 = v11;*/
	TempBufferIndicesStored = 0;
	TempBufferVerticesStored = 0;
	WaterColor = { 255,255,255,255 };
	
	someShift += 0.079999998 * CTimer__ms_fTimeStep * 0.04;
	someShift2 += 0.079999998 * CTimer__ms_fTimeStep * 0.04;
	if (someShift > 3.14*2)
		someShift -= 3.14 * 2;
	if (someShift2 > 3.14 * 2)
		someShift2 -= 3.14 * 2;
	
	float v12 = (CTimer__m_snTimeInMilliseconds & 0xFFF) * 0.0015339808;
	TextureShiftU = someShift;
	TextureShiftV = someShift2;
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
				RenderWaterRectangle(
					m_aVertices[a].posX, m_aVertices[b].posX, m_aVertices[a].posY, m_aVertices[c].posY,
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

		float x = 500 * m_BlocksToBeRenderedOutsideWorldX[k];
		float y = 500 * m_BlocksToBeRenderedOutsideWorldY[k];

		RenderWaterRectangle(
			(x - 3000.0), ((x + 500) - 3000.0), (y - 3000.0), ((y + 500) - 3000.0),
			{ 0.0,	1.0,	0.0,	0.0 }, { 0.0,	1.0,	0.0,	0.0 },
			{ 0.0,	1.0,	0.0,	0.0 }, { 0.0,	1.0,	0.0,	0.0 });

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
	CWaterLevel_SetCameraRange();
}

void CWaterLevel::RenderBoatWakes()
{
	CWaterLevel_RenderBoatWakes();
}



CRenPar * __cdecl CRenPar_Lerp(CRenPar *a1, CRenPar a, CRenPar b, float t)
{
	float invT = 1.0f - t;
	
	a1->z = a.z * invT + b.z * t;
	a1->u = a.u * invT + b.u * t;
	a1->v = a.v * invT + b.v * t;
	a1->velocity = 0;
	return a1;
}

void CWaterLevel::RenderWaterRectangle(int x, int y, int z, int w, CRenPar a, CRenPar b, CRenPar c, CRenPar d)
{
	int w0 = w;
	int z0 = z;
	if (z < w)
	{
		w = z;
		z = w0;
	}
	if (x >= CameraRangeMaxX || y <= CameraRangeMinX|| w >= CameraRangeMaxY || z <= CameraRangeMinY)
	{
		RenderFlatWaterRectangle(x, y, z, w, a, b, c, d);
		//CWaterLevel::SetUpWaterFog(x, w, y, z);
	}
	else if (y > CameraRangeMaxX)
	{
		SplitWaterRectangleAlongXLine(CameraRangeMaxX, x, y, z, w, a, b, c, d);
	}
	else if (x < CameraRangeMinX)
	{
		SplitWaterRectangleAlongXLine(CameraRangeMinX, x, y, z, w, a, b, c, d);
	}
	else if (z > CameraRangeMaxY)
	{
		SplitWaterRectangleAlongYLine(CameraRangeMaxY, x, y, z, w, a, b, c, d);
	}
	else if (w < CameraRangeMinY)
	{
		SplitWaterRectangleAlongYLine(CameraRangeMinY, x, y, z, w, a, b, c, d);
	}
	else
		RenderDetailedWaterRectangle_OneLayer(x, y, z, w, a, b, c, d,0);
}

void CWaterLevel::RenderFlatWaterRectangle(int x, int y, int z, int w, CRenPar a, CRenPar b, CRenPar c, CRenPar d)
{
	RenderFlatWaterRectangle_OneLayer(x, y, z, w, a, b, c, d, 0);
	//CWaterLevel_RenderFlatWaterRectangle(x, y, z, w, a, b, c, d);
}

void CWaterLevel::SplitWaterRectangleAlongYLine(int m, int x, int y, int z, int w, CRenPar a, CRenPar b, CRenPar c, CRenPar d)
{
	CRenPar b_d, a_c;
	float blendFactor = (m - z) / (w - z);

	// First split part
	CRenPar_Lerp(&b_d, b, d, blendFactor);
	CRenPar_Lerp(&a_c, a, c, blendFactor);

	RenderWaterRectangle(x, y, z, m, a, b, a_c, b_d);

	// Second split part
	CRenPar_Lerp(&b_d, b, d, blendFactor);
	CRenPar_Lerp(&a_c, a, c, blendFactor);

	RenderWaterRectangle(x, y, m, w, a_c, b_d, c, d);
}

void CWaterLevel::SplitWaterRectangleAlongXLine(int m, int x, int y, int z, int w, CRenPar a, CRenPar b, CRenPar c, CRenPar d)
{
	CRenPar c_d, a_b;
	float blendFactor = (m - x) / (y - x);

	// First split part
	CRenPar_Lerp(&c_d, c, d, blendFactor);
	CRenPar_Lerp(&a_b, a, b, blendFactor);

	RenderWaterRectangle(x, m, z, w, a, a_b, c, c_d);

	// Second split part
	CRenPar_Lerp(&c_d, c, d, blendFactor);
	CRenPar_Lerp(&a_b, a, b, blendFactor);

	RenderWaterRectangle(m, y, z, w, a_b, b, c_d, d);
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


void CWaterLevel::RenderFlatWaterRectangle_OneLayer(int x, int y, int z, int w, CRenPar a, CRenPar b, CRenPar c, CRenPar d, int e)
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
	if (w - z <= 0)
		tV = tV + 7.0;
	else
		tV = tV - 7.0;
	//v17 = *&WaterColor.red;
	
	//v18 = v10;
	TempVertexBuffer[startVertex].u = 0;
	TempVertexBuffer[startVertex].v = 0;
	TempVertexBuffer[startVertex].objVertex.x = x;
	TempVertexBuffer[startVertex].objVertex.y = z;
	TempVertexBuffer[startVertex].objVertex.z = a.z + height;
	TempVertexBuffer[startVertex].objNormal.z = 1.0;
	TempVertexBuffer[startVertex].color = 0xFFFFFFFF;
	float tU2 = (y - x) * v27 + tU;
	float tV2 = (w - z) * v27 + tV;
	TempVertexBuffer[startVertex + 1].u = 1;
	TempVertexBuffer[startVertex + 1].v = 0;
	TempVertexBuffer[startVertex + 1].objVertex.x = y;
	TempVertexBuffer[startVertex + 1].objVertex.y = z;
	TempVertexBuffer[startVertex + 1].objVertex.z = b.z + height;
	TempVertexBuffer[startVertex + 1].objNormal.z = 1.0;
	TempVertexBuffer[startVertex + 1].color = 0xFFFFFFFF;

	TempVertexBuffer[startVertex + 2].u = 0;
	TempVertexBuffer[startVertex + 2].v = 1;
	TempVertexBuffer[startVertex + 2].objVertex.x = x;
	TempVertexBuffer[startVertex + 2].objVertex.y = w;
	TempVertexBuffer[startVertex + 2].objVertex.z = c.z + height;
	TempVertexBuffer[startVertex + 2].objNormal.z = 1.0;
	TempVertexBuffer[startVertex + 2].color = 0xFFFFFFFF;

	TempVertexBuffer[startVertex + 3].u = 1;
	TempVertexBuffer[startVertex + 3].v = 1;
	TempVertexBuffer[startVertex + 3].objVertex.x = y;
	TempVertexBuffer[startVertex + 3].objVertex.y = w;
	TempVertexBuffer[startVertex + 3].objVertex.z = d.z + height;
	TempVertexBuffer[startVertex + 3].objNormal.z = 1.0;
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

void CWaterLevel::RenderDetailedWaterRectangle_OneLayer(int x, int y, int z, int w, CRenPar a, CRenPar b, CRenPar c, CRenPar d, int e)
{
	int startIndex, startVertex;
	if (TempBufferIndicesStored >= 4090 || TempBufferVerticesStored >= 2044)
	{
		RenderAndEmptyRenderBuffer();
	}
	
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
	if (w - z <= 0)
		tV = tV + 7.0;
	else
		tV = tV - 7.0;
	float tU2 = (y - x) * v27 + tU;
	float tV2 = (w - z) * v27 + tV;
	int tessFactor = 16;
	float xSize = (y - x)/ (float)tessFactor;
	float ySize = (w - z)/ (float)tessFactor;
	for (size_t x_offset = 0; x_offset < tessFactor; x_offset++)
	{
		for (size_t y_offset = 0; y_offset < tessFactor; y_offset++)
		{
			startIndex = TempBufferIndicesStored;
			startVertex = TempBufferVerticesStored;
			TempVertexBuffer[startVertex].u = x_offset / (float)tessFactor;
			TempVertexBuffer[startVertex].v = y_offset / (float)tessFactor;
			TempVertexBuffer[startVertex].objVertex.x = x + xSize*x_offset;
			TempVertexBuffer[startVertex].objVertex.y = z + ySize*y_offset;
			TempVertexBuffer[startVertex].objVertex.z = a.z + height;
			TempVertexBuffer[startVertex].objNormal.z = 1.0;
			TempVertexBuffer[startVertex].color = 0xFFFFFFFF;

			TempVertexBuffer[startVertex + 1].u =( x_offset+1) / (float)tessFactor;
			TempVertexBuffer[startVertex + 1].v = y_offset / (float)tessFactor;
			TempVertexBuffer[startVertex + 1].objVertex.x = x + xSize*(x_offset + 1);
			TempVertexBuffer[startVertex + 1].objVertex.y = z + ySize*y_offset;
			TempVertexBuffer[startVertex + 1].objVertex.z = b.z + height;
			TempVertexBuffer[startVertex + 1].objNormal.z = 1.0;
			TempVertexBuffer[startVertex + 1].color = 0xFFFFFFFF;

			TempVertexBuffer[startVertex + 2].u = x_offset / (float)tessFactor;
			TempVertexBuffer[startVertex + 2].v = (y_offset+1) / (float)tessFactor;
			TempVertexBuffer[startVertex + 2].objVertex.x = x + xSize*x_offset;
			TempVertexBuffer[startVertex + 2].objVertex.y = z + ySize*(y_offset + 1);
			TempVertexBuffer[startVertex + 2].objVertex.z = c.z + height;
			TempVertexBuffer[startVertex + 2].objNormal.z = 1.0;
			TempVertexBuffer[startVertex + 2].color = 0xFFFFFFFF;

			TempVertexBuffer[startVertex + 3].u = (x_offset+1) / (float)tessFactor;
			TempVertexBuffer[startVertex + 3].v = (y_offset+1) / (float)tessFactor;
			TempVertexBuffer[startVertex + 3].objVertex.x = x + xSize*(x_offset + 1);
			TempVertexBuffer[startVertex + 3].objVertex.y = z + ySize*(y_offset + 1);
			TempVertexBuffer[startVertex + 3].objVertex.z = d.z + height;
			TempVertexBuffer[startVertex + 3].objNormal.z = 1.0;
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
