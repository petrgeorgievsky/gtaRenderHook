#include "GameMath.hlsl"
#include "LightingFunctions.hlsl"
#include "GBuffer.hlsl"
#include "VoxelizingHelper.hlsl"
#include "AtmosphericScatteringFunctions.hlsli"
#include "Shadows.hlsl"
//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D    txDiffuse : register( t0 );
Texture2D    txSpec : register( t1 );
Texture2D    txNormals : register( t2 );
Texture2D txShadow 	: register(t4);
TextureCube            txCubeMap : register( t5 );
SamplerState           samLinear : register( s0 );
SamplerComparisonState samShadow : register( s1 );

struct VS_INPUT
{
    float3 vPosition   	: POSITION;
    float2 vTexCoord    : TEXCOORD;
    float3 vInNormal : NORMAL;
    float4 vInColor : COLOR;
    float3 vInTangents : TEXCOORD1;
    float3 vInBiTangents : TEXCOORD2;
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
struct PS_DEFERRED_DN_IN
{
    float4 vPosition    : SV_POSITION;
    float4 vColor       : COLOR;
    float4 vNormalDepth : NORMAL;
    float4 vTexCoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vTangent : TEXCOORD2;
    float4 vBiTangent : TEXCOORD3;
};
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_DEFERRED_DN_IN VS(VS_INPUT i)
{
    PS_DEFERRED_DN_IN o;
	float4 	outPos 		= float4( i.vPosition,1.0);// transform to screen space
			outPos 		= mul( outPos, mWorld );
    o.vWorldPos = outPos;
			outPos		= mul( outPos, mView );
    o.vPosition         = mul( outPos, mProjection );

    o.vNormalDepth =
        float4( mul( (float3x3)mWorldInv, i.vInNormal ), outPos.z );
    o.vTangent   = float4( mul( (float3x3)mWorldInv, i.vInTangents ), 1.0 );
    o.vBiTangent = float4( mul( (float3x3)mWorldInv, i.vInBiTangents ), 1.0 );
	o.vTexCoord     = float4(i.vTexCoord, 0, 0);
	o.vColor		= i.vInColor;
	
    return o;
}
GS_VOXEL_IN VoxelVS(VS_INPUT i)
{
	GS_VOXEL_IN Out = (GS_VOXEL_IN)0.0f;

	Out.vPosition = mul(float4(i.vPosition, 1.0), mWorld);
	Out.vTexCoord = i.vTexCoord;
	Out.vNormal = float4(mul(i.vInNormal, (float3x3)mWorld), 1.0);
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
float4 PS(PS_DEFERRED_DN_IN i) : SV_Target
{
    const float3 ViewPos  = mViewInv[3].xyz;
    float3       WorldPos = i.vWorldPos.xyz;
    float3       Normals  = ( i.vNormalDepth.xyz * 0.5f + 0.5f ) * 2.0f - 1.0f;
    Normals.z             = sqrt( 1.01 - dot( Normals.xy, Normals.xy ) );
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
    // DiffuseTerm *= vSunLightDir.w;

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
    float3 ReflDir = normalize( reflect( ViewDir, normalize( Normals.xyz ) ) );
    float3 FullScattering;

    float3 ObjectColor = CalculateFogColor( float3( 0, 0, 0 ), ReflDir,
                                            LightDir, 1000, 0, FullScattering );

    float3 SkyColor = GetSkyColor( ReflDir, LightDir, FullScattering );

    float3 ReflectionFallBack;
    ReflDir.x *= -1;
    float4 CubeMap =
        txCubeMap.SampleLevel( samLinear, ReflDir, ( 1 - fGlossiness ) * 9.0f );
    ReflectionFallBack = lerp( CubeMap.rgb, SkyColor, 1 - CubeMap.a );
    float3 sun_lighting = DiffuseTerm * ShadowTerm * vSunColor.rgb;
    float3 radiance = i.vColor.rgb * saturate( 1.0f - vSunLightDir.w + 0.2f );
    // todo: add lighting methods for forward renderer
    outColor.rgb =
        albedoSample.rgb *
            ( sun_lighting + radiance +
              vSkyLightCol.rgb * 0.3f ) +
        SpecularTerm * fSpecularIntensity * vSunLightDir.w * vSunColor.rgb *
            ShadowTerm +
        ReflectionFallBack * fSpecularIntensity;
    outColor.rgb =
        CalculateFogColor( outColor.rgb, ViewDir, LightDir, i.vNormalDepth.w,
                           WorldPos.z, FullScattering );
    outColor.a = albedoSample.a * i.vColor.a;

    return outColor;
}

void ShadowPS(PS_DEFERRED_IN i)
{
#if HAS_TEXTURE == 1
	float4 outColor = txDiffuse.Sample(samLinear, i.vTexCoord.xy);
    outColor.a = outColor.a > 0.95f ? outColor.a : InterleavedGradientNoise(i.vPosition.xy) * outColor.a;
	if (outColor.a < 0.2f)
		discard;
#endif
}

PS_DEFERRED_OUT DeferredPS(PS_DEFERRED_IN i)
{
    PS_DEFERRED_OUT Out;
    float4 baseColor = cDiffuseColor;
    if (bHasTexture != 0)
        baseColor *= txDiffuse.Sample(samLinear, i.vTexCoord.xy);
    
    float4 params = bHasSpecTex > 0 ? float4(txSpec.Sample(samLinear, i.vTexCoord.xy).xyz, 3) : float4(fSpecularIntensity, fGlossiness, 0, 3);
    params.w = baseColor.a > 0.95f ? 3 : 5;
    baseColor.a = baseColor.a > 0.95f ? baseColor.a : InterleavedGradientNoise(i.vPosition.xy) * baseColor.a;
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

	if (baseColor.a < 0.2f)
		discard;
    FillGBufferVertexRadiance( Out, baseColor, normal, i.vNormalDepth.w, params,
                               i.vColor * lerp( 0.25f, 1.0f, 1 - vSunLightDir.a ) );
	return Out;
}

void VoxelPS(PS_VOXEL_INPUT i)
{
	float4 outColor = txDiffuse.Sample(samLinear, i.vTexCoord)* cDiffuseColor;
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
	float4 outColor = txDiffuse.Sample(samLinear, i.vTexCoord)* cDiffuseColor;
	float DiffuseTerm;
	CalculateDiffuseTerm(i.vNormal.xyz, vSunLightDir.xyz, DiffuseTerm, 0.5f);

	int3 gPos = (int3) ConvertToVoxelSpace(i.vWorldPos.xyz);

	if (gPos.x > 0 && gPos.x < voxelGridSize && gPos.y > 0 && gPos.y < voxelGridSize && gPos.z > 0 && gPos.z < voxelGridSize) {
        float4 color = float4(outColor.xyz * outColor.w * i.vColor.xyz * 4, outColor.w);
        InjectColorAndNormal(gPos, color, -i.vNormal.xyz);
	}
}