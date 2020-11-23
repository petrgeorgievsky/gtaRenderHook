#include "GameMath.hlsl"
#include "LightingFunctions.hlsl"
#include "GBuffer.hlsl"
#include "VoxelizingHelper.hlsl"
#include "AtmosphericScatteringFunctions.hlsli"
#include "Shadows.hlsl"
//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register( t0 );
Texture2D    txSpec : register( t1 );
Texture2D    txNormals : register( t2 );
Texture2D              txShadow : register( t4 );
TextureCube            txCubeMap : register( t5 );
SamplerState           samLinear : register( s0 );
SamplerComparisonState samShadow : register( s1 );

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
struct PS_DEFERRED_CF_IN
{
    float4 vPosition : SV_POSITION;
    float4 vColor : COLOR;
    float4 vNormalDepth : NORMAL;
    float4 vTexCoord : TEXCOORD0;
    float4 vTangent : TEXCOORD1;
    float4 vBiTangent : TEXCOORD2;
    float4 vWorldPos : TEXCOORD3;
};
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_DEFERRED_CF_IN VS( VS_BUILDING_IN i )
{
    PS_DEFERRED_CF_IN o;
	float4 	OutPos 	= float4( i.vPosition,1.0);// transform to screen space
    OutPos          = mul( OutPos, mWorld );
    o.vWorldPos          = OutPos;
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
float4 PS( PS_DEFERRED_CF_IN i )
    : SV_Target
{
    const float3 ViewPos  = mViewInv[3].xyz;
    float3       WorldPos = i.vWorldPos.xyz;
    float3       Normals  = i.vNormalDepth.xyz;
    Normals               = normalize( Normals );
    float3 ViewDir        = normalize( WorldPos.xyz - ViewPos );
    float3 LightDir       = normalize( vSunLightDir.xyz );

    float DiffuseTerm, SpecularTerm;
    CalculateDiffuseTerm_ViewDependent( Normals.xyz, LightDir, ViewDir,
                                        DiffuseTerm, 1 - fGlossiness );
    CalculateSpecularTerm( Normals.xyz, LightDir, -ViewDir, 1 - fGlossiness,
                           SpecularTerm );
    // clamp to avoid unrealisticly high values
    // SpecularTerm = min(SpecularTerm, 16.0f);
    DiffuseTerm *= vSunLightDir.w;

    float4 albedoSample = cDiffuseColor;
    if ( bHasTexture != 0 )
        albedoSample *= txDiffuse.Sample( samLinear, i.vTexCoord.xy );
    if ( albedoSample.a < 0.3 )
        discard;
    float4 outColor;
#if SAMPLE_SHADOWS != 1
    float ShadowTerm =
        SampleShadowCascades( txShadow, samShadow, samLinear, WorldPos,
                              length( WorldPos.xyz - ViewPos ) ) *
        vSunLightDir.w;
#else
    float ShadowTerm = 1.0;
#endif
    float3 ReflDir =
        normalize( reflect( ViewDir, normalize( i.vNormalDepth.xyz ) ) );
    float3 FullScattering;

    float3 ObjectColor = CalculateFogColor( float3( 0, 0, 0 ), ReflDir,
                                            LightDir, 1000, 0, FullScattering );

    float3 SkyColor = GetSkyColor( ReflDir, LightDir, FullScattering );

    float3 ReflectionFallBack;
    ReflDir.x *= -1;
    float4 CubeMap =
        txCubeMap.SampleLevel( samLinear, ReflDir, ( 1 - fGlossiness ) * 9.0f );
    ReflectionFallBack = lerp( CubeMap.rgb, SkyColor, 1 - CubeMap.a );
    // todo: add lighting methods for forward renderer
    outColor.rgb =
        albedoSample.rgb *
            ( DiffuseTerm * ShadowTerm* vSunColor.rgb +
              lerp( vSkyLightCol.rgb, vHorizonCol.rgb, i.vNormalDepth.z ) *
                  0.25f ) +
        SpecularTerm * fSpecularIntensity * vSunLightDir.w * vSunColor.rgb *
            ShadowTerm +
        ReflectionFallBack * fSpecularIntensity;
    outColor.a = albedoSample.a * i.vColor.a;
    outColor.rgb =
        CalculateFogColor( outColor.rgb, ViewDir, LightDir, i.vNormalDepth.w,
                           WorldPos.z, FullScattering );

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
    if ( bHasNormalTex > 0 && length( i.vTangent.xyz ) > 0 )
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