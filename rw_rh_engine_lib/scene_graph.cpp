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

} // namespace rh::rw::engine