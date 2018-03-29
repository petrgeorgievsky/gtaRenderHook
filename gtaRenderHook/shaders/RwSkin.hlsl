#include "GameMath.hlsl"
#include "GBuffer.hlsl"
#include "VoxelizingHelper.hlsl"
//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register( t0 );
SamplerState samLinear : register( s0 );

// Constant buffer for bone matrices
cbuffer SkinningInfoBuffer : register( b3 )
{
	float4x3 g_mConstBoneWorld[64];
};
struct VS_INPUT
{
	float4 inPosition   	: POSITION;
	float2 inTexCoord     	: TEXCOORD;
	float3 vInNormal    	: NORMAL;
	float4 vInColor 		: COLOR;
	float4 Weights			: WEIGHTS;
	uint4  Bones			: BONES;
#if FEATURE_LEVEL >= 0xb000
	uint   uVertexID      	: SV_VERTEXID;
#endif
};
struct VS_OUTPUT
{
	float4 vColor		: COLOR;
	float4 vWorldPos	: WPOS;
	float3 vNormal   	: NORMAL;
	float2 texCoord  	: TEXCOORD0;
	float4 vPosition 	: SV_POSITION;
};
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
DEFERRED_INPUT VS(VS_INPUT i)
{
	DEFERRED_INPUT vsout;
	uint magic0=3;
	int magic1=0;
	float4 vPos=float4(i.inPosition.xyz,1.0f);
	float4 outPos=float4(0.0f,0.0f,0.0f,0.0f);//transform to screen space
	
	float4x3 BoneToLocal = g_mConstBoneWorld[i.Bones.x] * i.Weights.x + g_mConstBoneWorld[i.Bones.y] * i.Weights.y + g_mConstBoneWorld[i.Bones.z] * i.Weights.z + g_mConstBoneWorld[i.Bones.w] * i.Weights.w;

	outPos.xyz = mul(vPos,BoneToLocal).xyz;
	outPos.w = 1.0f;
	//outPos = mul(outPos, World);
	float4 vPositionWS = mul(outPos, World);
	
    outPos = mul(vPositionWS, View);
    vsout.vNormalDepth = float4(mul(float4(mul(i.vInNormal, (float3x3) BoneToLocal), 0.0f), World).xyz, outPos.z);
	vsout.vTexCoord =float4(i.inTexCoord,0,0);
	vsout.vColor = i.vInColor;
	outPos = mul(outPos, Projection);
	vsout.vPosition=outPos;
	
	return vsout;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(DEFERRED_INPUT i) : SV_Target
{
	float4 outColor=txDiffuse.Sample( samLinear, i.vTexCoord.xy);
	outColor.xyz*=(max(dot(i.vNormalDepth.xyz,float3(0,0,1.0f)),0.0f));
	//outColor.a*=Col.w;
	return outColor;
}
void ShadowPS(DEFERRED_INPUT i)
{
	// empty because we render in depth buffer only
}
void VoxelPS(DEFERRED_INPUT i)
{

}
void VoxelEmmissivePS(DEFERRED_INPUT i)
{
	
}
DEFERRED_OUTPUT DeferredPS(DEFERRED_INPUT i)
{
	DEFERRED_OUTPUT Out;
	FillGBuffer(Out, txDiffuse.Sample(samLinear, i.vTexCoord.xy), (i.vNormalDepth.xyz), i.vNormalDepth.w, float2(0.1f, 0.3f));
	return Out;
}