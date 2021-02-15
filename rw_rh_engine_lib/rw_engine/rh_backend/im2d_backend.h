#pragma once
#include "../../common_headers.h"
#include <Engine/Common/IShader.h>
#include <ipc/MemoryWriter.h>
#include <unordered_map>
#include <vector>
namespace rh
{

namespace engine
{
class IWindow;
class ISyncPrimitive;
class IImageBuffer;
class IImageView;
class IPipeline;
class IPipelineLayout;
class IDescriptorSet;
class IDescriptorSetAllocator;
class IBuffer;
class ICommandBuffer;
class IDescriptorSetLayout;
class IRenderPass;
class ISampler;
} // namespace engine

namespace rw::engine
{

struct Im2DState
{
    uint8_t mColorBlendSrc;
    uint8_t mColorBlendDst;
    uint8_t mColorBlendOp;
    uint8_t mBlendEnable;

    uint8_t mZTestEnable;
    uint8_t mZWriteEnable;
    uint8_t mStencilEnable;
};

struct Im2DDrawCall
{
    uint64_t mRasterId;
    uint32_t mVertexBufferOffset;
    uint32_t mVertexCount;
    uint32_t mIndexBufferOffset;
    uint32_t mIndexCount;
    // BlendState
    Im2DState mBlendState;
};
int32_t Im2DRenderPrimitiveFunction( RwPrimitiveType primType,
                                     RwIm2DVertex *  vertices,
                                     int32_t         numVertices );
int32_t Im2DRenderIndexedPrimitiveFunction( RwPrimitiveType primType,
                                            RwIm2DVertex *  vertices,
                                            int32_t         numVertices,
                                            int16_t *       indices,
                                            int32_t         numIndices );
struct ImmediateState;
class Im2DClientGlobals
{
  public:
    Im2DClientGlobals( ImmediateState &im_state ) noexcept;

    void     RecordDrawCall( RwIm2DVertex *vertices, int32_t numVertices );
    void     RecordDrawCall( RwIm2DVertex *vertices, int32_t numVertices,
                             int16_t *indices, int32_t numIndices );
    uint64_t Serialize( MemoryWriter &writer );
    void     Flush();

  private:
    ImmediateState &          ImState;
    std::vector<Im2DDrawCall> mDrawCalls{};
    std::vector<RwIm2DVertex> mVertexBuffer{};
    std::vector<int16_t>      mIndexBuffer{};
    uint32_t                  mIndexCount    = 0;
    uint32_t                  mVertexCount   = 0;
    uint32_t                  mDrawCallCount = 0;
};

} // namespace rw::engine

} // namespace rh