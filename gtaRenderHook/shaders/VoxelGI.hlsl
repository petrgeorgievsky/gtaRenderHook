//#include "Globals.hlsl"
#include "GameMath.hlsl"
//--------------------------------------------------------------------------------------
// Variables
//--------------------------------------------------------------------------------------

Texture3D<float4> txVGrid : register(t6);
Texture3D<float4> txVGridLod0 : register(t7);
Texture3D<float4> txVGridLod1 : register(t8);
#ifndef SAMLIN
#define SAMLIN
SamplerState samLinear : register(s0);
#endif

#define PI 3.14
 
float3 orthogonal(float3 u) {
	float3 v = float3(0.99146, 0.11664, 0.05832); // Pick any normalized vector.
	return abs(dot(u, v)) > 0.99999f ? cross(u, float3(0, 1, 0)) : cross(u, v);
}

float3 getSkyLightingColor(float3 dir) {
	dir = normalize(dir);
	float sunSpot = max(dot(dir, vSunLightDir.xyz), 0.0f);
	sunSpot = saturate(pow(sunSpot, 16));
	return lerp(vSkyLightCol.xyz, float3(1.5, 1.5, 1.5), sunSpot * vSunLightDir.w) * 0.5f; //;
}

float3 VoxelGI_GetVoxelSpacePos(float3 wpos, float lod)
{
    if (lod <= 1)
    {
        return ConvertToVoxelSpace(wpos, 4 * voxelGridScale2) / voxelGridSize;
    }
    else if (lod <= 2)
    {
        return ConvertToVoxelSpace(wpos, 2 * voxelGridScale2) / voxelGridSize;
    }
    else
    {
        return ConvertToVoxelSpace(wpos, voxelGridScale2) / voxelGridSize;
    }
}

float VoxelGI_GetCorrectLod(float3 wpos, float supposedlod)
{
    if (supposedlod < 1)
    {
        float3 pos = ConvertToVoxelSpace(wpos, 4 * voxelGridScale2) / voxelGridSize;
        if (pos.x > 0.99 || pos.y > 0.99 || pos.z > 0.99 || pos.x < 0.01 || pos.y < 0.01 || pos.z < 0.01)
        {
            pos = ConvertToVoxelSpace(wpos, 2 * voxelGridScale2) / voxelGridSize;
            if (pos.x > 0.99 || pos.y > 0.99 || pos.z > 0.99 || pos.x < 0.01 || pos.y < 0.01 || pos.z < 0.01)
            {
                return 3;
            }
            return 2;
        }
        return 1;
    }
    else if (supposedlod < 2)
    {
        float3 pos = ConvertToVoxelSpace(wpos, 2 * voxelGridScale2) / voxelGridSize;
        if (pos.x > 0.99 || pos.y > 0.99 || pos.z > 0.99 || pos.x < 0.01 || pos.y < 0.01 || pos.z < 0.01)
        {
            return 3;
        }
        return 2;
    }
    else
    {
        return 3;
    }
}

float4 VoxelGI_GetColorAt(float3 pos, float lod)
{
    if (lod < 1)
    {
        return txVGrid.SampleLevel(samLinear, pos,0);
    }
    else if (lod < 2)
    {
        return lerp(txVGrid.SampleLevel(samLinear, pos, 0), txVGridLod0.SampleLevel(samLinear, pos, 0), saturate(lod - 1.0f));
    }
    else
    {
        return lerp(txVGridLod0.SampleLevel(samLinear, pos, 0), txVGridLod1.SampleLevel(samLinear, pos, 0), saturate(lod - 2.0f));
    }
}
float4 VoxelGI_ComputeReflections(float3 wpos, float3 wnorm, float3 direction, float tanHalfAngle) {

	// lod level 0 mipmap is full size, level 1 is half that size and so on
	float lod = 0.0;
	float3 color = float3(0, 0, 0);
	float alpha = 0.0;

    float lodLevel = VoxelGI_GetCorrectLod(wpos, 0);
    float dist = 2.0 / ((float) (1 << (int)floor(lodLevel)) * voxelGridScale2); // Start one voxel away to avoid self occlusion

	float3 startPos = wpos + wnorm*(dist)*2; // Plus move away slightly in the normal direction to avoid
										   // self occlusion in flat surfaces
    
	while (dist < voxelGridSize && alpha < 1) {
		// smallest sample diameter possible is the voxel size
		float diameter = max(1.0, 2.0 * tanHalfAngle * dist);
        lodLevel = min(max(log2(diameter / ((float) (1 << (int) floor(lodLevel)) * voxelGridScale2)), 0), 3);
        lodLevel = VoxelGI_GetCorrectLod(startPos + dist * direction, lodLevel);
        float3 pos = VoxelGI_GetVoxelSpacePos(startPos + dist * direction, lodLevel); //ConvertToVoxelSpace(startPos + dist * direction) / voxelGridSize;
		if (pos.x > 0.99 || pos.y > 0.99 || pos.z > 0.99 || pos.x < 0.01 || pos.y < 0.01 || pos.z < 0.01) {
           // color = getSkyLightingColor(direction);
			break;
		}
        float4 voxelColor = VoxelGI_GetColorAt(pos, lodLevel);
		// front-to-back compositing
		float a = (1.0 - alpha);
		color += a * voxelColor.rgb;
		alpha += a * saturate(voxelColor.a);
        dist += (1.0 / ((float) (1 << (int) floor(lodLevel)) * voxelGridScale2)) * (lodLevel * 0.125f + 1);
        //((lodLevel + 1)); // smoother
												  //dist += diameter; // faster but misses more voxels
	}

	return float4(color, alpha);
}





float4 coneTrace(float3 wpos, float3 wnorm, float3 direction, float tanHalfAngle, out float occlusion)
{

	// lod level 0 mipmap is full size, level 1 is half that size and so on
    float lod = 0.0;
    float3 color = float3(0, 0, 0);
    float alpha = 0.0;
    occlusion = 0.0;
    float lodLevel = VoxelGI_GetCorrectLod(wpos, 0);
    float dist = 2.0 / ((float) (1 << (int) floor(lodLevel)) * voxelGridScale2); // Start one voxel away to avoid self occlusion
    float3 startPos = wpos + wnorm * (dist); // Plus move away slightly in the normal direction to avoid
    float3 dir = direction; // self occlusion in flat surfaces
    float3 normal;
    float3 curPos = startPos + dist * dir;
    float fadingCoeff = 1;
    float lasthitDist = 1.0 / ((float) (1 << (int) floor(lodLevel)) * voxelGridScale2);
    while (dist < voxelGridSize)
    {
		// smallest sample diameter possible is the voxel size
        float diameter = max(1.0, 2.0 * tanHalfAngle * lasthitDist);
        lodLevel = min(max(log2(diameter / ((float) (1 << (int) floor(lodLevel)) * voxelGridScale2)), 0), 3);
        lodLevel = VoxelGI_GetCorrectLod(startPos + dist * direction, lodLevel);
        float3 pos = VoxelGI_GetVoxelSpacePos(curPos, lodLevel); // ConvertToVoxelSpace(curPos) / voxelGridSize;
        if (pos.x > 0.99 || pos.y > 0.99 || pos.z > 0.99 || pos.x < 0.01 || pos.y < 0.01 || pos.z < 0.01)
        {
            color += getSkyLightingColor(dir) * fadingCoeff;
            break;
        }
        float4 voxelColor = VoxelGI_GetColorAt(pos, lodLevel); //txVGrid.SampleLevel(samLinear, pos, lodLevel);

        float a = (1.0 - alpha);
        color += a * voxelColor.rgb * fadingCoeff;
        alpha += a * saturate(voxelColor.a) * fadingCoeff;

        float ll = (lodLevel + 1) * (lodLevel + 1);
        occlusion += (a * saturate(voxelColor.a)) / (1.0 + 0.03 * diameter);
        dist += (ll / ( 1 * voxelGridScale2))* 0.25; // diameter*0.5;
        lasthitDist += (ll / (1 * voxelGridScale2)); //* 0.25;
        curPos += dir *  (ll / (1 * voxelGridScale2)); //* diameter * 0.25;

    }

    return float4(color, alpha);
}
float4 coneTraceRecursion(float3 startPos, float3 direction, float tanHalfAngle, in out float dist)
{

	// lod level 0 mipmap is full size, level 1 is half that size and so on
    float lod = 0.0;
    float3 color = float3(0, 0, 0);
    float alpha = 0.0;

     // Plus move away slightly in the normal direction to avoid
    float3 dir = direction; // self occlusion in flat surfaces
    float3 normal;
    float3 curPos = startPos + dir;
    while (dist < 400 && alpha < 1)
    {
		// smallest sample diameter possible is the voxel size
        float diameter = max(1.0, 2.0 * tanHalfAngle * dist);
        float lodLevel = min(max(log2(diameter / voxelGridScale), 0), max(log2(voxelGridSize) - 2, 0));

        float3 pos = ConvertToVoxelSpace(curPos) / voxelGridSize;
        if (pos.x > 0.99 || pos.y >= 0.99 || pos.z >= 0.99 || pos.x <= 0.01 || pos.y <= 0.01 || pos.z <= 0.01)
        {
            color += getSkyLightingColor(dir);
            break;
        }
        float4 voxelColor = txVGrid.SampleLevel(samLinear, pos, lodLevel);
        if (voxelColor.a > 0.5)
        {
            //normal = txVGridN.SampleLevel(samLinear, pos, lodLevel);
            //dir = normalize(reflect(dir, normal));
            //voxelColor += coneTraceRecursion(curPos, dir, tanHalfAngle, dist)*0.5f;
        }

        float a = (1.0 - alpha);
        color += a * voxelColor.rgb * (lodLevel + 1);
        alpha += a * saturate(voxelColor.a * (lodLevel + 1));

        dist += diameter * (lodLevel + 1); // diameter*0.5;
        curPos += dir * diameter * (lodLevel + 1);

    }

    return float4(color, alpha);
}
#define SQRT2 1.414213
#define ISQRT2 0.707106
#define VOXEL_SIZE (1/32.0)
float3 scaleAndBias(const float3 p)
{
    return 0.5f * p + float3(0.5f, 0.5f, 0.5f);
}

float3 traceDiffuseVoxelCone(const float3 from, float3 direction)
{
    direction = normalize(direction);
	
    const float CONE_SPREAD = 3;

    float4 acc = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float skyVisibility = 1.0;
    float dist = 2.6953125;
    float FarOcclusionStrength = 1.0f;
    float NearOcclusionStrength = 0.6f;
    float FarthestOcclusionStrength = 1.0f;
    float OcclusionStrength =0.69f;
    float OcclusionPower = 1.06f;
    for (int i = 0; i < 8; i++)
    {
        float fi = ((float) i) / 8.0f;
        fi = lerp(fi, 1.0, 0.01);
        float coneDistance = (exp2(fi * 4.0) - 0.9) / 8.0;
        float coneSize = fi * CONE_SPREAD;
        float3 c = from + coneDistance * direction;
        float level = VoxelGI_GetCorrectLod(c, coneSize);
        float4 voxel = VoxelGI_GetColorAt(VoxelGI_GetVoxelSpacePos(c, level), level);
        voxel.a *= lerp(saturate(coneSize / 1.0), 1.0, NearOcclusionStrength);
        skyVisibility *= pow(saturate(1.0 - (voxel.a) * (coneSize * 0.2 * FarOcclusionStrength + 1.0 + coneSize * coneSize * 0.05 * FarthestOcclusionStrength) * OcclusionStrength), lerp(0.014, 1.5 * OcclusionPower, min(1.0, coneSize / 5.0)));
        float falloffFix = pow(fi, 1.0) * 4.0;
        float occlusion = skyVisibility * skyVisibility;
        acc += voxel * (coneSize * 1.0 + 1.0) * occlusion * falloffFix;
    }
	// Trace.
    /*while (dist < SQRT2*8 && acc.a < 1)
    {
        float3 c = from + dist * direction;
        //c = scaleAndBias(from + dist * direction);
        float l = (1 + CONE_SPREAD * dist / VOXEL_SIZE);
        float level = VoxelGI_GetCorrectLod(c, l);
        float ll = (level + 1) * (level + 1);
        
        float4 voxel = VoxelGI_GetColorAt(VoxelGI_GetVoxelSpacePos(c, level), level); //textureLod(texture3D, c, min(MIPMAP_HARDCAP, level));
        acc += 0.075 * ll * voxel * pow(1 - voxel.a, 2);
        dist += ll * VOXEL_SIZE * 2;
    }*/
    return (acc.rgb*0.125f)*0.8f;
}

float3 VoxelGI_GatherLightInfo(float3 pos, float3 normal, out float occl)
{
	const float3 voxelCones[] =
	{
        { 0.5, 0.0, 0.0 }, //{ 0.898904, 0.435512, 0.0479745 },
        { 0.0, 0.5, 0.0 },// { 0.898904, -0.435512, -0.0479745 },
        { 0.0, 0.0, 0.5 }, //{			0.898904, 0.0479745, -0.435512					},
        { 0.7, 0.0, 0.7 }, //{			0.898904, -0.0479745, 0.435512		},
        { 0.7, 0.7, 0.0 }, //{-0.898904, 0.435512, -0.0479745	},
        { 0.0, 0.7, 0.7 }, ////{ -0.898904, -0.435512, 0.0479745 },
    {-0.898904, 0.0479745, 0.435512},
    {-0.898904, -0.0479745, -0.435512},
    {0.0479745, 0.898904, 0.435512},
    {-0.0479745, 0.898904, -0.435512},
    {-0.435512, 0.898904, 0.0479745},
    {0.435512, 0.898904, -0.0479745},
    {-0.0479745, -0.898904, 0.435512},
    {0.0479745, -0.898904, -0.435512},
    {0.435512, -0.898904, 0.0479745},
    {-0.435512, -0.898904, -0.0479745},
    {0.435512, 0.0479745, 0.898904},
    {-0.435512, -0.0479745, 0.898904},
    {0.0479745, -0.435512, 0.898904},
    {-0.0479745, 0.435512, 0.898904},
    {0.435512, -0.0479745, -0.898904},
    {-0.435512, 0.0479745, -0.898904},
    {0.0479745, 0.435512, -0.898904},
    {-0.0479745, -0.435512, -0.898904},
    {0.57735, 0.57735, 0.57735},
    {0.57735, 0.57735, -0.57735},
    {0.57735, -0.57735, 0.57735},
    {0.57735, -0.57735, -0.57735},
    {-0.57735, 0.57735, 0.57735},
    {-0.57735, 0.57735, -0.57735},
    {-0.57735, -0.57735, 0.57735},
    {-0.57735, -0.57735, -0.57735}
};
	float3 startPoint = pos;
	float Distance = length(startPoint);
	float3 radiance = float3(0, 0, 0);
   /* const float ANGLE_MIX = 0.5f; // Angle mix (1.0f => orthogonal direction, 0.0f => direction of normal).

    const float w[3] = { 1.0, 1.0, 1.0 }; // Cone weights.

	// Find a base for the side cones with the normal as one of its base vectors.
    const float3 ortho = normalize(orthogonal(normal));
    const float3 ortho2 = normalize(cross(ortho, normal));

	// Find base vectors for the corner cones too.
    const float3 corner = 0.5f * (ortho + ortho2);
    const float3 corner2 = 0.5f * (ortho - ortho2);

	// Find start position of trace (start with a bit of offset).
    const float3 N_OFFSET = normal * (1 + 4 * ISQRT2) * VOXEL_SIZE;
    const float3 C_ORIGIN = pos + N_OFFSET;

	// Accumulate indirect diffuse light.
    float3 acc = float3(0,0,0);

	// We offset forward in normal direction, and backward in cone direction.
	// Backward in cone direction improves GI, and forward direction removes
	// artifacts.
    const float CONE_OFFSET = -0.01;

	// Trace front cone
    acc += w[0] * traceDiffuseVoxelCone(C_ORIGIN + CONE_OFFSET * normal, normal);

	// Trace 4 side cones.
    const float3 s1 = lerp(normal, ortho, ANGLE_MIX);
    const float3 s2 = lerp(normal, -ortho, ANGLE_MIX);
    const float3 s3 = lerp(normal, ortho2, ANGLE_MIX);
    const float3 s4 = lerp(normal, -ortho2, ANGLE_MIX);

    acc += w[1] * traceDiffuseVoxelCone(C_ORIGIN + CONE_OFFSET * ortho, s1);
    acc += w[1] * traceDiffuseVoxelCone(C_ORIGIN - CONE_OFFSET * ortho, s2);
    acc += w[1] * traceDiffuseVoxelCone(C_ORIGIN + CONE_OFFSET * ortho2, s3);
    acc += w[1] * traceDiffuseVoxelCone(C_ORIGIN - CONE_OFFSET * ortho2, s4);

	// Trace 4 corner cones.
    const float3 c1 = lerp(normal, corner, ANGLE_MIX);
    const float3 c2 = lerp(normal, -corner, ANGLE_MIX);
    const float3 c3 = lerp(normal, corner2, ANGLE_MIX);
    const float3 c4 = lerp(normal, -corner2, ANGLE_MIX);

    acc += w[2] * traceDiffuseVoxelCone(C_ORIGIN + CONE_OFFSET * corner, c1);
    acc += w[2] * traceDiffuseVoxelCone(C_ORIGIN - CONE_OFFSET * corner, c2);
    acc += w[2] * traceDiffuseVoxelCone(C_ORIGIN + CONE_OFFSET * corner2, c3);
    acc += w[2] * traceDiffuseVoxelCone(C_ORIGIN - CONE_OFFSET * corner2, c4);
    */
	float3 tangent = orthogonal(normal);

	tangent = normalize(tangent);

	float3 bitangent = normalize(cross(tangent, normal));
	occl=0;
	float occ=0;
    const float phi = 1.618033988;
    const float gAngle = phi * PI * 1.0;

	const int gisamples = 6;
	const float tan60 = tan(PI / 9);
    for (int i = 0; i < 16; i++)
    {
        float fi = (float) i;
        float fiN = fi / 16.0f;

        float longitude = gAngle * fi;
        float latitude = asin(fiN * 2.0 - 1.0);
					
        float3 kernel;
        kernel.x = cos(latitude) * cos(longitude);
        kernel.z = cos(latitude) * sin(longitude);
        kernel.y = sin(latitude);
					
        kernel = normalize(kernel + normal.xyz * 1.0);

        float cosTheta = dot(kernel, normal.xyz);
        
		//if (cosTheta < 0.0)
		//	continue;
		//float3 rndDir = CosineSampleHemisphere(((i) / (gisamples / 2)) / ((float) gisamples), ((i) % (gisamples / 2)) / ((float) gisamples));
        radiance += traceDiffuseVoxelCone(pos, kernel + normal.xyz * 1.25f) * max(cosTheta, 0);
		occl += occ * max(cosTheta, 0);
	}
	occl = 1 - occl;
    occl = 1;
    return radiance/16.0;
}

// Draws basic voxel structure
float3 VoxelGI_DebugVoxels(float3 worldPos)
{
    float3 pos = ConvertToVoxelSpace(worldPos, 4 * voxelGridScale2) / voxelGridSize;
    return txVGrid.Sample(samLinear, pos).rgb;
}