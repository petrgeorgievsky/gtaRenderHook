#pragma once
#include <stdint.h>
namespace rh::rw::engine {
class RpGeometryInterface;
}
typedef struct RxPipelineNode RxPipelineNodeInstance;
struct RxPipelineNodeParam;

class RenderingPipelineExecutor
{
    void PostInstance( rh::rw::engine::RpGeometryInterface *geometry_proxy );
    void Render( rh::rw::engine::RpGeometryInterface *geometry_proxy );
};

class RwDefaultRenderingPipeline
{
    static int32_t AtomicAllInOneNode( RxPipelineNodeInstance *self,
                                       const RxPipelineNodeParam *params );
    static RenderingPipelineExecutor *Executor;
};
