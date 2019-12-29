#pragma once
struct RwRaster;
struct RwImage;

namespace rh::rw::engine
{

class RwRasterSetImageCmd
{
public:
    RwRasterSetImageCmd(RwRaster *raster, RwImage *image);
    bool Execute();
private:
    RwRaster* m_pRaster = nullptr;
    RwImage *m_pImage = nullptr;
};

}
