//
// Created by peter on 17.02.2021.
//

#include "raster_pool.h"
#include <Engine/Common/IImageBuffer.h>
#include <Engine/Common/IImageView.h>

namespace rh::rw::engine
{

void RasterData::Release()
{
    delete mImageView;
    delete mImageBuffer;
    mImageView   = nullptr;
    mImageBuffer = nullptr;
}

} // namespace rh::rw::engine