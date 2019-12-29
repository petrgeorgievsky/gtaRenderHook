#include "GameMath.hlsl"
#include "LightingFunctions.hlsl"
#include "GBuffer.hlsl"
#include "AtmosphericScatteringFunctions.hlsli"
#include "ReflectionFunctions.hlsli"
#include "VoxelizingHelper.hlsl"
#include "Shadows.hlsl"
//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register(t0);
Texture2D txGB1     : register(t1);
Texture2D txGB0     : register(t2);
Texture2D txShadow 	: register(t4);
Texture2D txWaterWake : register(t5);

SamplerState samLinear : register(s0);
SamplerComparisonState samShadow : register(s1);

struct VS_INPUT
{
    float4 inPosition : POSITION;
    float2 inTexCoord : TEXCOORD;
    float3 vInNormal : NORMAL;
    float4 vInColor : COLOR;
	
    uint uVertexID : SV_VERTEXID;
};
struct VS_OUTPUT_HS_INPUT
{
    float3 vWorldPos : WORLDPOS;
    float3 vNormal : NORMAL;

    float2 vTexCoord : TEXCOORD0;
    float4 vColor : COLOR;
    float fVertexDistanceFactor : VERTEXDISTANCEFACTOR;
};
struct HS_CONSTANT_DATA_OUTPUT
{
    float Edges[4] : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
};

struct HS_CONTROL_POINT_OUTPUT
{
    float3 vWorldPos : WORLDPOS;
    float3 vNormal : NORMAL;
    float2 vTexCoord : TEXCOORD;
    float4 vColor : COLOR;
};


struct DS_OUTPUT
{
    float2 texCoord : TEXCOORD0;
    float4 vColor : COLOR;
    float3 vNormal : NORMAL;
    float3 vWorldPos : WORLDPOS;
    float fDepth : TEXCOORD1;
    float4 vPosition : SV_POSITION;
};

struct PS_INPUT
{
    float2 vTexCoord : TEXCOORD0;
    float4 vColor : COLOR;
    float3 vNormal : NORMAL;
    float3 vWorldPos : WORLDPOS;
    float fDepth : TEXCOORD1;
    float4 vVPos : SV_POSITION;
};
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT_HS_INPUT VS(VS_INPUT i)
{
    VS_OUTPUT_HS_INPUT Out;
	// Compute clip-space position.
	float4 	outPos = float4(i.inPosition.xyz, 1.0);
    //outPos.z += sin(i.inPosition.x) * cos(i.inPosition.y);
	//outPos = mul(outPos, World);
    Out.vWorldPos = outPos.xyz;

	//Out.vPosition = mul(outPos, Projection);

	Out.vNormal = i.vInNormal;//float4(mul(i.vInNormal, (float3x3)World), outPos.z);
    Out.vTexCoord = i.inTexCoord;
	Out.vColor = i.vInColor;
    // Min and max distance should be chosen according to scene quality requirements
    const float fMinDistance = 2.0f;
    const float fMaxDistance = 550.0f;

    // Calculate distance between vertex and camera, and a vertex distance factor issued from it
    float fDistance = distance(outPos.xyz, mViewInv[3].xyz);
    Out.fVertexDistanceFactor = 1.0 - clamp(((fDistance - fMinDistance) / (fMaxDistance - fMinDistance)),
                                             0.0, 1.0 - (1.0 / 16.0f));

	return Out;
}

HS_CONSTANT_DATA_OUTPUT ConstantsHS(InputPatch<VS_OUTPUT_HS_INPUT, 4> p)
{
    HS_CONSTANT_DATA_OUTPUT output = (HS_CONSTANT_DATA_OUTPUT) 0;
    float4 vEdgeTessellationFactors;
    float tess = 2;
    // Tessellation level fixed by variable
    vEdgeTessellationFactors = float4(tess, tess, tess, tess);
	
    float3 fScaleFactor;
    fScaleFactor.x = 0.5 * (p[1].fVertexDistanceFactor + p[2].fVertexDistanceFactor);
    fScaleFactor.y = 0.5 * (p[2].fVertexDistanceFactor + p[0].fVertexDistanceFactor);
    fScaleFactor.z = 0.5 * (p[0].fVertexDistanceFactor + p[1].fVertexDistanceFactor);

    // Scale edge factors 
    //vEdgeTessellationFactors *= fScaleFactor.xyzx;
	
    // Assign tessellation levels
    output.Edges[0] = vEdgeTessellationFactors.x;
    output.Edges[1] = vEdgeTessellationFactors.y;
    output.Edges[2] = vEdgeTessellationFactors.z;
    output.Edges[3] = vEdgeTessellationFactors.z;
    output.Inside[0] = output.Inside[1] = vEdgeTessellationFactors.w;

    return output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantsHS")]
[maxtessfactor(2.0)]
HS_CONTROL_POINT_OUTPUT HS(InputPatch<VS_OUTPUT_HS_INPUT, 4> inputPatch,
                            uint uCPID : SV_OutputControlPointID)
{
    HS_CONTROL_POINT_OUTPUT output = (HS_CONTROL_POINT_OUTPUT) 0;
    
    // Copy inputs to outputs
    output.vWorldPos = inputPatch[uCPID].vWorldPos.xyz;
    output.vNormal = inputPatch[uCPID].vNormal;
    output.vTexCoord = inputPatch[uCPID].vTexCoord;
    output.vColor = inputPatch[uCPID].vColor;

    return output;
}
//--------------------------------------------------------------------------------------
// Domain Shader
//--------------------------------------------------------------------------------------
static const int g_WaveCount = 3;
// (Frequency, Amplitude, Speed)
static const float3 g_WaveParams[] =
{
    float3(0.4, 0.25, 20), float3(0.25, 0.35, 10), float3(0.425, 0.25, 8)
};
static const float2 g_WaveDirections[] = {
    float2(0.5, 0.5), float2(0.65, 0.65), float2(0.35, 0.35)
};
float SimpleWaveHeight(int id, float2 position, float t)
{
    return g_WaveParams[id].y * sin(dot(g_WaveDirections[id], position) * g_WaveParams[id].x + g_WaveParams[id].z * g_WaveParams[id].x * t);
}
float3 GerstnerWaveDispl(int id, float2 position, float t)
{
    float q = 1.0 / (g_WaveParams[id].x * g_WaveParams[id].y * 2.0f);
    float c = cos(dot(g_WaveDirections[id], position) * g_WaveParams[id].x + g_WaveParams[id].z * g_WaveParams[id].x * t);
    float s = sin(dot(g_WaveDirections[id], position) * g_WaveParams[id].x + g_WaveParams[id].z * g_WaveParams[id].x * t);
    float xDisp = q * g_WaveParams[id].y * g_WaveDirections[id].x * c;
    float yDisp = q * g_WaveParams[id].y * g_WaveDirections[id].y * c;
    float zDisp = s * g_WaveParams[id].y;
    return float3(xDisp, yDisp, zDisp);
}
float2 SimpleWaveNormal(int id, float2 position, float t)
{
    float dHdX = g_WaveParams[id].x * g_WaveDirections[id].x * g_WaveParams[id].y * cos(dot(g_WaveDirections[id], position) * g_WaveParams[id].x + g_WaveParams[id].z * g_WaveParams[id].x * t);
    float dHdY = g_WaveParams[id].x * g_WaveDirections[id].y * g_WaveParams[id].y * cos(dot(g_WaveDirections[id], position) * g_WaveParams[id].x + g_WaveParams[id].z * g_WaveParams[id].x * t);
    return float2(dHdX,dHdY);
}
float3 GerstnerWaveNormal(int id, float2 position, float t)
{
    float wa = (g_WaveParams[id].x * g_WaveParams[id].y)*2.0f;
    float q = 1.0 / wa;
    
    float c = cos(dot(g_WaveDirections[id], position) * g_WaveParams[id].x + g_WaveParams[id].z * g_WaveParams[id].x * t);
    float s = sin(dot(g_WaveDirections[id], position) * g_WaveParams[id].x + g_WaveParams[id].z * g_WaveParams[id].x * t);

    float xDisp = wa* g_WaveDirections[id].x * c;
    float yDisp = wa* g_WaveDirections[id].y * c;
    float zDisp = s * g_WaveParams[id].y;
    return float3(xDisp, yDisp, zDisp);
}
void ComputeWaves(float2 pos, out float3 displ, out float3 normals,float time)
{
    float3 Height = 0.0;
    for (int i = 0; i < g_WaveCount;i++)
        Height += GerstnerWaveDispl(i, pos, time); //SimpleWaveHeight(i, pos, time);
    
    float3 Normal = float3(0, 0, 0);
    for (int j = 0; j < g_WaveCount; j++)
        Normal += GerstnerWaveNormal(j, pos, time);

    displ = Height;
    normals = float3(-Normal.xy, 1 - Normal.z);
}

[domain("quad")]
DS_OUTPUT DS(HS_CONSTANT_DATA_OUTPUT input, float2 vUVs : SV_DomainLocation,
             const OutputPatch<HS_CONTROL_POINT_OUTPUT, 4> TrianglePatch)
{
    DS_OUTPUT output = (DS_OUTPUT) 0;

    // Interpolate world space position
    float3 vWorldPos = vUVs.y * (vUVs.x * TrianglePatch[0].vWorldPos + (1-vUVs.x) * TrianglePatch[1].vWorldPos) +
                       (1 - vUVs.y) * (vUVs.x * TrianglePatch[2].vWorldPos + (1 - vUVs.x) * TrianglePatch[3].vWorldPos);
	
    // Interpolate other inputs
    output.texCoord = vUVs.y * (vUVs.x * TrianglePatch[0].vTexCoord + (1 - vUVs.x) * TrianglePatch[1].vTexCoord) +
                (1 - vUVs.y) * (vUVs.x * TrianglePatch[2].vTexCoord + (1 - vUVs.x) * TrianglePatch[3].vTexCoord); 
    /*(1 - vUVs.x) * TrianglePatch[0].vTexCoord +
    vUVs.x * TrianglePatch[1].vTexCoord +
                      vUVs.y * TrianglePatch[2].vTexCoord +
                      (1 - vUVs.y) * TrianglePatch[3].vTexCoord;*/
    output.vColor = vUVs.y * (vUVs.x * TrianglePatch[0].vColor + (1 - vUVs.x) * TrianglePatch[1].vColor) +
              (1 - vUVs.y) * (vUVs.x * TrianglePatch[2].vColor + (1 - vUVs.x) * TrianglePatch[3].vColor); /*(1 - vUVs.x) * TrianglePatch[0].vColor +
                      vUVs.x * TrianglePatch[1].vColor +
                      vUVs.y * TrianglePatch[2].vColor +
                      (1 - vUVs.y) * TrianglePatch[3].vColor;*/
        float fHeightMapMIPLevel = clamp((distance(vWorldPos, mViewInv[3].xyz) - 100.0f) / 100.0f, 3.0f, 6.0f);
    float3 disp;
    output.vNormal = vUVs.y * (vUVs.x * TrianglePatch[0].vNormal + (1 - vUVs.x) * TrianglePatch[1].vNormal) +
                (1 - vUVs.y) * (vUVs.x * TrianglePatch[2].vNormal + (1 - vUVs.x) * TrianglePatch[3].vNormal);
    //ComputeWaves(vWorldPos.xy, disp, output.vNormal, fTimeStep);
    //float4 water_normal = txDiffuse.SampleLevel(samLinear, output.texCoord.xy*8, 0);
    //output.vNormal += water_normal.xyw*0.01f;
    //output.vNormal = normalize(output.vNormal);
    //vWorldPos.xy += disp.xy;
    //vWorldPos.z += disp.z - water_normal.w*0.1+0.1;
    // Transform world position with viewprojection matrix
    float4 outPos = mul(float4(vWorldPos.xyz, 1.0), mView);
    outPos = mul(outPos, mProjection);
    output.vPosition = outPos;
    output.fDepth = outPos.z;
    output.vWorldPos = vWorldPos;
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT i) : SV_Target
{
    float4 OutColor = float4(i.vNormal.xyz,1);

    float3 Normal = normalize(i.vNormal.xyz);
    float scatter_factor;

    float3 vLightPosition = mViewInv[3].xyz + vSunLightDir.xyz * 1000;

    float3 LightDir = normalize(vSunLightDir.xyz);
    float3 ViewDir = normalize(i.vWorldPos - mViewInv[3].xyz);

    float2 ScreenCoords = i.vVPos.xy / float2(fScreenWidth, fScreenHeight);
    // Retrieve depth and normals
    float3 Normals;
    float ViewZ;
    GetNormalsAndDepth(txGB1, samLinear, ScreenCoords, ViewZ, Normals);

    float Shadow = SampleShadowCascades( txShadow, samShadow, samLinear, i.vWorldPos, length(i.vWorldPos.xyz - mViewInv[3].xyz) );
    float2 ws_tc     = frac( i.vWorldPos.xy / 64 );
    float2 waterWake = txWaterWake.Sample( samLinear, ws_tc * 16 ).ra;
    float3 SmallWaveNormal =
        normalize( 2 * txDiffuse.Sample( samLinear, ws_tc * 16 ).gbr -
                   float3( 1, -8, 1 ) );
    SmallWaveNormal +=
        normalize( 2 * txDiffuse.Sample( samLinear, ws_tc * 8 + 0.05 ).gbr -
                   float3( 1, -8, 1 ) );
    
    float3x3 NormalSpaceMatrix;
	// calculating base normal rotation matrix
    NormalSpaceMatrix[1] = Normal.xyz;
    NormalSpaceMatrix[2] = normalize(cross(float3(0.0, -1.0, 0.0), NormalSpaceMatrix[1]));
    NormalSpaceMatrix[0] = normalize(cross(NormalSpaceMatrix[2], NormalSpaceMatrix[1]));

	// applying base normal rotation matrix to high frequency bump normal
    SmallWaveNormal = mul(normalize(SmallWaveNormal), NormalSpaceMatrix);
    
    float g_WaterColorIntensity = 0.2;

    float FresnelCoeff = MicrofacetFresnel(-ViewDir, SmallWaveNormal, 0.5f);
    float DiffuseLighting = max(0, dot(LightDir, SmallWaveNormal)) * Shadow;
    float DiffuseTerm = 0.1 + g_WaterColorIntensity * DiffuseLighting;

    float3 ReflectDir = normalize(reflect(ViewDir, SmallWaveNormal));

    float SpecularTerm;
    CalculateSpecularTerm(SmallWaveNormal.xyz, LightDir, -ViewDir, GetLuminance(SmallWaveNormal), SpecularTerm);
    // = Shadow * FresnelCoeff;
    
    float WaterDepth = ViewZ - i.fDepth;
    WaterDepth = max(0, WaterDepth);

    float ReflectionFallback;
    float3 ReflectionColor = lerp(SSR(txGB0, txGB1, samLinear, i.vWorldPos, ReflectDir, 0.5f, ReflectionFallback), vSkyLightCol.rgb, ReflectionFallback);
    float3 RefractionColor = txGB0.Sample(samLinear, ScreenCoords).rgb;
    RefractionColor = lerp(DiffuseTerm, RefractionColor, min(1, exp(-WaterDepth / 8.0))) * vWaterColor.rgb;

    OutColor.rgb = lerp( lerp( RefractionColor, ReflectionColor, FresnelCoeff ),
                         waterWake.rrr * min( DiffuseLighting + 0.5, 1.0f ),
                         min( exp( -WaterDepth ), 1 ) * waterWake.g );
    OutColor.rgb += min(SpecularTerm, 16.0f) * Shadow * vSunLightDir.w * FresnelCoeff;
    float3 FullScattering;
    OutColor.rgb = CalculateFogColor(OutColor.rgb, ViewDir, LightDir, min(ViewZ, i.fDepth), i.vWorldPos.z, FullScattering);
    OutColor.a = 1-min(exp(-WaterDepth*2), 1);
    return OutColor;
}