#include "VulkanRenderPass.h"
#include "VulkanConvert.h"
using namespace rh::engine;
namespace rh::engine
{
vk::AttachmentDescription Convert( const AttachmentDescription &desc )
{
    vk::AttachmentDescription vk_desc{};
    vk_desc.format = Convert( desc.mFormat );
    // TODO: add to desc params?
    vk_desc.samples        = vk::SampleCountFlagBits::e1;
    vk_desc.loadOp         = Convert( desc.mLoadOp );
    vk_desc.storeOp        = Convert( desc.mStoreOp );
    vk_desc.stencilLoadOp  = Convert( desc.mStencilLoadOp );
    vk_desc.stencilStoreOp = Convert( desc.mStencilStoreOp );
    vk_desc.initialLayout  = Convert( desc.mSrcLayout );
    vk_desc.finalLayout    = Convert( desc.mDestLayout );

    return vk_desc;
}

vk::AttachmentReference Convert( const AttachmentRef &ref )
{
    vk::AttachmentReference vk_ref{};
    vk_ref.layout     = Convert( ref.mReqLayout );
    vk_ref.attachment = ref.mAttachmentId;
    return vk_ref;
}

std::vector<vk::AttachmentReference>
Convert( const std::vector<AttachmentRef> &ref_list )
{
    using namespace std;
    vector<vk::AttachmentReference> vk_attachment_refs;
    vk_attachment_refs.reserve( ref_list.size() );

    ranges::transform(
        ref_list, back_inserter( vk_attachment_refs ),
        []( const AttachmentRef &desc ) -> vk::AttachmentReference {
            return Convert( desc );
        } );
    return vk_attachment_refs;
}

// Required for temporary memory allocation
struct cppVkSubpassDesc
{
    std::vector<vk::AttachmentReference>   mColorAttachments;
    std::vector<vk::AttachmentReference>   mInputAttachments;
    std::optional<vk::AttachmentReference> mDepthStencilAttachment;
};

vk::SubpassDescription Convert( const SubpassDescription &desc,
                                cppVkSubpassDesc &        vk_desc_cpp )
{
    vk::SubpassDescription vk_desc{};
    vk_desc.pipelineBindPoint = Convert( desc.mBindPoint );

    vk_desc_cpp.mColorAttachments = Convert( desc.mColorAttachments );
    if ( !vk_desc_cpp.mColorAttachments.empty() )
    {
        vk_desc.colorAttachmentCount = vk_desc_cpp.mColorAttachments.size();
        vk_desc.pColorAttachments    = vk_desc_cpp.mColorAttachments.data();
    }
    vk_desc_cpp.mInputAttachments = Convert( desc.mInputAttachments );
    if ( !vk_desc_cpp.mInputAttachments.empty() )
    {
        vk_desc.inputAttachmentCount = vk_desc_cpp.mInputAttachments.size();
        vk_desc.pInputAttachments    = vk_desc_cpp.mInputAttachments.data();
    }

    if ( desc.mDepthStencilAttachment.has_value() )
    {
        vk_desc_cpp.mDepthStencilAttachment =
            Convert( desc.mDepthStencilAttachment.value() );
        vk_desc.pDepthStencilAttachment =
            &vk_desc_cpp.mDepthStencilAttachment.value();
    }
    return vk_desc;
}
} // namespace rh::engine

VulkanRenderPass::VulkanRenderPass(
    const VulkanRenderPassCreateInfo &create_info )
    : m_vkDevice( create_info.mDevice )
{
    using namespace std;
    vk::RenderPassCreateInfo vk_create_info{};

    vector<vk::AttachmentDescription> vk_attachment_descs;
    vk_attachment_descs.reserve( create_info.mDesc.mAttachments.size() );

    ranges::transform(
        create_info.mDesc.mAttachments, back_inserter( vk_attachment_descs ),
        []( const AttachmentDescription &desc ) -> vk::AttachmentDescription {
            return Convert( desc );
        } );

    vector<vk::SubpassDescription> subpass_desc_list;
    vector<cppVkSubpassDesc>       subpass_desc_list_temp;

    for ( const auto &subpass_desc : create_info.mDesc.mSubpasses )
    {
        cppVkSubpassDesc desc{};
        subpass_desc_list_temp.push_back( desc );
        subpass_desc_list.push_back(
            Convert( subpass_desc, subpass_desc_list_temp.back() ) );
    }

    vk_create_info.pAttachments    = vk_attachment_descs.data();
    vk_create_info.attachmentCount = vk_attachment_descs.size();
    vk_create_info.pSubpasses      = subpass_desc_list.data();
    vk_create_info.subpassCount    = subpass_desc_list.size();

    m_vkRenderPass = m_vkDevice.createRenderPass( vk_create_info );
}

VulkanRenderPass::~VulkanRenderPass()
{
    m_vkDevice.destroyRenderPass( m_vkRenderPass );
}

VulkanRenderPass::operator vk::RenderPass() { return m_vkRenderPass; }