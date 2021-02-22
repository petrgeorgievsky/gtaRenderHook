#include "VulkanPipelineLayout.h"
#include "VulkanCommon.h"

rh::engine::VulkanPipelineLayout::VulkanPipelineLayout(
    const VulkanPipelineLayoutCreateInfo &create_info )
    : mDevice( create_info.mDevice )
{
    std::vector<vk::DescriptorSetLayout> desc_set_layouts_impl{};
    desc_set_layouts_impl.reserve( create_info.mDescriptorSetLayouts.Size() );

    std::ranges::transform( create_info.mDescriptorSetLayouts,
                            std::back_inserter( desc_set_layouts_impl ),
                            []( IDescriptorSetLayout *layout ) {
                                assert( layout != nullptr );
                                return static_cast<VulkanDescriptorSetLayout *>(
                                           layout )
                                    ->GetDescSet();
                            } );

    vk::PipelineLayoutCreateInfo create_info_impl{};
    create_info_impl.pSetLayouts = desc_set_layouts_impl.data();
    create_info_impl.setLayoutCount =
        static_cast<uint32_t>( desc_set_layouts_impl.size() );

    auto result = mDevice.createPipelineLayout( create_info_impl );
    CALL_VK_API( result.result, TEXT( "Failed to submit gpu work!" ) );
    mPipelineLayoutImpl = result.value;
}

rh::engine::VulkanPipelineLayout::~VulkanPipelineLayout()
{
    mDevice.destroyPipelineLayout( mPipelineLayoutImpl );
}
