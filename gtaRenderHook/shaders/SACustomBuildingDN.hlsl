#include "GameMath.hlsl"
#include "LightingFunctions.hlsl"
#include "GBuffer.hlsl"
#include "VoxelizingHelper.hlsl"
//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register( t0 );
Texture2D txSpec 	: register( t1 );
SamplerState samLinear : register(s0);
Texture2D txShadow 	: register(t4);

struct VS_INPUT
{
    float3 inPosition   	: POSITION;
    float2 inTexCoord     	: TEXCOORD;
    float3 vInNormal    	: NORMAL;
	float4 vInColor 		: COLOR;
};
struct GS_VOXEL_IN
{
	float4 vPosition		: SV_POSITION;    // World position
	float4 vNormal			: NORMAL;
	float4 vColor			: COLOR;
	float2 vTexCoord		: TEXCOORD0;         // Texture coord
};
struct PS_VOXEL_INPUT
{
	float4 vPosition	: SV_POSITION;
	float4 vWorldPos	: WPOS;
	float4 vNormal		: NORMAL;
	float4 vColor		: COLOR;
	float2 vTexCoord	: TEXCOORD0;
};
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
DEFERRED_INPUT VS( VS_INPUT i )
{
	DEFERRED_INPUT Out;
	float4 	outPos 		= float4( i.inPosition,1.0);// transform to screen space
			outPos 		= mul( outPos, World );
			outPos		= mul( outPos, View );
    Out.vPosition 	= mul( outPos, Projection );
	Out.vNormalDepth = float4(mul( i.vInNormal,(float3x3)World), outPos.z);
	Out.vTexCoord = float4(i.inTexCoord, 0, 0);
	Out.vColor		= i.vInColor;
	
    return Out;
}
GS_VOXEL_IN VoxelVS(VS_INPUT i)
{
	GS_VOXEL_IN Out = (GS_VOXEL_IN)0.0f;

	Out.vPosition = mul(float4(i.inPosition, 1.0), World);
	Out.vTexCoord = i.inTexCoord;
	Out.vNormal = float4(mul(i.vInNormal, (float3x3)World), 1.0);
	Out.vColor = i.vInColor;
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
			output.vColor = input[v].vColor;
			output.vPosition = mul(input[v].vPosition, VoxelView[f]);
			output.vPosition = mul(output.vPosition, Projection);
			output.vTexCoord = input[v].vTexCoord;
			VoxelStream.Append(output);
		}
		VoxelStream.RestartStrip();
	}
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(DEFERRED_INPUT i ) : SV_Target
{
	float4 outColor =txDiffuse.Sample( samLinear, i.vTexCoord.xy ) * DiffuseColor;
    outColor.xyz *= i.vColor.xyz;
	outColor.a		*=i.vColor.w;

	return outColor;
}

void ShadowPS(DEFERRED_INPUT i)
{
	float4 outColor = txDiffuse.Sample(samLinear, i.vTexCoord.xy);
	if (outColor.a < 0.3)
		discard;
}

DEFERRED_OUTPUT DeferredPS(DEFERRED_INPUT i )
{
	DEFERRED_OUTPUT Out;
	float4 baseColor = txDiffuse.Sample(samLinear, i.vTexCoord.xy);
    float4 params = HasSpecTex > 0 ? txSpec.Sample(samLinear, i.vTexCoord.xy) : float4(SpecularIntensity, Glossiness,0,0);

    FillGBuffer(Out, baseColor * DiffuseColor, -i.vNormalDepth.xyz, i.vNormalDepth.w, params);
	if (baseColor.a < 0.3)
		discard;
    
	return Out;
}

void VoxelPS(PS_VOXEL_INPUT i)
{
	float4 outColor = txDiffuse.Sample(samLinear, i.vTexCoord)* DiffuseColor;
	float DiffuseTerm;
	CalculateDiffuseTerm(-i.vNormal.xyz, vSunLightDir.xyz, DiffuseTerm, 0.5f);

	int3 gPos = (int3) ConvertToVoxelSpace(i.vWorldPos.xyz);

	if (gPos.x > 0 && gPos.x < voxelGridSize && gPos.y > 0 && gPos.y < voxelGridSize && gPos.z > 0 && gPos.z < voxelGridSize) {
        float4 color = float4(outColor.xyz * outColor.w * max(DiffuseTerm, 1 - vSunLightDir.w), outColor.w);
        InjectColorAndNormal(gPos, color, -i.vNormal.xyz);
    }
}
void VoxelEmmissivePS(PS_VOXEL_INPUT i)
{
	float4 outColor = txDiffuse.Sample(samLinear, i.vTexCoord)* DiffuseColor;
	float DiffuseTerm;
	CalculateDiffuseTerm(i.vNormal.xyz, vSunLightDir.xyz, DiffuseTerm, 0.5f);

	int3 gPos = (int3) ConvertToVoxelSpace(i.vWorldPos.xyz);

	if (gPos.x > 0 && gPos.x < voxelGridSize && gPos.y > 0 && gPos.y < voxelGridSize && gPos.z > 0 && gPos.z < voxelGridSize) {
        float4 color = float4(outColor.xyz * outColor.w * i.vColor.xyz * 4, outColor.w);
        InjectColorAndNormal(gPos, color, -i.vNormal.xyz);
	}
}