#pragma once
#include <stdint.h>
namespace rw_rh_engine {
class RpGeometryInterface;
}
typedef struct RxPipelineNode RxPipelineNodeInstance;
struct RxPipelineNodeParam;

class RenderingPipelineExecutor
{
    void PostInstance( rw_rh_engine::RpGeometryInterface *geometry_proxy );
    void Render( rw_rh_engine::RpGeometryInterface *geometry_proxy );
};

class RwDefaultRenderingPipeline
{
    static int32_t AtomicAllInOneNode( RxPipelineNodeInstance *self,
                                       const RxPipelineNodeParam *params );
    static RenderingPipelineExecutor *Executor;
};
