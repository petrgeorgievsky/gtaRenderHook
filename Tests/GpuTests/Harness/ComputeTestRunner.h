#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace rh::engine
{
class IBuffer;
class IDescriptorSet;
class IDescriptorSetAllocator;
class IDescriptorSetLayout;
class IPipelineLayout;
class IShader;
class VulkanCommandBuffer;
class VulkanComputePipeline;
class VulkanDeviceState;
} // namespace rh::engine

namespace rh::tests::gpu
{

struct BufferBinding
{
    uint32_t             set;
    uint32_t             binding;
    rh::engine::IBuffer *buffer;
};

struct Dispatch
{
    uint32_t x = 1;
    uint32_t y = 1;
    uint32_t z = 1;
};

// Headless runner for compute shader tests: no window, no swapchain.
// Every binding is treated as a read-write storage buffer.
class ComputeTestRunner
{
  public:
    static bool IsAvailable();

    ComputeTestRunner();
    ~ComputeTestRunner();
    ComputeTestRunner( const ComputeTestRunner & )            = delete;
    ComputeTestRunner &operator=( const ComputeTestRunner & ) = delete;

    rh::engine::IBuffer *CreateStorageBuffer( uint32_t size,
                                              const void *init = nullptr );

    void Run( const std::string             &shader_path,
              std::span<const BufferBinding> bindings,
              Dispatch                       dispatch = {} );

    std::vector<std::byte> Readback( rh::engine::IBuffer *buffer,
                                     uint32_t            size );

  private:
    struct Pipeline
    {
        std::string                          key;
        rh::engine::IDescriptorSetLayout    *layout      = nullptr;
        rh::engine::IDescriptorSetAllocator *alloc       = nullptr;
        rh::engine::IDescriptorSet          *set         = nullptr;
        rh::engine::IPipelineLayout         *pipe_layout = nullptr;
        rh::engine::IShader                 *shader      = nullptr;
        rh::engine::VulkanComputePipeline   *pipeline    = nullptr;
    };

    Pipeline &EnsurePipeline( const std::string             &shader_path,
                              std::span<const BufferBinding> bindings );

    rh::engine::VulkanDeviceState                    *m_device = nullptr;
    std::vector<Pipeline>                             m_pipelines;
    std::vector<std::unique_ptr<rh::engine::IBuffer>> m_buffers;
};

} // namespace rh::tests::gpu
