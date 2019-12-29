#pragma once
namespace rh::engine
{
struct ImageBufferInfo;
struct VertexBufferInfo;
struct IndexBufferInfo;
struct ConstantBufferInfo;
struct StructuredBufferInfo;
struct InputLayoutInfo;
struct Sampler;
struct ShaderInfo;
struct DepthStencilState;
struct BlendState;
struct RasterizerState;
class IGPUResource;
enum class ImageBufferType : unsigned char;
// TODO: add comments
class IGPUAllocator
{
  public:
    virtual ~IGPUAllocator()                                      = default;
    virtual bool AllocateImageBuffer( const ImageBufferInfo &info,
                                      IGPUResource *&        buffer_ptr ) = 0;
    virtual bool FreeImageBuffer( void *buffer, ImageBufferType type )     = 0;
    virtual bool AllocateVertexBuffer( const VertexBufferInfo &info,
                                       IGPUResource *&         buffer_ptr )         = 0;
    virtual bool AllocateIndexBuffer( const IndexBufferInfo &info,
                                      IGPUResource *&        buffer_ptr )          = 0;
    virtual bool AllocateConstantBuffer( const ConstantBufferInfo &info,
                                         IGPUResource *&buffer_ptr )       = 0;
    virtual bool AllocateStructuredBuffer( const StructuredBufferInfo &info,
                                           IGPUResource *&buffer_ptr )     = 0;
    virtual bool AllocateInputLayout( const InputLayoutInfo &info,
                                      IGPUResource *&        buffer_ptr )          = 0;
    virtual bool AllocateSampler( const Sampler &info, void *&buffer_ptr ) = 0;
    virtual bool AllocateShader( const ShaderInfo &info,
                                 IGPUResource *&   buffer_ptr )               = 0;
    virtual bool AllocateDepthStencilState( const DepthStencilState &info,
                                            void *&buffer_ptr )            = 0;
    virtual bool AllocateBlendState( const BlendState &info,
                                     void *&           buffer_ptr )                   = 0;
    virtual bool AllocateRasterizerState( const RasterizerState &info,
                                          void *&buffer_ptr )              = 0;
    virtual bool AllocateDeferredContext( IGPUResource *&context_ptr )     = 0;
};
}; // namespace rh::engine
