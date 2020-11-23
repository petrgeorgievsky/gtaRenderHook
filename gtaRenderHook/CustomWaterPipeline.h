#pragma once
#include "D3D1XPipeline.h"
#include "SettingsHolder.h"
class CCustomWaterPipeline :
    public CD3D1XPipeline
{
public:
    CCustomWaterPipeline();
    ~CCustomWaterPipeline();
    void RenderWater( RwIm3DVertex * verticles, UINT vertexCount, USHORT* indices, UINT indexCount );
    RwRaster* m_pWaterRaster = nullptr;
    RwRaster* m_pWaterWakeRaster = nullptr;
    ID3D11InputLayout*      m_pVertexLayout = nullptr;
    ID3D11Buffer*           m_pVertexBuffer = nullptr;
    ID3D11Buffer*           m_pIndexBuffer = nullptr;
    CD3D1XShader*		m_pHS = nullptr;
    CD3D1XShader*		m_pDS = nullptr;
};
extern CCustomWaterPipeline* g_pCustomWaterPipe;
// Tonemap settings block class
class WaterSettingsBlock : public SettingsBlock
{
public:
    WaterSettingsBlock()
    {
        m_sName = "WaterSettings";
        m_aFields["EnableWater"] = new ToggleSField( "EnableWater", false, false, false, m_sName, true );
        Reset();
    }
    void InitGUI( TwBar* );
};
extern WaterSettingsBlock gWaterSettings;
