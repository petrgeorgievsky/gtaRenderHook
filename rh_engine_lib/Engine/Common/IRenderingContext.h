#pragma once
#include <Engine/Common/IGPUResource.h>
#include <vector>
namespace rh::engine
{
struct ViewPort;
struct IndexPtrPair;
enum class ImageBindType : uint8_t;
enum class ImageClearType : uint8_t;
enum ShaderStage : uint32_t;
enum class PrimitiveType : uint8_t;
// TODO: add comments
class IRenderingContext : public IGPUResource
{
  public:
    virtual ~IRenderingContext() = default;
    virtual void BindViewPorts( const std::vector<ViewPort> &viewports ) = 0;
    virtual bool
    BindImageBuffers( ImageBindType                    bindType,
                      const std::vector<IndexPtrPair> &buffers ) = 0;

    virtual bool ClearImageBuffer( ImageClearType clearType, void *buffer,
                                   const std::array<float, 4> &clearColor ) = 0;
    virtual void FlushCache()                                               = 0;
    virtual void ReleaseResources()                                         = 0;
    virtual void FinishCmdList( void *&cmdlist )                            = 0;
    virtual void ReplayCmdList( void *cmdlist )                             = 0;

    virtual void BindVertexBuffers( const std::vector<void *> &buffers,
                                    uint32_t *strides, uint32_t *offsets ) = 0;
    virtual bool BindIndexBuffer( void *buffer )                           = 0;
    virtual bool BindInputLayout( void *layout )                           = 0;
    virtual void BindShader( void *shader )                                = 0;
    virtual void ResetShader( void *shader )                               = 0;
    virtual bool
                 BindConstantBuffers( uint8_t                          bindStages,
                                      const std::vector<IndexPtrPair> &buffers ) = 0;
    virtual void BindSamplers( const std::vector<IndexPtrPair> &sampler_states,
                               ShaderStage                      stage )          = 0;
    virtual void UpdateBuffer( void *buffer, const void *data,
                               int32_t size )               = 0;
    virtual void SetPrimitiveTopology( PrimitiveType type ) = 0;

    virtual void DispatchThreads( uint32_t x, uint32_t y, uint32_t z ) = 0;

    virtual void RecordDrawCall( uint32_t vertexCount,
                                 uint32_t startVertex )       = 0;
    virtual void RecordDrawCallIndexed( uint32_t indexCount,
                                        uint32_t startIndex,
                                        uint32_t baseVertex ) = 0;
    virtual void CopyImageBuffer( void *dest_buffer, void *src_buffer,
                                  uint32_t dest_subres_id, uint32_t dest_pos_x,
                                  uint32_t dest_pos_y,
                                  uint32_t dest_pos_z )       = 0;
};

}; // namespace rh::engine
