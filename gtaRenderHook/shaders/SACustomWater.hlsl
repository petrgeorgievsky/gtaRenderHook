#include "GameMath.hlsl"
#include "LightingFunctions.hlsl"
#include "GBuffer.hlsl"
//#include "AtmosphericScattering.hlsl"
#include "VoxelizingHelper.hlsl"
#include "Shadows.hlsl"
//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register(t0);
Texture2D txGB1     : register(t1);
Texture2D txGB0     : register(t2);
Texture2D txShadow 	: register(t3);

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
	Out.vTexCoord = float4(i.inTexCoord, 0, 0);
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
    ComputeWaves(vWorldPos.xy, disp, output.vNormal, fTimeStep);
    float4 water_normal = txDiffuse.SampleLevel(samLinear, output.texCoord.xy*8, 0);
    output.vNormal += water_normal.xyw*0.01f;
    output.vNormal = normalize(output.vNormal);
    vWorldPos.xy += disp.xy;
    vWorldPos.z += disp.z - water_normal.w*0.1+0.1;
    // Transform world position with viewprojection matrix
    float4 outPos = mul(float4(vWorldPos.xyz, 1.0), mView);
    outPos = mul(outPos, mProjection);
    output.vPosition = outPos;
    output.fDepth = outPos.z;
    output.vWorldPos = vWorldPos;
    return output;
}


float3 GetUV(float3 position)
{
    float4 pVP = mul(mul(float4(position, 1.0f), mView), mProjection);
    pVP.xy = float2(0.5f, 0.5f) + float2(0.5f, -0.5f) * pVP.xy / pVP.w;
    return float3(pVP.xy, pVP.z / pVP.w);
}
float3 SSR(float3 texelPosition, float3 reflectDir)
{
    float3 currentRay = 0;

    float3 nuv = 0;
    float L = 0.001;

    for (int i = 0; i < 8; i++)
    {
        currentRay = texelPosition + reflectDir * L;

        nuv = GetUV(currentRay); // проецирование позиции на экран
        if (nuv.x < 0 || nuv.x > 1 || nuv.y < 0 || nuv.y > 1)
            return float3(0, 0, 0);
        float4 NormalSpec = txGB1.Sample(samLinear, nuv.xy);
        float ViewZ = DecodeFloatRG(NormalSpec.zw);
        float3 newPosition = DepthToWorldPos(ViewZ, nuv.xy).xyz;
        L = length(texelPosition - newPosition);
        if (L <= 0.0011)
            return float3(0, 0, 0);
    }
    float error0 = saturate(max(L - 0.011, 0) / 0.088);
    float maxOutScreenRayDist = 0.05;
    float error1 = saturate(max(nuv.x - maxOutScreenRayDist, 0) / maxOutScreenRayDist * 0.25) *
                   saturate(max(nuv.y - maxOutScreenRayDist, 0) / maxOutScreenRayDist * 0.25) *
                   saturate(max(1 - nuv.x + maxOutScreenRayDist, 0) / maxOutScreenRayDist * 0.25) *
                   saturate(max(1 - nuv.y + maxOutScreenRayDist, 0) / maxOutScreenRayDist * 0.25);
    return txGB0.Sample(samLinear, nuv.xy).rgb * error0 * error1;
}
//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT i) : SV_Target
{
    float4 outColor = float4(i.vNormal.xyz,1);
    float3 vNormal = normalize(i.vNormal.xyz);
    float scatter_factor;
    float3 vLightPosition = mViewInv[3].xyz + vSunLightDir.xyz * 1000;
    float3 pixel_to_light_vector = normalize(vLightPosition - i.vWorldPos);
    float3 pixel_to_eye_vector = normalize(mViewInv[3].xyz - i.vWorldPos);
    float2 screenCoords = i.vVPos.xy / float2(fScreenWidth, fScreenHeight);
    float4 NormalSpec = txGB1.Sample(samLinear, screenCoords);
    float3 Normals = normalize(DecodeNormals(NormalSpec.xy));
    float ViewZ = DecodeFloatRG(NormalSpec.zw);
    float shadow_factor = SampleShadowCascades(txShadow,samShadow,samLinear,i.vWorldPos, length(i.vWorldPos.xyz - mViewInv[3].xyz));
    ViewZ = ViewZ <= 0 ? fFarClip : ViewZ;
    
    float3 microbump_normal = normalize(2 * txDiffuse.Sample(samLinear, i.vTexCoord*16).gbr - float3(1, -8, 1));
    microbump_normal += normalize(2 * txDiffuse.Sample(samLinear, i.vTexCoord * 8 + 0.05).gbr - float3(1, -8, 1));
    float3x3 normal_rotation_matrix;
	// calculating base normal rotation matrix
    normal_rotation_matrix[1] = vNormal.xyz;
    normal_rotation_matrix[2] = normalize(cross(float3(0.0, -1.0, 0.0), normal_rotation_matrix[1]));
    normal_rotation_matrix[0] = normalize(cross(normal_rotation_matrix[2], normal_rotation_matrix[1]));

	// applying base normal rotation matrix to high frequency bump normal
    microbump_normal = mul(normalize(microbump_normal), normal_rotation_matrix);

    scatter_factor = 2.5 * max(0, i.vWorldPos.z * 0.25 + 0.25);
    scatter_factor *= shadow_factor*pow(max(0.0, dot(normalize(float3(pixel_to_light_vector.x, 0.0, pixel_to_light_vector.z)), -pixel_to_eye_vector)), 2.0);
    scatter_factor *= pow(max(0.0, 1.0 - dot(pixel_to_light_vector, microbump_normal)), 8.0);
    float g_WaterColorIntensity = 0.2;
    // water crests gather more light than lobes, so more light is scattered under the crests
    scatter_factor += shadow_factor*   1.5 * g_WaterColorIntensity * max(0, i.vWorldPos.z + 1) *
		// the scattered light is best seen if observing direction is normal to slope surface
		max(0, dot(pixel_to_eye_vector, microbump_normal)) *
		// fading scattered light out at distance and if viewing direction is vertical to avoid unnatural look
		max(0, 1 - pixel_to_eye_vector.y) * (300.0 / (300 + length(mViewInv[3].xyz - i.vWorldPos)));

    float r = (1.2 - 1.0) / (1.2 + 1.0);
    float fresnel_factor = max(0.0, min(1.0, r + (1.0 - r) * pow(1.0 - dot(microbump_normal, pixel_to_eye_vector), 4)));

    float diffuse_factor = 0.1 + g_WaterColorIntensity * max(0, dot(pixel_to_light_vector, microbump_normal));
    float3 reflected_eye_to_pixel_vector = -pixel_to_eye_vector + 2 * dot(pixel_to_eye_vector, microbump_normal) * microbump_normal;
    float specular_factor = shadow_factor * fresnel_factor * pow(max(0, dot(pixel_to_light_vector, reflected_eye_to_pixel_vector)), 1000.0);
    float3 refl = SSR(i.vWorldPos, reflected_eye_to_pixel_vector); // +
    //CalculateRayleighScattering(vLightPosition, i.vWorldPos, reflect(-pixel_to_eye_vector, microbump_normal), ViewInv[3].z, i.fDepth) * vSkyLightCol.rgb;
    float waterDepth = ViewZ - i.fDepth;
    waterDepth = max(0, waterDepth);

    // fading refraction color to water color according to distance that refracted ray travels in water 
    float3 refraction_color = txGB0.Sample(samLinear, screenCoords);
    refraction_color = vWaterColor.rgb * lerp(diffuse_factor, refraction_color, min(1, 1.0 * exp(-waterDepth / 8.0)));

    outColor.rgb = lerp(refraction_color, refl, fresnel_factor);
    outColor.rgb += /*g_WaterSpecularIntensity */specular_factor * vSunLightDir.w * fresnel_factor*350;
    outColor.rgb += vSunColor.rgb * scatter_factor * 0.8f * vSunLightDir.w;

    //float3 MieScattering = CalculateMieScattering(vLightPosition, ViewInv[3].xyz, float3(0, 0, -1), ViewInv[3].z, i.fDepth) * vHorizonCol.rgb;
    //float3 RayleighScattering = CalculateRayleighScattering(vLightPosition, ViewInv[3].xyz, -pixel_to_eye_vector, ViewInv[3].z, i.fDepth) * vSkyLightCol.rgb;
    //float3 SunContribution = saturate(pow(max(dot(normalize(vSunLightDir.xyz), -pixel_to_eye_vector), 0), 8)) * vSunColor.rgb * vSunLightDir.w;
    fresnel_factor *= min(1, waterDepth * 5.0);
   // float3 FullScattering = (RayleighScattering + MieScattering + SunContribution);
    
    //outColor.rgb = lerp(outColor.rgb, FullScattering, saturate(max(i.fDepth - fFogStart, 0) / abs(fFogRange)));
    //outColor.xyz = txDiffuse.Sample(samLinear, i.vTexCoord.xy);
    

    return outColor;
}