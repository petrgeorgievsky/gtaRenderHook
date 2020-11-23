#pragma once
#include "DeferredPipeline.h"
#include "RenderMeshPool.h"

class CCustomBuildingPipeline :
    public CDeferredPipeline
{
public:
    CCustomBuildingPipeline();
    ~CCustomBuildingPipeline();
    static void Patch();
    void ResetAlphaList();
    void RenderAlphaList();
    void Render( RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags );
private:
    static CRenderMeshPool<AlphaMesh> m_aAlphaMeshList;
};
extern CCustomBuildingPipeline* g_pCustomBuildingPipe;