#include "ComputeTestRunner.h"

#include <DebugUtils/DebugLogger.h>
#include <Engine/Common/IBuffer.h>
#include <Engine/Common/IDescriptorSet.h>
#include <Engine/Common/IDescriptorSetAllocator.h>
#include <Engine/Common/IDescriptorSetLayout.h>
#include <Engine/Common/IPipelineLayout.h>
#include <Engine/Common/IShader.h>
#include <Engine/VulkanImpl/VulkanCommandBuffer.h>
#include <Engine/VulkanImpl/VulkanComputePipeline.h>
#include <Engine/VulkanImpl/VulkanDeviceState.h>
#include <rendering_loop/DescriptorGenerator.h>
#include <rendering_loop/DescriptorUpdater.h>

#include "TestPaths.h"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace rh::tests::gpu
{

bool ComputeTestRunner::IsAvailable()
{
    static const bool available = []() {
        char  *skip_gpu = nullptr;
        size_t skip_len = 0;
        if ( _dupenv_s( &skip_gpu, &skip_len, "RH_SKIP_GPU_TESTS" ) == 0 &&
             skip_gpu != nullptr )
        {
            free( skip_gpu );
            return false;
        }
        free( skip_gpu );

        rh::engine::VulkanDeviceState device;
        unsigned int               adapter_count = 0;
        if ( !device.GetAdaptersCount( adapter_count ) || adapter_count == 0 )
            return false;
        if ( !device.Init() )
            return false;
        device.Shutdown();
        return true;
    }();
    return available;
}

ComputeTestRunner::ComputeTestRunner()
{
    rh::debug::DebugLogger::Init( ( ModuleDir() / "rh_gpu_tests.log" ).string(),
                                  rh::debug::LogLevel::Info );

    m_device = new rh::engine::VulkanDeviceState();
    if ( !m_device->Init() )
    {
        std::fprintf( stderr,
                      "rh_gpu_tests: failed to initialize Vulkan device\n" );
        std::abort();
    }
}

ComputeTestRunner::~ComputeTestRunner()
{
    for ( auto &p : m_pipelines )
    {
        delete p.pipeline;
        delete p.shader;
        delete p.pipe_layout;
        delete p.set;
        delete p.alloc;
        delete p.layout;
    }
    m_buffers.clear();

    if ( m_device != nullptr )
    {
        m_device->Shutdown();
        delete m_device;
    }
}

rh::engine::IBuffer *
ComputeTestRunner::CreateStorageBuffer( uint32_t size, const void *init )
{
    auto *buffer = m_device->CreateBuffer( {
        .mSize        = size,
        .mUsage       = rh::engine::BufferUsage::StorageBuffer,
        .mFlags       = rh::engine::BufferFlags::Dynamic,
        .mInitDataPtr = const_cast<void *>( init ),
    } );
    m_buffers.emplace_back( buffer );
    return buffer;
}

ComputeTestRunner::Pipeline &
ComputeTestRunner::EnsurePipeline( const std::string             &shader_path,
                                   std::span<const BufferBinding> bindings )
{
    std::string key = shader_path;
    for ( const auto &binding : bindings )
        key += "|" + std::to_string( binding.set ) + ":" +
               std::to_string( binding.binding );

    for ( auto &pipeline : m_pipelines )
        if ( pipeline.key == key )
            return pipeline;

    Pipeline pipeline{};
    pipeline.key = key;

    rh::rw::engine::DescriptorGenerator generator{ *m_device };
    for ( const auto &binding : bindings )
        generator.AddDescriptor( binding.set, binding.binding, binding.binding,
                                 rh::engine::DescriptorType::RWBuffer, 1,
                                 rh::engine::ShaderStage::Compute );

    pipeline.layout = generator.FinalizeDescriptorSet( 0, 1 );
    pipeline.alloc  = generator.FinalizeAllocator();
    pipeline.set =
        pipeline.alloc->AllocateDescriptorSets( { { pipeline.layout } } )[0];

    std::array layouts = { pipeline.layout };
    pipeline.pipe_layout =
        m_device->CreatePipelineLayout( { .mSetLayouts = layouts } );

    pipeline.shader = m_device->CreateShader(
        { .mShaderPath  = shader_path,
          .mEntryPoint  = "main",
          .mShaderStage = rh::engine::ShaderStage::Compute } );

    pipeline.pipeline = m_device->CreateComputePipeline(
        { .mLayout      = pipeline.pipe_layout,
          .mShaderStage = { .mStage      = rh::engine::ShaderStage::Compute,
                            .mShader     = pipeline.shader,
                            .mEntryPoint = "main" } } );

    m_pipelines.push_back( pipeline );
    return m_pipelines.back();
}

void ComputeTestRunner::Run( const std::string             &shader_path,
                             std::span<const BufferBinding> bindings,
                             Dispatch                       dispatch )
{
    auto &pipeline = EnsurePipeline( shader_path, bindings );

    rh::rw::engine::DescSetUpdateBatch batch{ *m_device };
    auto                                 &update = batch.Begin( pipeline.set );
    for ( const auto &binding : bindings )
        update.UpdateBuffer( binding.binding,
                             rh::engine::DescriptorType::RWBuffer,
                             { rh::engine::BufferUpdateInfo{
                                 0, VK_WHOLE_SIZE, binding.buffer } } );
    update.End();

    auto cmd = dynamic_cast<rh::engine::VulkanCommandBuffer *>(
        m_device->CreateCommandBuffer() );

    cmd->BeginRecord();
    cmd->BindComputePipeline( pipeline.pipeline );
    cmd->BindDescriptorSets( { rh::engine::PipelineBindPoint::Compute,
                               pipeline.pipe_layout,
                               0,
                               { pipeline.set } } );
    cmd->DispatchCompute( { dispatch.x, dispatch.y, dispatch.z } );
    cmd->PipelineBarrier(
        { .mSrcStage = rh::engine::PipelineStage::ComputeShader,
          .mDstStage = rh::engine::PipelineStage::Host,
          .mMemoryBarriers = { rh::engine::MemoryBarrierInfo{
              rh::engine::MemoryAccessFlags::ShaderWrite,
              rh::engine::MemoryAccessFlags::HostRead } } } );
    cmd->EndRecord();

    m_device->ExecuteCommandBuffer( cmd, nullptr, nullptr );
    m_device->WaitForGPU();
    delete cmd;
}

std::vector<std::byte> ComputeTestRunner::Readback( rh::engine::IBuffer *buffer,
                                                    uint32_t             size )
{
    std::vector<std::byte> result( size );
    void                  *mapped = buffer->Lock();
    std::memcpy( result.data(), mapped, size );
    buffer->Unlock();
    return result;
}

} // namespace rh::tests::gpu
