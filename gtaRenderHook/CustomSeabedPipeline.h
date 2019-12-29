#pragma once
#include "DeferredPipeline.h"
class CCustomSeabedPipeline :
    public CDeferredPipeline
{
public:
    CCustomSeabedPipeline();
    ~CCustomSeabedPipeline();
    void RenderSeabed( RwIm3DVertex * verticles, UINT vertexCount, USHORT* indices, UINT indexCount );
private:
    RwRaster* m_pSeaBedTexRaster = nullptr;
    ID3D11InputLayout*      m_pVertexLayout = nullptr;
    ID3D11Buffer*           m_pVertexBuffer = nullptr;
    ID3D11Buffer*           m_pIndexBuffer = nullptr;
};
extern CCustomSeabedPipeline* g_pCustomSeabedPipe;
