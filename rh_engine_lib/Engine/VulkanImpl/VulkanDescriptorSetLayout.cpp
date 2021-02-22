#include "VulkanDescriptorSetLayout.h"
#include "VulkanCommon.h"
#include "VulkanConvert.h"
using namespace rh::engine;

constexpr vk::ShaderStageFlags ConvertShaderStage( uint32_t stage )
{
    vk::ShaderStageFlags flags{};
    if ( stage & ShaderStage::Compute )
        flags |= vk::ShaderStageFlagBits::eCompute;
    if ( stage & ShaderStage::Vertex )
        flags |= vk::ShaderStageFlagBits::eVertex;
    if ( stage & ShaderStage::Pixel )
        flags |= vk::ShaderStageFlagBits::eFragment;
    if ( stage & ShaderStage::Hull )
        flags |= vk::ShaderStageFlagBits::eGeometry;
    if ( stage & ShaderStage::RayGen )
        flags |= vk::ShaderStageFlagBits::eRaygenNV;
    if ( stage & ShaderStage::RayHit )
        flags |= vk::ShaderStageFlagBits::eClosestHitNV;
    if ( stage & ShaderStage::RayMiss )
        flags |= vk::ShaderStageFlagBits::eMissNV;
    if ( stage & ShaderStage::RayAnyHit )
        flags |= vk::ShaderStageFlagBits::eAnyHitNV;
    return flags;
}
constexpr vk::DescriptorBindingFlags ConvertDescriptorFlags( uint32_t flags )
{
    vk::DescriptorBindingFlags result{};
    if ( flags & DescriptorFlags::dfUpdateAfterBind )
        result |= vk::DescriptorBindingFlagBits::eUpdateAfterBind;
    if ( flags & DescriptorFlags::dfUpdateUnusedWhilePending )
        result |= vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending;
    if ( flags & DescriptorFlags::dfVariableDescriptorCount )
        result |= vk::DescriptorBindingFlagBits::eVariableDescriptorCount;
    if ( flags & DescriptorFlags::dfPartiallyBound )
        result |= vk::DescriptorBindingFlagBits::ePartiallyBound;
    return result;
}
namespace rh::engine
{
vk::DescriptorSetLayoutBinding Convert( const DescriptorBinding &desc )
{
    vk::DescriptorSetLayoutBinding vk_desc_binding{};
    vk_desc_binding.binding         = desc.mBindingId;
    vk_desc_binding.descriptorCount = desc.mCount;
    vk_desc_binding.descriptorType  = Convert( desc.mDescriptorType );
    vk_desc_binding.stageFlags      = ConvertShaderStage( desc.mShaderStages );

    return vk_desc_binding;
}
} // namespace rh::engine

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(
    const VulkanDescriptorSetLayoutCreateInfo &create_info )
    : mDevice( create_info.mDevice )
{
    std::vector<vk::DescriptorSetLayoutBinding> layout_bindings{};
    layout_bindings.reserve( create_info.mBindingList.Size() );

    std::ranges::transform(
        create_info.mBindingList, std::back_inserter( layout_bindings ),
        []( const DescriptorBinding &binding ) { return Convert( binding ); } );

    std::vector<vk::DescriptorBindingFlags> flags;
    for ( const auto &b : create_info.mBindingList )
        flags.push_back( ConvertDescriptorFlags( b.mFlags ) );

    vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo{};
    bindingFlagsCreateInfo.bindingCount =
        static_cast<uint32_t>( layout_bindings.size() );
    bindingFlagsCreateInfo.pBindingFlags = flags.data();

    vk::DescriptorSetLayoutCreateInfo create_info_impl{};
    create_info_impl.pNext = &bindingFlagsCreateInfo;
    create_info_impl.bindingCount =
        static_cast<uint32_t>( layout_bindings.size() );
    create_info_impl.pBindings = layout_bindings.data();
    auto result = mDevice.createDescriptorSetLayout( create_info_impl );
    if ( !CALL_VK_API( result.result,
                       TEXT( "Failed to create descriptor set layout!" ) ) )
        return;
    mDescSetLayoutImpl = result.value;
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
{
    mDevice.destroyDescriptorSetLayout( mDescSetLayoutImpl );
}
