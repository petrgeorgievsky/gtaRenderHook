#include "GameMath.hlsl"
#include "LightingFunctions.hlsl"
#include "GBuffer.hlsl"
#include "VoxelizingHelper.hlsl"
//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register(t0);
Texture2D txSpec 	: register(t1);
Texture2D txShadow 	: register(t4);

SamplerState samLinear : register(s0);

struct VS_SEABED_IN
{
	float3 vPosition   	: POSITION;
	float2 vTexCoord     	: TEXCOORD;
	float3 vInNormal    	: NORMAL;
	float4 vInColor 		: COLOR;
};
struct GS_VOXEL_IN
{
	float4 vPosition		: SV_POSITION;    // World position
	float4 vNormal			: NORMAL;
	float2 vTexCoord		: TEXCOORD0;         // Texture coord
};
struct PS_VOXEL_INPUT
{
	float4 vPosition	: SV_POSITION;
	float4 vWorldPos	: WPOS;
	float4 vNormal		: NORMAL;
	float2 vTexCoord	: TEXCOORD0;
};
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_DEFERRED_IN VS(VS_SEABED_IN i)
{
    PS_DEFERRED_IN o;
	// Compute clip-space position.
	float4 	outPos = float4(i.vPosition, 1.0);
	outPos = mul(outPos, mWorld);
	outPos = mul(outPos, mView);

	o.vPosition = mul(outPos, mProjection);

	o.vNormalDepth = float4(mul(i.vInNormal, (float3x3)mWorld), outPos.z);
	o.vTexCoord = float4(i.vTexCoord, 0, 0);
	o.vColor = i.vInColor;

	return o;
}

GS_VOXEL_IN VoxelVS(VS_SEABED_IN i)
{
	GS_VOXEL_IN Out = (GS_VOXEL_IN)0.0f;

	Out.vPosition = mul(float4(i.vPosition, 1.0), mWorld);
	Out.vTexCoord = i.vTexCoord;
	Out.vNormal = float4(mul(i.vInNormal, (float3x3)mWorld), 1.0);
	return Out;
}

[maxvertexcount(18)]
void VoxelGS(triangle GS_VOXEL_IN input[3], inout TriangleStream<PS_VOXEL_INPUT> VoxelStream)
{
	for (int f = 0; f < 6; ++f)
	{
		// Compute screen coordinates
		PS_VOXEL_INPUT output;
		for (int v = 0; v < 3; v++)
		{
			output.vWorldPos = input[v].vPosition;
			output.vNormal = input[v].vNormal;
			output.vPosition = mul(input[v].vPosition, VoxelView[f]);
			output.vPosition = mul(output.vPosition, mProjection);
			output.vTexCoord = input[v].vTexCoord;
			VoxelStream.Append(output);
		}
		VoxelStream.RestartStrip();
	}
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_DEFERRED_IN i) : SV_Target
{
	float4 outColor = txDiffuse.Sample(samLinear, i.vTexCoord.xy) * cDiffuseColor;
    /*float diff;
    CalculateDiffuseTerm(normalize(i.vNormalDepth.xyz), normalize(vSunLightDir.xyz), diff, Glossiness);
    outColor.xyz *= diff;
    float SpecularTerm;
    float3 ViewDir = normalize(i.vPosition.xyz - ViewInv[3].xyz);
    CalculateSpecularTerm(normalize(i.vNormalDepth.xyz), normalize(vSunLightDir.xyz), ViewDir, 0.5, SpecularTerm);
    outColor.xyz += SpecularTerm;*/
    //outColor.xyz *= i.vColor.xyz;
	//outColor.a *= i.vColor.w;

	return outColor;
}
void ShadowPS(PS_DEFERRED_IN i)
{
	float4 outColor = txDiffuse.Sample(samLinear, i.vTexCoord.xy);
	if (outColor.a < 0.3)
		discard;
}
PS_DEFERRED_OUT DeferredPS(PS_DEFERRED_IN i)
{
    PS_DEFERRED_OUT Out;
    float4 baseColor = txDiffuse.Sample(samLinear, i.vTexCoord.xy);
    if (baseColor.a < 0.3)
        discard;
	FillGBuffer(Out, baseColor, i.vNormalDepth.xyz, i.vNormalDepth.w, float4(fSpecularIntensity, fGlossiness,0,0));
	

	return Out;
}
void VoxelPS(PS_VOXEL_INPUT i)
{
	float4 outColor = txDiffuse.Sample(samLinear, i.vTexCoord)* cDiffuseColor;
	float DiffuseTerm;
	CalculateDiffuseTerm(i.vNormal.xyz, vSunLightDir.xyz, DiffuseTerm, 0.5f);
	int3 gPos = (int3) ConvertToVoxelSpace(i.vWorldPos.xyz);

	if (gPos.x > 0 && gPos.x < voxelGridSize && gPos.y > 0 && gPos.y < voxelGridSize && gPos.z > 0 && gPos.z < voxelGridSize) {
        float4 color = float4(outColor.xyz * outColor.w * max(DiffuseTerm, 1 - vSunLightDir.w), outColor.w);
        InjectColorAndNormal(gPos, color, i.vNormal.xyz);
	}
}
void VoxelEmmissivePS(PS_VOXEL_INPUT i)
{

}