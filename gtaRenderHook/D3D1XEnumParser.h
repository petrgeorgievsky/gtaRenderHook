#pragma once
class CD3D1XEnumParser
{
public:
	static D3D11_PRIMITIVE_TOPOLOGY ConvertPrimTopology	(int prim);
	static void						ConvertRasterFormat(RwRaster* raster, RwUInt32 flags);
private:
	static const D3D11_PRIMITIVE_TOPOLOGY m_primConvertTable[];
};

