#include "Globals.hlsl"
#ifndef GBUFFER
#define GBUFFER

/*!
    Encoding 32 bit float to 16bit RG chanell.
*/
inline float2 EncodeFloatRG(float v)
{
	float2 kEncodeMul = float2(1.0, 255.0);
	float kEncodeBit = 1.0 / 255.0;
	float2 enc = kEncodeMul * v;
	enc = frac(enc);
	enc.x -= enc.y * kEncodeBit;
	return enc;
}

/*!
    Decoding 32bit float from 16bit RG chanell.
*/
inline float DecodeFloatRG(float2 enc)
{
	float2 kDecodeDot = float2(1.0, 1 / 255.0);
    float res = dot(enc, kDecodeDot);
    return res * fFarClip;
}
/*!
    Encodes normals.
*/
inline float2 EncodeNormals(float2 v)
{
	return v*0.5+0.5;
}
/*!
    Encodes normals via SM.
*/
inline float2 EncodeNormalsSphereMap(float3 v)
{
    float p = sqrt(v.z * 8 + 8);
    return v.xy / p + 0.5f;
}
/*!
    Decodes normals.
*/
inline float3 DecodeNormals(float2 v)
{
	float3 n;
	n.xy = v * 2 - 1;
	n.z = -sqrt(max(1.01 - dot(n.xy, n.xy), 0.0f));// added 0.01 to avoid z-fighting(or sqrt(0) to be more precise)
	return n;
}
/*!
    Decodes normals via SM.
*/
inline float3 DecodeNormalsSphereMap(float2 v)
{
    float2 fenc = v * 4 - 2;
    float f = dot(fenc, fenc);
    float g = sqrt(1 - f / 4);
    float3 n;
    n.xy = fenc * g;
    n.z = 1 - f / 2;
    return n;
}
/*!
    Encodes material type to float in 0..1 range.
*/
inline float ConvertFromMatType(float p)
{
    return p / 255.0f;
}
/*!
    Decodes material type from float. Simply restores it to 0..255 range.
*/
inline uint ConvertToMatType(float p)
{
    return (255 * p);
}

/*!
    Deferred output structure.
*/
struct PS_DEFERRED_OUT
{
	float4 vColor				: SV_Target0; // color rgb, roughness a
	float4 vNormalDepth			: SV_Target1; // normals - xy and depth - zw
	float4 vParameters			: SV_Target2; // x - spec intensity, y - glossiness, 
                                              // z - custom parameter, w - material id
                                              // for now material id-s is following 0 - simple object, 1 - car object, 2 - ped/skin object, 3 - vertexlit object
                                              // custom param for simple object - metallness or detail tex id or something else?
                                              // custom param for car - metallness or second layer glossiness ?
                                              // custom param for ped/skin - translucency
    float4 vLighting            : SV_Target3;
};
/*!
    Deferred input structure.
*/
struct PS_DEFERRED_IN
{
	float4 vPosition		: SV_POSITION;
	float4 vColor			: COLOR;
	float4 vNormalDepth		: NORMAL;
    float4 vTexCoord : TEXCOORD0;
    float4 vTangent : TEXCOORD1;
    float4 vBiTangent : TEXCOORD2;
};
/*!
    Deferred output structure.
*/
void FillGBuffer(out PS_DEFERRED_OUT output, float4 Color, float3 Normals, float ViewZ, float4 Params)
{
    // transform normals to view-space
    Normals = length(Normals) <= 0.0f ? 0.0f.xxx : mul(normalize(Normals), (float3x3) mView);
	output.vColor = Color;
    float2 EncNormals = EncodeNormals(Normals.xy);
    output.vNormalDepth = float4(isnan(EncNormals) ? 0.0f.xx : EncNormals, EncodeFloatRG(ViewZ / fFarClip));
	output.vParameters = Params;
    output.vParameters.w = ConvertFromMatType(output.vParameters.w); // compress material id, to prevent information loss
    float3 AmbLighting = lerp(vSkyLightCol.rgb, vHorizonCol.rgb, saturate(Normals.z));
    output.vLighting     = float4( AmbLighting * 0.25f, 1 );
}

/*!
    Deferred output structure.
*/
void FillGBufferVertexRadiance(out PS_DEFERRED_OUT output, float4 Color, float3 Normals, float ViewZ, float4 Params, float4 lighting)
{
    // transform normals to view-space
    Normals = isnan(Normals) ? 0.0f.xxx : mul(normalize(Normals), (float3x3) mView);
    output.vColor = Color;
    float2 EncNormals = EncodeNormals(Normals.xy);
    output.vNormalDepth = float4(isnan(EncNormals) ? 0.0f.xx : EncNormals, EncodeFloatRG(ViewZ / fFarClip));
    output.vParameters = Params;
    output.vParameters.w = ConvertFromMatType(output.vParameters.w); // compress material id, to prevent information loss
    output.vLighting = float4(isnan(lighting.rgb) ? 0.0f.xxx : lighting.rgb, 1);
}
/*!
    Returns view-space depth and world-space normals from GBuffer
*/
void GetNormalsAndDepth(Texture2D TexBuffer, SamplerState Sampler, float2 TexCoords, out float ViewDepth, out float3 Normals)
{
    float4 NormalSpec = TexBuffer.SampleLevel(Sampler, TexCoords, 0);

    ViewDepth = DecodeFloatRG(NormalSpec.zw);
    ViewDepth = ViewDepth <= 0 ? fFarClip : ViewDepth;
    Normals = DecodeNormals(NormalSpec.xy);
    // transform normals back to world-space
    Normals = mul(Normals, (float3x3) mViewInv);
}

/*!
    Returns view-space depth from GBuffer
*/
void GetDepth(Texture2D TexBuffer, SamplerState Sampler, float2 TexCoords, out float ViewDepth)
{
    float4 NormalSpec = TexBuffer.SampleLevel(Sampler, TexCoords, 0);

    ViewDepth = DecodeFloatRG(NormalSpec.zw);
    ViewDepth = ViewDepth <= 0 ? fFarClip : ViewDepth;
}

/*!
    Returns average view-space depth from GBuffer at specific texcoord
*/
void GetAvgDepth(Texture2D TexBuffer, SamplerState Sampler, float2 TexCoords, out float AvgDepth)
{
    float2 Offset = float2(1.0f / fScreenWidth, 1.0f / fScreenHeight);
    float2 NeighborPattern[8] = { float2(0, 1), float2(1, 0), float2(0, -1), float2(-1, 0), float2(-1, -1), float2(1, -1), float2(-1, 1), float2(1, 1) };
    float ViewDepth;
    float4 NormalSpec = TexBuffer.SampleLevel(Sampler, TexCoords, 0);

    ViewDepth = DecodeFloatRG(NormalSpec.zw);
    ViewDepth = ViewDepth <= 0 ? fFarClip : ViewDepth;
    AvgDepth = ViewDepth;
    for (int k; k < 8; k++)
    {
        float4 NormalSpec = TexBuffer.SampleLevel(Sampler, TexCoords + NeighborPattern[k] * Offset, 0);

        ViewDepth = DecodeFloatRG(NormalSpec.zw);
        ViewDepth = ViewDepth <= 0 ? fFarClip : ViewDepth;
        AvgDepth += ViewDepth;
    }
    AvgDepth /= 9.0f;
}
/*!
    Returns difference between max and min view-space depth from GBuffer at specific texcoord
*/
void GetNeighborDepthDiff(Texture2D TexBuffer, SamplerState Sampler, float2 TexCoords, out float DepthDiff)
{
    float2 Offset = float2(1.0f / fScreenWidth, 1.0f / fScreenHeight);
    float2 NeighborPattern[8] = { float2(0, 1), float2(1, 0), float2(0, -1), float2(-1, 0), float2(-1, -1), float2(1, -1), float2(-1, 1), float2(1, 1) };
    float ViewDepth;
    float MaxDepth, MinDepth;
    float4 NormalSpec = TexBuffer.SampleLevel(Sampler, TexCoords, 0);

    ViewDepth = DecodeFloatRG(NormalSpec.zw);
    ViewDepth = ViewDepth <= 0 ? fFarClip : ViewDepth;
    MinDepth = ViewDepth;
    MaxDepth = ViewDepth;
    for (int k; k < 8; k++)
    {
        float4 NormalSpec = TexBuffer.SampleLevel(Sampler, TexCoords + NeighborPattern[k] * Offset, 0);

        ViewDepth = DecodeFloatRG(NormalSpec.zw);
        ViewDepth = ViewDepth <= 0 ? fFarClip : ViewDepth;
        MinDepth = min(ViewDepth, MinDepth);
        MaxDepth = max(ViewDepth, MaxDepth);
    }
    DepthDiff = MaxDepth - MinDepth;
}
#endif