#include "GameMath.hlsl"
#include "LightingFunctions.hlsl"
#include "GBuffer.hlsl"
#include "VoxelizingHelper.hlsl"
//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register( t0 );
Texture2D    txSpec : register( t1 );
Texture2D    txNormals : register( t2 );
Texture2D txShadow  : register(t4);
SamplerState samLinear : register(s0);


struct VS_BUILDING_IN
{
    float3 vPosition   	: POSITION;
    float2 vTexCoord    : TEXCOORD;
    float3 vNormal    	: NORMAL;
    float4 cColor : COLOR;
    float3 vInTangents : TEXCOORD1;
    float3 vInBiTangents : TEXCOORD2;
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
PS_DEFERRED_IN VS(VS_BUILDING_IN i)
{
    PS_DEFERRED_IN o;
	float4 	OutPos 	= float4( i.vPosition,1.0);// transform to screen space
			OutPos 	= mul( OutPos, mWorld );
			OutPos	= mul( OutPos, mView );
    o.vPosition 	= mul( OutPos, mProjection );
	o.vNormalDepth  = float4(mul( i.vNormal,(float3x3)mWorld), OutPos.z);
    o.vTangent   = float4( mul( (float3x3)mWorldInv, i.vInTangents ), 1.0 );
    o.vBiTangent = float4( mul( (float3x3)mWorldInv, i.vInBiTangents ), 1.0 );
	o.vTexCoord	    = float4(i.vTexCoord,0,0);
	o.vColor		= i.cColor;
	
    return o;
}
GS_VOXEL_IN VoxelVS(VS_BUILDING_IN i)
{
	GS_VOXEL_IN Out = (GS_VOXEL_IN)0.0f;

	Out.vPosition = mul(float4(i.vPosition, 1.0), mWorld);
	Out.vTexCoord = i.vTexCoord;
	Out.vNormal = float4(mul(i.vNormal, (float3x3)mWorld),1.0);
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
	float DiffuseTerm;
	float4 outColor;
    float diff;
    CalculateDiffuseTerm(normalize(i.vNormalDepth.xyz), normalize(vSunLightDir.xyz), diff, 1.0f-fGlossiness);
    diff *= vSunLightDir.w;

	outColor 		 = txDiffuse.Sample( samLinear, i.vTexCoord.xy ) * cDiffuseColor;
    outColor.xyz *= (diff + 0.1f);

	return outColor;
}
void VoxelPS(PS_VOXEL_INPUT i)
{
	float4 outColor = txDiffuse.Sample(samLinear, i.vTexCoord)* cDiffuseColor;
	float DiffuseTerm;
	CalculateDiffuseTerm(i.vNormal.xyz, vSunLightDir.xyz, DiffuseTerm, 0.5f);

	int3 gPos = (int3) ConvertToVoxelSpace(i.vWorldPos.xyz);

	if (gPos.x > 0 && gPos.x < voxelGridSize && gPos.y > 0 && gPos.y < voxelGridSize && gPos.z > 0 && gPos.z < voxelGridSize) {
        float4 color = float4(outColor.xyz * outColor.w, outColor.w);
        InjectColorAndNormal(gPos, color, i.vNormal.xyz);
    }
}
void ShadowPS(PS_DEFERRED_IN i)
{
    float4 baseColor = txDiffuse.Sample(samLinear, i.vTexCoord.xy) * cDiffuseColor;
	if (baseColor.a < 0.3)
		discard;
}
PS_DEFERRED_OUT DeferredPS(PS_DEFERRED_IN i)
{
    PS_DEFERRED_OUT Out;
	float4 baseColor=txDiffuse.Sample( samLinear, i.vTexCoord.xy );
    float4 params = bHasSpecTex > 0 ? txSpec.Sample(samLinear, i.vTexCoord.xy) : float4(fSpecularIntensity, fGlossiness, 0, 0);
    float3 normal = i.vNormalDepth.xyz;
    if ( bHasNormalTex > 0 )
    {
        float3x3 tbn =
            float3x3( normalize( i.vTangent.xyz ),
                      normalize( cross( normalize( normal ),
                                        normalize( i.vTangent.xyz ) ) ),
                      normalize( normal ) );

        normal = txNormals.Sample( samLinear, i.vTexCoord.xy ).xyz;
        normal = normalize( normal * 2.0 - 1.0 );
        normal = normalize( mul( normal, tbn ) );
    }
    
    FillGBuffer( Out, baseColor * cDiffuseColor, normal, i.vNormalDepth.w,
                 params );
	if (baseColor.a < 0.3)
		discard;
	
	return Out;
}
void VoxelEmmissivePS(PS_VOXEL_INPUT i)
{
	
}