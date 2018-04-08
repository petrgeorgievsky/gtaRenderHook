#include "Globals.hlsl"
#ifndef GBUFFER
#define GBUFFER
// Encoding 32 bit float to 16bit RG chanell.
inline float2 EncodeFloatRG(float v)
{
	float2 kEncodeMul = float2(1.0, 255.0);
	float kEncodeBit = 1.0 / 255.0;
	float2 enc = kEncodeMul * v;
	enc = frac(enc);
	enc.x -= enc.y * kEncodeBit;
	return enc;
}
// Decoding 32bit float from 16bit RG chanell.
inline float DecodeFloatRG(float2 enc)
{
	float2 kDecodeDot = float2(1.0, 1 / 255.0);
    float res = dot(enc, kDecodeDot);
    //res = res * 2 - 1;
    float fNearClip = 0.1;
    float c1 = fFarClip / fNearClip;
    float c0 = 1.0 - c1;
    float linearDepth = (1.0 / res - c1)/c0;

    return res * fFarClip;
}
// Encoding normals.
inline float2 EncodeNormals(float2 v)
{
	return v*0.5+0.5;
}
// Decoding normals.
inline float3 DecodeNormals(float2 v)
{
	float3 n;
	n.xy = v * 2 - 1;
	n.z = sqrt(1.01 - dot(n.xy, n.xy));// added 0.01 to avoid z-fighting(or sqrt(0) to be more precise)
	return n;
}

/*!
    Encodes material type to float in 0..1 range.
*/
inline float ConvertFromMatType(float p)
{
    return p / 255.0;
}
/*!
    Decodes material type from float. Simply restores it to 0..255 range.
*/
inline uint ConvertToMatType(float p)
{
    return (255 * p);
}

// Deferred output structure.
struct DEFERRED_OUTPUT
{
	float4 vColor				: SV_Target0; // color rgb, roughness a
	float4 vNormalDepth			: SV_Target1; // normals - xy and depth - zw
	float4 vParameters			: SV_Target2; // x - spec intensity, y - glossiness, 
                                              // z - custom parameter, w - material id
                                              // for now material id-s is following 0 - simple object, 1 - car object, 2 - ped/skin object
                                              // custom param for simple object - metallness or detail tex id or something else?
                                              // custom param for car - metallness or second layer glossiness ?
                                              // custom param for ped/skin - translucency
};
struct DEFERRED_INPUT
{
	float4 vPosition		: SV_POSITION;
	float4 vColor			: COLOR;
	float4 vNormalDepth		: NORMAL;
	float4 vTexCoord		: TEXCOORD0;
};

void FillGBuffer(out DEFERRED_OUTPUT output, float4 Color, float3 Normals, float ViewZ, float4 Params)
{
	output.vColor = Color;
    float fNearClip = 0.1;
    float c1 = fFarClip / fNearClip;
    float c0 = 1.0 - c1;
    float linearDepth = ViewZ / fFarClip; // 1.0 / (c0 * ViewZ + c1);
    output.vNormalDepth = float4(EncodeNormals(Normals.xy), EncodeFloatRG(linearDepth));
	output.vParameters = Params;
    output.vParameters.w = ConvertFromMatType(output.vParameters.w); // compress material id, to prevent information loss
}

#endif