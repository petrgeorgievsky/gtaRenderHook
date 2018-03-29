#include "Globals.hlsl"
#ifndef GAMEMATH
#define GAMEMATH
// Shitty dx9 alpha test replacement.
#define DO_ALPHA_TEST(alpha) if(uiAlphaTestType==1){\
		if(outColor.a<=fAlphaTestRef) discard;\
	}else if(uiAlphaTestType==2){\
		if(outColor.a<fAlphaTestRef) discard;\
	}else if(uiAlphaTestType==3){\
		if(outColor.a>=fAlphaTestRef) discard;\
	}else if(uiAlphaTestType==4){\
		if(outColor.a>fAlphaTestRef) discard;\
	}else if(uiAlphaTestType==5){\
		if(outColor.a==fAlphaTestRef) discard;\
	}else if(uiAlphaTestType==6){\
		if(outColor.a!=fAlphaTestRef) discard;\
	}else if(uiAlphaTestType==0){\
		outColor.a=1;\
	}
// Converts from world space to voxel space. TODO: use matrices
float3 ConvertToVoxelSpace(float3 p)
{
    float3 op = ((p - ViewInv[3].xyz) * voxelGridScale + float3(voxelGridSize, voxelGridSize, voxelGridSize) / (2));
    return op;
}
// Converts from voxel space to world space. TODO: use matrices
float3 ConvertFromVoxelSpace(float3 p)
{
    float3 op = ((p - float3(voxelGridSize, voxelGridSize, voxelGridSize) / (2)) / voxelGridScale) + ViewInv[3].xyz;
    return op;
} 
// Converts from world space to voxel space. TODO: use matrices
float3 ConvertToVoxelSpace(float3 p,float scale)
{
    float3 op = ((p - ViewInv[3].xyz) * scale + float3(voxelGridSize, voxelGridSize, voxelGridSize) / (2));
    return op;
}
// Converts from voxel space to world space. TODO: use matrices
float3 ConvertFromVoxelSpace(float3 p, float scale)
{
    float3 op = ((p - float3(voxelGridSize, voxelGridSize, voxelGridSize) / (2)) / scale) + ViewInv[3].xyz;
    return op;
}
// Converts uint to rgba.
float4 UINTtoRGBA(uint color) {
	if (color == 0xffffffff) return float4(1, 1, 1, 1);

	float a = ((color & 0xff000000) >> 24);

	float r = ((color & 0xff0000) >> 16);

	float g = ((color & 0xff00) >> 8);

	float b = ((color & 0xff));
	return float4(r, g, b, a)/255.0f;
}

float3 CosineSampleHemisphere(float u, float v)
{
	float phi = v * 2.0 * 3.14;
	float cosTheta = sqrt(1.0 - u);
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	return float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

float4 viewPosFromDepth(float depth, float2 texCoord)
{
    float x = texCoord.x * 2 - 1;
    float y = (1 - texCoord.y) * 2 - 1;
    float2 screenSpaceRay = float2(x / Projection[0].x, y / Projection[1].y);
    float4 pos = float4(screenSpaceRay * depth, depth, 1.0);
    return pos;
}

float3 posFromDepth(float depth, float2 texCoord)
{
    float4 pos = viewPosFromDepth(depth, texCoord);
    pos = mul(pos, ViewInv);
    return pos.xyz;
}

#endif