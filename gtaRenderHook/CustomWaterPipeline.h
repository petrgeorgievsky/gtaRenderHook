#pragma once
#include "D3D1XPipeline.h"
#include "SettingsHolder.h"
class CCustomWaterPipeline :
	public CD3D1XPipeline
{
public:
	CCustomWaterPipeline();
	~CCustomWaterPipeline();
	void RenderWater(RwIm3DVertex * verticles, UINT vertexCount, USHORT* indices, UINT indexCount);
	RwRaster* m_pWaterRaster;
	RwRaster* m_pWaterWakeRaster;
	ID3D11InputLayout*      m_pVertexLayout = nullptr;
	ID3D11Buffer*           m_pVertexBuffer = nullptr;
	ID3D11Buffer*           m_pIndexBuffer = nullptr;
	CD3D1XShader*		m_pHS = nullptr;
	CD3D1XShader*		m_pDS = nullptr;
};
extern CCustomWaterPipeline* g_pCustomWaterPipe;
// Tonemap settings block class
class WaterSettingsBlock : public SettingsBlock {
public:
	WaterSettingsBlock() {
		m_sName = "WaterSettings";
		Reset();
	}
	tinyxml2::XMLElement* Save(tinyxml2::XMLDocument* doc);
	void Load(const tinyxml2::XMLDocument &doc);
	void Reset();
	void InitGUI(TwBar*);
public:
	bool EnableWater;
};
extern WaterSettingsBlock gWaterSettings;
