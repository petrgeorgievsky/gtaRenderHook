#include "GameMath.hlsl"
#include "Shadows.hlsl"
RWTexture3D<float4> voxelVolume: register(u0);
Texture3D voxelNotUAVVolume: register(t0);
RWTexture3D<float4> lightingVolume;
Texture2D txShadow 	: register(t1);

struct Light {
	float3 Position;
	int   m_nLightType;
	float3 m_Color;
	float m_fRange;
};
StructuredBuffer<Light> lightInfo: register(t2);

[numthreads(8, 8, 8)]
void FillVolume(uint3 DTid : SV_DispatchThreadID)
{
    voxelVolume[DTid] = float4(0, 0, 0, 0);
}

[numthreads(4, 4, 4)]
void InjectRadiance(uint3 DTid : SV_DispatchThreadID)
{
	const int LightCount = 32;
	float4 color = voxelNotUAVVolume.Load(int4(DTid, 0));
	float alpha = voxelNotUAVVolume.Load(int4(DTid, 0)).w;
	//float4(0.0f, 0.0f, 0.0f, 0.0f);
	color.xyz *= (SampleShadowMapCompute(txShadow, ConvertFromVoxelSpace(DTid)) * vSunLightDir.w);
	int k = 0;
	for (int i = 0; i < LightCount; i++) {
		int3 loc = (int3)ConvertToVoxelSpace(lightInfo[i].Position);
		float3 _color = lightInfo[i].m_Color;
		if (distance(loc,DTid) < lightInfo[i].m_fRange*voxelGridScale)
		{
			if (distance(loc, DTid) < 1.5f) {
				voxelVolume[DTid] = float4(_color *3,1.0);
				return;
			}
			color += voxelNotUAVVolume.Load(int4(DTid, 0))*float4(_color*max(1 - (distance(loc, DTid) / (lightInfo[i].m_fRange*voxelGridScale)), 0.0),1.0);
			k++;
		}
	}
    //InterlockedAdd()
	voxelVolume[DTid] = float4(color.xyz, alpha);
}
