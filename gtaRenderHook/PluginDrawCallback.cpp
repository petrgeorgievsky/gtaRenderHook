#include "stdafx.h"
#include "PluginDrawCallback.h"
#include "DeferredRenderer.h"
#include "RwRenderEngine.h"
#include "RwD3D1XEngine.h"
#include "D3DRenderer.h"

static RHBuildingDrawCallback g_buildingDrawCallback = nullptr;

void __cdecl RHSetBuildingDrawCallback( RHBuildingDrawCallback callback )
{
    g_buildingDrawCallback = callback;
}

int CurrentBuildingDrawPass()
{
    switch ( m_uiDeferredStage )
    {
    case 1: return RH_PASS_GBUFFER;
    case 2: return RH_PASS_SHADOW;
    case 3:
    case 4: return RH_PASS_OTHER;
    default: return RH_PASS_FORWARD;
    }
}

unsigned int NotifyBuildingDraw( void *atomic, const RwMatrix *worldMatrix, int meshIndex, int pass,
                                 bool dualPassEdges )
{
    if ( !g_buildingDrawCallback )
        return 0;
    RHBuildingDraw draw{};
    draw.size          = sizeof( draw );
    draw.context       = GET_D3D_CONTEXT;
    draw.atomic        = atomic;
    draw.worldMatrix   = reinterpret_cast<const float *>( worldMatrix );
    draw.meshIndex     = meshIndex;
    draw.pass          = pass;
    draw.dualPassEdges = dualPassEdges ? 1 : 0;
    return g_buildingDrawCallback( &draw );
}
