//
// Created by peter on 16.04.2020.
//
#include "scene_graph.h"
#include "render_loop.h"
#include <ipc/MemoryWriter.h>
namespace rh::rw::engine
{

SceneGraph *GetCurrentSceneGraph()
{
    // TODO: Add injector process version
    static SceneGraph local_graph{};
    return &local_graph;
}

void SerializeSceneGraph( MemoryWriter &memory_writer )
{
    GetCurrentSceneGraph()->mFrameId =
        ( GetCurrentSceneGraph()->mFrameId + 1 ) % 1000;

    memory_writer.Write( &GetCurrentSceneGraph()->mFrameInfo );

    EngineClient::gIm2DGlobals.Serialize( memory_writer );
    EngineClient::gIm3DGlobals.Serialize( memory_writer );
    EngineClient::gSkinRendererGlobals.Serialize( memory_writer );
    EngineClient::gRendererGlobals.Serialize( memory_writer );
}
} // namespace rh::rw::engine