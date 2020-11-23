#pragma once
#include "DeferredPipeline.h"
#include "RenderMeshPool.h"

class CCustomBuildingDNPipeline :
    public CDeferredPipeline
{
public:
    CCustomBuildingDNPipeline();
    ~CCustomBuildingDNPipeline();
    static void Patch();
    void SetPipeline();
    void        Render( RwResEntry *repEntry, void *object, RwUInt8 type,
                        RwUInt32 flags );
    void        ResetAlphaList(); 
    void        RenderAlphaList();

  private:
    static CRenderMeshPool<AlphaMesh> m_aAlphaMeshList;
};
extern CCustomBuildingDNPipeline* g_pCustomBuildingDNPipe;