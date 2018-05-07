#include "GameMath.hlsl"
#include "GBuffer.hlsl"
#include "VoxelizingHelper.hlsl"
//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register( t0 );
Texture2D txSpec : register(t1);
SamplerState samLinear : register( s0 );

// Constant buffer for bone matrices
cbuffer SkinningInfoBuffer : register( b3 )
{
	float4x3 maConstBoneWorld[64];
};
struct VS_SKIN_IN
{
	float4 vPosition   	: POSITION;
	float2 vTexCoord     	: TEXCOORD;
	float3 vInNormal    	: NORMAL;
	float4 vInColor 		: COLOR;
	float4 vWeights			: WEIGHTS;
	uint4  aBones			: BONES;
#if FEATURE_LEVEL >= 0xb000
	uint   uVertexID      	: SV_VERTEXID;
#endif
};
struct PS_SKIN_IN
{
	float4 vColor		: COLOR;
	float4 vWorldPos	: WPOS;
	float3 vNormal   	: NORMAL;
	float2 vTexCoord  	: TEXCOORD0;
	float4 vPosition 	: SV_POSITION;
};
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_DEFERRED_IN VS(VS_SKIN_IN i)
{
    PS_DEFERRED_IN vsout;
	uint magic0=3;
	int magic1=0;
	float4 vPos     = float4(i.vPosition.xyz,1.0f);
	float4 OutPos   = float4(0.0f,0.0f,0.0f,0.0f);//transform to screen space
	
	float4x3 BoneToLocal =  maConstBoneWorld[i.aBones.x] * i.vWeights.x +
                            maConstBoneWorld[i.aBones.y] * i.vWeights.y + 
                            maConstBoneWorld[i.aBones.z] * i.vWeights.z + 
                            maConstBoneWorld[i.aBones.w] * i.vWeights.w;

    OutPos.xyz = mul(vPos, BoneToLocal).xyz;
    OutPos.w = 1.0f;
	//outPos = mul(outPos, World);
    float4 PositionWS = mul(OutPos, mWorld);
	
    OutPos = mul(PositionWS, mView);
    vsout.vNormalDepth = float4(mul(mWorldInv, float4(mul(i.vInNormal, (float3x3) BoneToLocal), 0.0f)).xyz, OutPos.z);
	vsout.vTexCoord     = float4(i.vTexCoord,0,0);
	vsout.vColor        = i.vInColor;
    OutPos = mul(OutPos, mProjection);
    vsout.vPosition = OutPos;
	
	return vsout;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_DEFERRED_IN i) : SV_Target
{
	float4 outColor=txDiffuse.Sample( samLinear, i.vTexCoord.xy);
	outColor.xyz*=(max(dot(i.vNormalDepth.xyz,float3(0,0,1.0f)),0.0f));
	//outColor.a*=Col.w;
	return outColor;
}
void ShadowPS(PS_DEFERRED_IN i)
{
	// empty because we render in depth buffer only
}
void VoxelPS(PS_DEFERRED_IN i)
{

}
void VoxelEmmissivePS(PS_DEFERRED_IN i)
{
	
}
PS_DEFERRED_OUT DeferredPS(PS_DEFERRED_IN i)
{
    PS_DEFERRED_OUT Out;
    float4 BaseColor = txDiffuse.Sample(samLinear, i.vTexCoord.xy);
    BaseColor.a = BaseColor.a > 0.9f ? BaseColor.a : InterleavedGradientNoise(i.vPosition.xy) * BaseColor.a;
    float4 params = bHasSpecTex != 0 ? txSpec.Sample(samLinear, i.vTexCoord.xy) : float4(fSpecularIntensity, fGlossiness, 0.2f, 2);
    if (BaseColor.a < 0.3)
        discard;
    FillGBuffer(Out, BaseColor, (i.vNormalDepth.xyz), i.vNormalDepth.w, params);
	return Out;
}