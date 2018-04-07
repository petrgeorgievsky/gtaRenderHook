#include "Globals.hlsl"
#ifndef SHADOWS_HLSL
#define SHADOWS_HLSL

#ifndef SHADOW_BLUR_KERNEL
#define SHADOW_BLUR_KERNEL 2
#endif
cbuffer ShadowBuffer : register(b4)
{
	row_major matrix DirLightViewProj[4];
	float4 FadeDistances;
	float MaxDrawDistance;
	int ShadowSize;
    float2 _pad;
	float4 ShadowBias;
}

// Samples shadow texture using poisson sampling. Approximatly uses (SHADOW_BLUR_KERNEL*2+1)^2 samples(perhaps that's way too much).
float poissonShadowSampling(Texture2D txShadow, SamplerComparisonState samShadow, float2 ShadowCoord, float Z,float poissonScale) 
{
	float visibility=0.0;
	float offsetScale = poissonScale / (float)ShadowSize;
	// Simple box 7x7 blur-dependent kernel. 
	// TODO: Implement ability to change kernel minimum and maximum size and more kernels for ex. gaussian or
	//		 user defined with custom matrix or ability to choose blur-independent kernel.
    int kernelSize = min(ceil(poissonScale), SHADOW_BLUR_KERNEL); // Here kernelSize is half the actual kernel size
#if BLUR_SHADOWS==1
    int count		 = (kernelSize*2+1)*(kernelSize*2+1);
	for (int x = -kernelSize;x<=kernelSize; x++) // (kernelSize*2+1)^2 comparisons and 3*(kernelSize*2+1)^2 ALU per cycle ~ o(kernelSize^2) complexity
	for (int y = -kernelSize;y<=kernelSize; y++)
	{
		visibility += txShadow.SampleCmpLevelZero(samShadow, ShadowCoord + float2(x, y) * offsetScale, Z);
	}
    return visibility/count;
#else
    return txShadow.SampleCmpLevelZero(samShadow, ShadowCoord, Z);
#endif	
}

//reciever's coordinate on shadow map, reciever's depth, shadow map sampler, current layer of the shadow map array sampler
float AvarageShadowCasterDepth(float2 ShadowCoord, float Depth,SamplerState samShadow, Texture2D txShadow)
{
	float RetValue=0.0f;
	int Count=0;
	int R=6;				//Radius of the blocker search
	const int start=-R/2;
	const int end=start+R;
	const float offsetScale = 1.0f / (ShadowSize);
	for (int IS=0;IS<4;IS++)//cause a discrete search
	for (int JS=0;JS<4;JS++)
	{
		for (int i=start+IS;i<end;i+=4)
		for (int j=start+JS;j<R;  j+=4)
		{
#if FEATURE_LEVEL>0xa100
			const float4 SamplesDepth = txShadow.Gather(samShadow, float2(ShadowCoord + float2(i, j) * offsetScale));
			for (int k=0;k<4;k++)
			if (SamplesDepth[k]<Depth)
			{
				RetValue+=SamplesDepth[k];
				Count++;
			}
#else
			const float SampleDepth = txShadow.Sample(samShadow, float2(ShadowCoord + float2(i, j) * offsetScale)).r;
			if (SampleDepth<Depth)
			{
				RetValue+=SampleDepth;
				Count++;
			}
#endif
			if (Count>16)//early fail
				return RetValue/Count;
		}
	}

	return (Count!=0)?RetValue/Count:0.0f;
}

// Samples shadow texture.
float SampleShadowMap(Texture2D txShadow, SamplerComparisonState samShadow,SamplerState samShadowNonComp, float3 LightViewPos, float2 offset, int i) 
{
	float2	ShadowTexCoord 		= 0.5 * LightViewPos.xy + float2(0.5, 0.5);
			ShadowTexCoord.y 	= 1.0f - ShadowTexCoord.y;
	// early fail
	if (ShadowTexCoord.x < 0 || ShadowTexCoord.x > 1 || ShadowTexCoord.y < 0 || ShadowTexCoord.y > 1)
		return 1;
	// calculate shadow coordinate(because we use 4 cascades, we need to get 4 texcoord spaces each is half of original space plus offset)
	ShadowTexCoord *= 0.5; 
	ShadowTexCoord += offset; 
    float blurScale;
#if USE_PCS_SHADOWS==1&&BLUR_SHADOWS==1
	float avgRecieverDepth 	= txShadow.Sample(samShadowNonComp, ShadowTexCoord);
	float avgCasterDepth   	= AvarageShadowCasterDepth(ShadowTexCoord,LightViewPos.z,samShadowNonComp,txShadow);
    
    blurScale = (LightViewPos.z - avgCasterDepth) / avgCasterDepth;
    blurScale = max(blurScale * MaxShadowBlur, MinShadowBlur);
#else
    blurScale = MaxShadowBlur;
#endif

    return poissonShadowSampling(txShadow, samShadow, ShadowTexCoord, LightViewPos.z - ShadowBias[i], blurScale); //(LightViewPos.z - sSample < 0.001);
}

// Samples shadow texture without any blurring.
float SampleShadowMapUnfiltered(Texture2D txShadow, SamplerComparisonState samShadow, SamplerState samShadowNonComp, float3 LightViewPos, float2 offset, int i)
{
    float2 ShadowTexCoord = 0.5 * LightViewPos.xy + float2(0.5, 0.5);
    ShadowTexCoord.y = 1.0f - ShadowTexCoord.y;
	// early fail
    if (ShadowTexCoord.x < 0 || ShadowTexCoord.x > 1 || ShadowTexCoord.y < 0 || ShadowTexCoord.y > 1)
        return 1;
	// calculate shadow coordinate(because we use 4 cascades, we need to get 4 texcoord spaces each is half of original space plus offset)
    ShadowTexCoord *= 0.5;
    ShadowTexCoord += offset;
	
    return txShadow.SampleCmpLevelZero(samShadow, ShadowTexCoord, LightViewPos.z - ShadowBias[i]);
}

// Samples shadow texture for compute shaders.
float SampleShadowMapCompute(Texture2D txShadow, float3 LightViewPos) {
	LightViewPos = mul(float4(LightViewPos, 1), DirLightViewProj[0]).xyz;
	float3 projectTexCoord;
	projectTexCoord = LightViewPos;
	float2	ShadowTexC = 0.5 * projectTexCoord.xy + float2(0.5, 0.5);
	ShadowTexC.y = 1.0f - ShadowTexC.y;
    float sSample = txShadow.Load(int3(ShadowTexC.x * ShadowSize, ShadowTexC.y * ShadowSize, 0));
	return (LightViewPos.z - sSample < ShadowBias[0]*8);
}

// Samples cascade shadow maps, blended 
float SampleShadowCascades(Texture2D txShadow, SamplerComparisonState samShadow, SamplerState samShadowNonComp, float3 worldPos, float viewZ)
{
	// early fail
	if(vSunLightDir.w<=0)
		return 0;
	// TODO: make blend distance customizible, perhaps different for each cascade.
	const float blendDistance=6; 
	// Shift view depth by blend distance, optimization for static blend distance
	viewZ+=blendDistance;
	static const float2 shadowTCOffsets[]={
		float2(0.0f,0.0f),
		float2(0.5f,0.0f),
		float2(0.0f,0.5f),
		float2(0.5f,0.5f)
	};
    float Shadow = 1;
    float blendFactor;
    float far, near;
    uint i;
    for (i = 0; i < 3; i++)
    {
        float4 LightViewPos = mul(float4(worldPos.xyz, 1), DirLightViewProj[i]);
        far = FadeDistances[i + 1];
        near = FadeDistances[i];
		// Cascade blending factor
        blendFactor = min(max(viewZ - near, 0) / blendDistance, 1.0);
        Shadow = lerp(Shadow, SampleShadowMap(txShadow, samShadow, samShadowNonComp, LightViewPos.xyz, shadowTCOffsets[i], i), blendFactor);
    }
	// Last cascade
    float4 LightViewPos = mul(float4(worldPos.xyz, 1), DirLightViewProj[3]);
    far = MaxDrawDistance;
    near = FadeDistances[3];
    blendFactor = min(max(viewZ - near, 0) / blendDistance, 1.0);
	
    Shadow = lerp(Shadow, SampleShadowMap(txShadow, samShadow, samShadowNonComp, LightViewPos.xyz, shadowTCOffsets[3], 3), blendFactor);

    return max(Shadow, 1 - vSunLightDir.w);
}
// Samples cascade shadow maps, blended 
float SampleShadowCascadesUnfiltered(Texture2D txShadow, SamplerComparisonState samShadow, SamplerState samShadowNonComp, float3 worldPos, float viewZ)
{
	// early fail
    if (vSunLightDir.w <= 0)
        return 0;
	// TODO: make blend distance customizible, perhaps different for each cascade.
    const float blendDistance = 6;
	// Shift view depth by blend distance, optimization for static blend distance
    viewZ += blendDistance;
    static const float2 shadowTCOffsets[] =
    {
        float2(0.0f, 0.0f),
		float2(0.5f, 0.0f),
		float2(0.0f, 0.5f),
		float2(0.5f, 0.5f)
    };
    float Shadow = 1;
    float blendFactor;
    float far, near;
    uint i;
    for (i = 0; i < 3; i++)
    {
        float4 LightViewPos = mul(float4(worldPos.xyz, 1), DirLightViewProj[i]);
        far = FadeDistances[i + 1];
        near = FadeDistances[i];
		// Cascade blending factor
        blendFactor = min(max(viewZ - near, 0) / blendDistance, 1.0);
        Shadow = lerp(Shadow, SampleShadowMapUnfiltered(txShadow, samShadow, samShadowNonComp, LightViewPos.xyz, shadowTCOffsets[i], i), blendFactor);
    }
	// Last cascade
    float4 LightViewPos = mul(float4(worldPos.xyz, 1), DirLightViewProj[3]);
    far = MaxDrawDistance;
    near = FadeDistances[3];
    blendFactor = min(max(viewZ - near, 0) / blendDistance, 1.0);
	
    Shadow = lerp(Shadow, SampleShadowMapUnfiltered(txShadow, samShadow, samShadowNonComp, LightViewPos.xyz, shadowTCOffsets[3], 3), blendFactor);

    return max(Shadow, 1 - vSunLightDir.w);
}
#endif