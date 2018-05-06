// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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
D3D11_PRIMITIVE_TOPOLOGY CD3D1XEnumParser::ConvertPrimTopology(RwPrimitiveType prim)
{
	return m_primConvertTable[prim];
}

RwPrimitiveType CD3D1XEnumParser::ConvertPrimTopology(RpMeshHeaderFlags flags)
{
	switch (flags)
	{
	case rpMESHHEADERTRISTRIP:
		return rwPRIMTYPETRISTRIP;
	case rpMESHHEADERTRIFAN:
		return rwPRIMTYPETRIFAN;
	case rpMESHHEADERLINELIST:
		return rwPRIMTYPELINELIST;
	case rpMESHHEADERPOLYLINE:
		return rwPRIMTYPEPOLYLINE;
	case rpMESHHEADERPOINTLIST:
		return rwPRIMTYPEPOINTLIST;
	default:
		return rwPRIMTYPETRILIST;
	}
}

D3D11_BLEND CD3D1XEnumParser::ConvertBlendFunc(RwBlendFunction func)
{
	switch (func)
	{
	case rwBLENDZERO:			return D3D11_BLEND_ZERO;
	case rwBLENDONE:			return D3D11_BLEND_ONE;
	case rwBLENDSRCCOLOR:		return D3D11_BLEND_SRC_COLOR;
	case rwBLENDINVSRCCOLOR:	return D3D11_BLEND_INV_SRC_COLOR;
	case rwBLENDSRCALPHA:		return D3D11_BLEND_SRC_ALPHA;
	case rwBLENDINVSRCALPHA:	return D3D11_BLEND_INV_SRC_ALPHA;
	case rwBLENDDESTALPHA:		return D3D11_BLEND_DEST_ALPHA;
	case rwBLENDINVDESTALPHA:	return D3D11_BLEND_INV_DEST_ALPHA;
	case rwBLENDDESTCOLOR:		return D3D11_BLEND_DEST_COLOR;
	case rwBLENDINVDESTCOLOR:	return D3D11_BLEND_INV_DEST_COLOR;
	case rwBLENDSRCALPHASAT:	return D3D11_BLEND_SRC_ALPHA_SAT;
	default:					return D3D11_BLEND_INV_SRC_ALPHA;
	}
}

RwBlendFunction CD3D1XEnumParser::ConvertBlendFunc(D3D11_BLEND func)
{
	switch (func)
	{
	case D3D11_BLEND_ZERO:				return rwBLENDZERO;
	case D3D11_BLEND_ONE:				return rwBLENDONE;
	case D3D11_BLEND_SRC_COLOR:			return rwBLENDSRCCOLOR;
	case D3D11_BLEND_INV_SRC_COLOR:		return rwBLENDINVSRCCOLOR;
	case D3D11_BLEND_SRC_ALPHA:			return rwBLENDSRCALPHA;
	case D3D11_BLEND_INV_SRC_ALPHA:		return rwBLENDINVSRCALPHA;
	case D3D11_BLEND_DEST_ALPHA:		return rwBLENDDESTALPHA;
	case D3D11_BLEND_INV_DEST_ALPHA:	return rwBLENDINVDESTALPHA;
	case D3D11_BLEND_DEST_COLOR:		return rwBLENDDESTCOLOR;
	case D3D11_BLEND_INV_DEST_COLOR:	return rwBLENDINVDESTCOLOR;
	case D3D11_BLEND_SRC_ALPHA_SAT:		return rwBLENDSRCALPHASAT;
	default:							return rwBLENDINVSRCALPHA;
	}
}

D3D11_STENCIL_OP CD3D1XEnumParser::ConvertStencilOp(RwStencilOperation op)
{
	switch (op)
	{
	case rwSTENCILOPERATIONZERO:		return D3D11_STENCIL_OP_ZERO;
	case rwSTENCILOPERATIONREPLACE:		return D3D11_STENCIL_OP_REPLACE;
	case rwSTENCILOPERATIONINCRSAT:		return D3D11_STENCIL_OP_INCR_SAT;
	case rwSTENCILOPERATIONDECRSAT:		return D3D11_STENCIL_OP_DECR_SAT;
	case rwSTENCILOPERATIONINVERT:		return D3D11_STENCIL_OP_INVERT;
	case rwSTENCILOPERATIONINCR:		return D3D11_STENCIL_OP_INCR;
	case rwSTENCILOPERATIONDECR:		return D3D11_STENCIL_OP_DECR;
	default:							return D3D11_STENCIL_OP_KEEP;
	}
}

D3D11_COMPARISON_FUNC CD3D1XEnumParser::ConvertStencilFunc(RwStencilFunction func)
{
	switch (func)
	{
	case rwSTENCILFUNCTIONNEVER:		return D3D11_COMPARISON_NEVER;
	case rwSTENCILFUNCTIONLESS:			return D3D11_COMPARISON_LESS;
	case rwSTENCILFUNCTIONEQUAL:		return D3D11_COMPARISON_EQUAL;
	case rwSTENCILFUNCTIONLESSEQUAL:	return D3D11_COMPARISON_LESS_EQUAL;
	case rwSTENCILFUNCTIONGREATER:		return D3D11_COMPARISON_GREATER;
	case rwSTENCILFUNCTIONNOTEQUAL:		return D3D11_COMPARISON_NOT_EQUAL;
	case rwSTENCILFUNCTIONGREATEREQUAL:	return D3D11_COMPARISON_GREATER_EQUAL;
	default:							return D3D11_COMPARISON_ALWAYS;
	}
}

D3D11_TEXTURE_ADDRESS_MODE CD3D1XEnumParser::ConvertTextureAddressMode(RwTextureAddressMode mode)
{
	switch (mode)
	{
	case rwTEXTUREADDRESSMIRROR:	return D3D11_TEXTURE_ADDRESS_MIRROR;
	case rwTEXTUREADDRESSCLAMP:		return D3D11_TEXTURE_ADDRESS_CLAMP;
	case rwTEXTUREADDRESSBORDER:	return D3D11_TEXTURE_ADDRESS_BORDER;
	default:						return D3D11_TEXTURE_ADDRESS_WRAP;
	}
}

RwTextureAddressMode CD3D1XEnumParser::ConvertTextureAddressMode(D3D11_TEXTURE_ADDRESS_MODE mode)
{
	switch (mode)
	{
	case D3D11_TEXTURE_ADDRESS_MIRROR:	return rwTEXTUREADDRESSMIRROR;
	case D3D11_TEXTURE_ADDRESS_CLAMP:	return rwTEXTUREADDRESSCLAMP;
	case D3D11_TEXTURE_ADDRESS_BORDER:	return rwTEXTUREADDRESSBORDER;
	default:							return rwTEXTUREADDRESSWRAP;
	}
}

D3D11_FILTER CD3D1XEnumParser::ConvertTextureFilterMode(RwTextureFilterMode mode)
{
	switch (mode)
	{
	case rwFILTERNEAREST:
	case rwFILTERMIPNEAREST:		return D3D11_FILTER_MIN_MAG_MIP_POINT;
	case rwFILTERLINEARMIPNEAREST:	return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	default:						return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	}
}

RwTextureFilterMode CD3D1XEnumParser::ConvertTextureFilterMode(D3D11_FILTER mode)
{
	switch (mode)
	{
	case D3D11_FILTER_MIN_MAG_MIP_POINT: 		return rwFILTERMIPNEAREST;
	case D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT:	return rwFILTERLINEARMIPNEAREST;
	default:									return rwFILTERLINEARMIPLINEAR;
	}
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
			d3dRaster->format = DXGI_FORMAT_B8G8R8A8_UNORM;
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

D3D11_CULL_MODE CD3D1XEnumParser::ConvertCullMode(RwCullMode mode)
{
	switch (mode)
	{
	case rwCULLMODECULLNONE: return D3D11_CULL_NONE;
	case rwCULLMODECULLBACK: return D3D11_CULL_BACK;
	case rwCULLMODECULLFRONT: return D3D11_CULL_FRONT;
	default:
		return D3D11_CULL_NONE;
	}
}

RwCullMode CD3D1XEnumParser::ConvertCullMode(D3D11_CULL_MODE mode)
{
	return RwCullMode();
}
