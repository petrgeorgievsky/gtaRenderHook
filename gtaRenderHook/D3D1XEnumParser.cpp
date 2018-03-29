#include "stdafx.h"
#include "D3D1XEnumParser.h"

const D3D11_PRIMITIVE_TOPOLOGY CD3D1XEnumParser::m_primConvertTable[]{
	D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED,
	D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
	D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,
	D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
	D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
	D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
	D3D11_PRIMITIVE_TOPOLOGY_POINTLIST
};
D3D11_PRIMITIVE_TOPOLOGY CD3D1XEnumParser::ConvertPrimTopology(int prim)
{
	return m_primConvertTable[prim];
}

void CD3D1XEnumParser::ConvertRasterFormat(RwRaster* raster, RwUInt32 flags)
{
	RwD3D1XRaster* d3dRaster = GetD3D1XRaster(raster);
	RwUInt32 rasterPixelFmt = flags & rwRASTERFORMATPIXELFORMATMASK;

	if (raster->cType == rwRASTERTYPECAMERATEXTURE)
	{
		if (rasterPixelFmt == rwRASTERFORMATDEFAULT)
			d3dRaster->format = DXGI_FORMAT_R8G8B8A8_UNORM;
		else if (rasterPixelFmt == rwRASTERFORMAT1555)//rgba16
			d3dRaster->format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		else if (rasterPixelFmt == rwRASTERFORMAT565)//rgba32
			d3dRaster->format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		else if (rasterPixelFmt == rwRASTERFORMAT16)//r16
			d3dRaster->format = DXGI_FORMAT_R16_FLOAT;
		else if (rasterPixelFmt == rwRASTERFORMAT555)//rg16
			d3dRaster->format = DXGI_FORMAT_R16G16_FLOAT;
		else if (rasterPixelFmt == rwRASTERFORMAT888)//3d texture 888
		{
			d3dRaster->format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			d3dRaster->textureFlags = 64;
		}
	}
	else if (flags&rwRASTERDONTALLOCATE)
	{
		if (rasterPixelFmt == rwRASTERFORMAT1555)//dxt1
		{
			d3dRaster->format = DXGI_FORMAT_BC1_UNORM;
		}
		else if (rasterPixelFmt == rwRASTERFORMAT565)//dxt2
		{
			d3dRaster->format = DXGI_FORMAT_BC1_UNORM;
		}
		else if (rasterPixelFmt == rwRASTERFORMAT4444)//dxt3
		{
			d3dRaster->format = DXGI_FORMAT_BC2_UNORM;
			d3dRaster->alpha = 1;
		}
		else if (rasterPixelFmt == rwRASTERFORMAT8888)//dxt5
		{
			d3dRaster->format = DXGI_FORMAT_BC3_UNORM;
			d3dRaster->alpha = 1;
		}
	}
	else {
		switch (rasterPixelFmt)
		{
		case rwRASTERFORMAT1555:
			d3dRaster->format = DXGI_FORMAT_B5G5R5A1_UNORM;
			d3dRaster->alpha = 1;
			break;
		case rwRASTERFORMAT4444:
			d3dRaster->format = DXGI_FORMAT_B4G4R4A4_UNORM;
			d3dRaster->alpha = 1;
			break;
		case rwRASTERFORMAT565:
			d3dRaster->format = DXGI_FORMAT_B5G6R5_UNORM;
			break;
		case rwRASTERFORMAT8888:
			d3dRaster->format = DXGI_FORMAT_R8G8B8A8_UNORM;
			d3dRaster->alpha = 1;
			break;
		case rwRASTERFORMAT888:
			d3dRaster->format = DXGI_FORMAT_B8G8R8X8_UNORM;
			break;
		case rwRASTERFORMAT16:
			d3dRaster->format = DXGI_FORMAT_D16_UNORM;
			break;
		case rwRASTERFORMAT24:
			d3dRaster->format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			break;
		case rwRASTERFORMAT32:
			d3dRaster->format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			break;
		default: ;
		}
	}
	if (raster->cType == rwRASTERTYPEZBUFFER)
	{
		if (rasterPixelFmt == rwRASTERFORMATDEFAULT)
			d3dRaster->format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	}
}