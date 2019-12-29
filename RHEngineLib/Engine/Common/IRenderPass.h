#pragma once
#include "Engine/Common/types/attachment_load_op.h"
#include "Engine/Common/types/attachment_store_op.h"
#include "Engine/Common/types/image_buffer_format.h"
#include "Engine/Common/types/image_layout.h"
#include "Engine/Common/types/pipeline_bind_point.h"
#include <optional>
#include <vector>

namespace rh::engine
{

struct AttachmentDescription
{
    ImageBufferFormat mFormat;
    LoadOp            mLoadOp;
    StoreOp           mStoreOp;
    LoadOp            mStencilLoadOp;
    StoreOp           mStencilStoreOp;
    ImageLayout       mSrcLayout;
    ImageLayout       mDestLayout;
};

struct AttachmentRef
{
    ImageLayout mReqLayout;
    uint32_t    mAttachmentId;
};

struct SubpassDescription
{
    PipelineBindPoint          mBindPoint;
    std::vector<AttachmentRef> mInputAttachments;
    // std::vector<AttachmentRef>   mResolveAttachments;
    std::vector<AttachmentRef>   mColorAttachments;
    std::optional<AttachmentRef> mDepthStencilAttachment;
};

struct RenderPassCreateParams
{
    std::vector<AttachmentDescription> mAttachments;
    std::vector<SubpassDescription>    mSubpasses;
};

class IRenderPass
{
  public:
    virtual ~IRenderPass() = default;
};

} // namespace rh::engine
