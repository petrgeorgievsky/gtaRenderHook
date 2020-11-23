#version 460
#extension GL_NV_ray_tracing : require

struct hitPayload
{
  vec3 hitValue;
};

layout(location = 0) rayPayloadInNV vec3 hitValue;

layout(binding = 4, set = 0) uniform SkyCfg
{
  vec4 sunDir;
  vec4 sunColor;
  vec4 horizonColor;
  vec4 skyColor;
}sky_cfg;

const float g_fAtmosphereHeight = 8000.0f;
const float g_fThickAtmosphereHeight = 1200.0f;

/*!
	Rayleigh density at given height
*/
float GetRayleighDensity(float Height)
{
    return exp(-Height / g_fAtmosphereHeight);
}

/*!
	Mie density at given height
*/
float GetMieDensity(float Height)
{
    return exp(-Height / g_fThickAtmosphereHeight);
}

vec3 CalculateRayleighScattering(float CosSqPlOne, float Height)
{
    // Rayleigh phase function approximation
    float phase = 0.75f * CosSqPlOne;
    // Rayleigh density approximation at current camera height
    float density = GetRayleighDensity(Height);
    // Rayleigh light scattering approximation(very rough)
    return (density * phase).xxx;
}

vec3 CalculateMieScattering(float CosPhi, float CosSqPlOne, float Height)
{
    // Assymetry factor for mie phase function
    const float g = -0.85;
    // Henyey-Greenstein mie phase function approximation
    float MiePhase = 1.5f * ((1 - g * g) / (2 + g * g)) * (CosSqPlOne / pow(1 + g * g - 2 * g * CosPhi, 1.5f));
    float MieDensity = GetMieDensity(Height);
    // Mie light scattering approximation(very rough)
    return (MieDensity * MiePhase).xxx;
}

vec3 CalculateFullScatter(vec3 ViewDir, vec3 LightDir, float Height)
{
	// Sky top			Sky bot		SunCore 		SunCorona   SunSz
	// 90 205 255   	187 184 178	255 128 0    	255 128 0   2.5
	vec3 vHorizonCol = sky_cfg.horizonColor.xyz;//vec3(187.0/255.0, 184.0/255.0, 178.0/255.0);
	vec3 vSkyLightCol = sky_cfg.skyColor.xyz;//vec3(90.0/255.0, 205.0/255.0, 255.0/255.0);
	vec3 vSunColor = sky_cfg.sunColor.xyz;//vec3(255.0/255.0, 128.0/255.0, 0.0);

    float CosPhi = max(dot(ViewDir, LightDir), 0.0);
    float CosSqPlOne = (1 + CosPhi * CosPhi);

    float MieCosPhi = max(-LightDir.z, 0.0);
    float MieCosSqPlOne = (1 + MieCosPhi * MieCosPhi);

    vec3 SkyColor = mix(vHorizonCol.rgb, vSkyLightCol.rgb, min(max(Height + 300.0f, 0.0f) / 600.0f, 1.0f));

    vec3 MieScattering = CalculateMieScattering(MieCosPhi, MieCosSqPlOne, Height) * vSunColor.rgb;
    vec3 RayleighScattering = CalculateRayleighScattering(CosSqPlOne, Height) * SkyColor;

    vec3 SunContribution = min(pow(CosPhi, 8.0f), 1.0f) * vSunColor.rgb;
    return ((RayleighScattering * 0.75f + MieScattering * 0.5f) + SunContribution * 0.5f);
}

vec3 GetSkyColor(vec3 ViewDir, vec3 LightDir, vec3 FullScattering)
{
	vec3 vSunColor = vec3(1,1,1)*2.0f;
    float CosPhi = max(dot(ViewDir, LightDir), 0.0);

    // Compute sky color at skydome by blending scattering with sun disk
    vec3 SunDiskColor = max(FullScattering, vSunColor.rgb);
    float SunDiskCoeff = CosPhi>0.9999f?1.0f:0.0f;

    return mix(FullScattering, SunDiskColor, SunDiskCoeff);
}

void main()
{
    vec3 sun_dir = sky_cfg.sunDir.xyz;//normalize(vec3(0,-100,20));
    hitValue = GetSkyColor(gl_WorldRayDirectionNV, sun_dir, CalculateFullScatter(gl_WorldRayDirectionNV, sun_dir, (gl_WorldRayOriginNV + gl_WorldRayDirectionNV * 1000.0f).z));//vec3(0.0, 0.1, 0.3);
}