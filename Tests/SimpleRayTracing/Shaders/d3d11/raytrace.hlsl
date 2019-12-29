RWTexture2D<float4> tResult : register(u0);
cbuffer SceneConstants : register(b0)
{
    float screen_width;
    float screen_height;
    uint sphere_count;
    uint tri_count;
};
cbuffer SceneVariables : register(b1)
{
    float random_a;
    float random_b;
    uint random_a_ui;
    uint random_b_ui;
    float4 camPos;
    float4 lightPos;
};

struct Sphere
{
    float3 position;
    float radius;
    float4 color;
};
struct Triangle
{
    float3 v0;
    float3 v1;
    float3 v2;
    float3 color;
};

struct Triangle_v2
{
    uint4 ids;
};

struct BVHLeaf
{
    float3 aabb_min;
    int subnode_right;
    float3 aabb_max;
    int subnode_left;
};

StructuredBuffer<Sphere> bufSphereList : register(t0);
StructuredBuffer<float4> bufVertexList : register(t2);
StructuredBuffer<Triangle_v2> bufTriList : register(t1);
StructuredBuffer<BVHLeaf> bufAABBList : register(t3);
StructuredBuffer<uint> bufTriLists : register(t4);
Texture2D gb0 : register(t5);
Texture2D gb1 : register(t6);
SamplerState s0 : register(s0);

struct Ray
{
    float4 origin;
    float4 direction;
    float4 invdirection;
};
#define MAX_SPHERES 4
#define MAX_TRIANGLES 2

struct Scene
{
    Sphere spheres[MAX_SPHERES];
    Triangle tris[MAX_TRIANGLES];
};

struct PayLoad
{
    float3 normal;
    float hitDist;
    float4 color;
};
static const float3 skyColor = float3(0.5f, 0.7f, 1.0f);
static const float3 botColor = float3(0.5f, 0, 0);

bool RaySphereIntersection(Ray r, Sphere s, out float res_distance)
{
    float t0, t1, temp;
    res_distance = -1;
	
	//dot(O+D*t-C,O+D*t-C)=R^2
	//t*t*dot(D,D)+2*t*dot(O-C,O-C)+dot(C,C)-R*R=0
	
    float3 vFromSphereDir = r.origin.xyz - s.position;
    float a = dot(r.direction.xyz, r.direction.xyz);
    float b = dot(vFromSphereDir, r.direction.xyz) * 2.0f;
    float c = dot(vFromSphereDir, vFromSphereDir) - s.radius * s.radius;
    float D = b * b - 4 * a * c;
	
    if (D < 0)
        return false;
    res_distance = (-b - sqrt(D)) / 2.0f * a;
    if (res_distance < 0)
        return false;
    return true;

}

bool RayTriangleIntersection(Ray r, Triangle_v2 tv2, out PayLoad payLoad)
{
    float3 v0 = bufVertexList[tv2.ids.x].xyz;
    float3 v1 = bufVertexList[tv2.ids.y].xyz;
    float3 v2 = bufVertexList[tv2.ids.z].xyz;
    //t.color
    float3 e1 = v1 - v0;
    float3 e2 = v2 - v0;
    // normal calc
    float3 pvec = cross(r.direction.xyz, e2);
    float det = dot(e1, pvec);

    // Ray is parallel
    if (det < 1e-8 && det > -1e-8)
    {
        return false;
    }

    float inv_det = 1 / det;
    float3 tvec = r.origin.xyz - v0;
    float u = dot(tvec, pvec) * inv_det;
    if (u < 0 || u > 1)
    {
        return false;
    }

    float3 qvec = cross(tvec, e1);
    float v = dot(r.direction.xyz, qvec) * inv_det;
    if (v < 0 || u + v > 1)
    {
        return false;
    }
    payLoad.hitDist = dot(e2, qvec) * inv_det;
    payLoad.normal = normalize(cross(e1, e2));
    payLoad.color = float4(1, 1, 1, 1);
    if (payLoad.hitDist < 0)
        return false;
    return true;
}


bool RayAABBIntersection(Ray r, float3 bbmin, float3 bbmax, out PayLoad payLoad)
{
    payLoad.hitDist = distance(r.origin.xyz, (bbmax + bbmin)*0.5f);
    payLoad.normal = float3(0,0,0);
    payLoad.color = float4(((bbmin + bbmax) * 0.5), 0.8);
    /*
    float tmax=1000, tmin=0;
    for ( int i = 0; i < 3; i++ )
    {
        float invD = r.invdirection[i];
        float t0 = ( bbmin[i] - r.origin[i] ) * invD;
        float t1 = ( bbmax[i] - r.origin[i] ) * invD;
        if ( invD < 0.0f )
        {
            float temp = t0;
            t0 = t1;
            t1 = temp;
        }
        tmin = t0 > tmin ? t0 : tmin;
        tmax = t1 < tmax ? t1 : tmax;
        if( tmax < tmin )
            return false;
    }
    payLoad.hitDist = min(tmin, tmax);*/
    const float3 t0 = (bbmin - r.origin.xyz) * r.invdirection.xyz;
    const float3 t1 = (bbmax - r.origin.xyz) * r.invdirection.xyz;
 
    const float3 tmax = max(t0, t1);
    const float3 tmin = min(t0, t1);
 
    const float a1 = min(tmax.x, min(tmax.y, tmax.z));
    const float a0 = max(max(tmin.x, tmin.y), max(tmin.z, 0.0f));
 
    if (a1 < a0)
        return false;
    payLoad.hitDist = min(a0, a1);
    return true;
}

bool TraverseScene(Ray r, out PayLoad payLoad)
{
    float heightLerp = 1 - min(normalize(r.origin + r.direction).y, 1);
    float3 skyContrib = lerp(float3(1, 1, 1), skyColor, heightLerp);

    payLoad.hitDist = 1000;
    payLoad.color.rgb = skyContrib;
    payLoad.color.a = 1;
    bool hit = false;

    for (uint i = 0; i < sphere_count; i++)
    {
        float resDist = 10000;
        float3 hitPoint;
        if (RaySphereIntersection(r, bufSphereList[i], resDist))
        {
            if (resDist < payLoad.hitDist)
            {
                payLoad.hitDist = resDist;
                hitPoint = r.origin.xyz + r.direction.xyz * resDist;
                payLoad.normal = normalize(hitPoint - bufSphereList[i].position);
                payLoad.color = bufSphereList[i].color;
            }
            hit = true;
        }
    }

    for (uint j = 0; j < tri_count; j++)
    {
        float resDist = 10000;
        float3 hitPoint;
        PayLoad payLoadCurr;
        payLoadCurr.hitDist = payLoad.hitDist;
        float3 minAABB, maxAABB;
        minAABB.x = min(min(bufVertexList[bufTriList[j].ids.x].x, bufVertexList[bufTriList[j].ids.y].x), bufVertexList[bufTriList[j].ids.z].x);
        minAABB.y = min(min(bufVertexList[bufTriList[j].ids.x].y, bufVertexList[bufTriList[j].ids.y].y), bufVertexList[bufTriList[j].ids.z].y);
        minAABB.z = min(min(bufVertexList[bufTriList[j].ids.x].z, bufVertexList[bufTriList[j].ids.y].z), bufVertexList[bufTriList[j].ids.z].z);

        maxAABB.x = max(max(bufVertexList[bufTriList[j].ids.x].x, bufVertexList[bufTriList[j].ids.y].x), bufVertexList[bufTriList[j].ids.z].x);
        maxAABB.y = max(max(bufVertexList[bufTriList[j].ids.x].y, bufVertexList[bufTriList[j].ids.y].y), bufVertexList[bufTriList[j].ids.z].y);
        maxAABB.z = max(max(bufVertexList[bufTriList[j].ids.x].z, bufVertexList[bufTriList[j].ids.y].z), bufVertexList[bufTriList[j].ids.z].z);

        if (RayAABBIntersection(r, minAABB, maxAABB, payLoadCurr))
        {
            if (payLoadCurr.hitDist < payLoad.hitDist)
            {
                payLoad = payLoadCurr;
            }
            hit = true;
        }
    }
    return hit;
}
bool TraverseSceneBVHLeaf(Ray r, BVHLeaf leaf, out PayLoad payLoad)
{
    if (leaf.subnode_left == leaf.subnode_right)
    {
        return RayAABBIntersection(r, leaf.aabb_min, leaf.aabb_max, payLoad);
    }
    if (RayAABBIntersection(r, bufAABBList[leaf.subnode_left].aabb_min, bufAABBList[leaf.subnode_left].aabb_max, payLoad))
        return TraverseSceneBVHLeaf(r, bufAABBList[leaf.subnode_left], payLoad);
    if (RayAABBIntersection(r, bufAABBList[leaf.subnode_right].aabb_min, bufAABBList[leaf.subnode_right].aabb_max, payLoad))
        return TraverseSceneBVHLeaf(r, bufAABBList[leaf.subnode_right], payLoad);
    return false;
}
bool TraverseSceneBVH(Ray r, out PayLoad payLoad, float maxDist)
{
    float heightLerp = max(min(normalize(r.origin + r.direction).y*0.5f + 1.0f, 1),0);
    float3 skyContrib = lerp(botColor, skyColor, heightLerp);
    payLoad.hitDist = maxDist;
    payLoad.color.rgb = skyContrib;
    payLoad.color.a = 1;

    PayLoad payLoadCurr;
    payLoadCurr.hitDist = payLoad.hitDist;
    
    BVHLeaf currentLeaf = bufAABBList[0];
    int currentDepth = 0;
    int maxDepth = 1000;
    int intersectingNodeCount = 1;
    int intersectingNodes[32];
    intersectingNodes[0] = 0;
    bool hit = false;
    while (intersectingNodeCount>0)
    {
        //if (currentDepth > 128)
        //    return false;
        currentLeaf = bufAABBList[intersectingNodes[intersectingNodeCount-1]];
        intersectingNodeCount--;
        if (currentLeaf.subnode_left < 0 || currentLeaf.subnode_right < 0)
        {
            for (int i = -currentLeaf.subnode_left-1; i < -currentLeaf.subnode_left - currentLeaf.subnode_right-1; i++)
            {
                if (RayTriangleIntersection(r, bufTriList[bufTriLists[i]], payLoadCurr))
                {
                    if (payLoadCurr.hitDist < payLoad.hitDist && payLoadCurr.hitDist > 0.01f)
                    {
                        payLoad = payLoadCurr;
                       /* payLoad.color.r = currentDepth > 256;
                        payLoad.color.g = currentDepth > 128 && currentDepth <= 256;
                        payLoad.color.b = currentDepth <= 16;*/
                    }
                    hit = true;
                }
            }
        }
        else
        {
            if (RayAABBIntersection(r, bufAABBList[currentLeaf.subnode_right].aabb_min, bufAABBList[currentLeaf.subnode_right].aabb_max, payLoadCurr))
            {
                if (payLoadCurr.hitDist < payLoad.hitDist)
                {
                    intersectingNodes[intersectingNodeCount] = currentLeaf.subnode_right;
                    intersectingNodeCount++;
                    currentDepth++;
                }
            }
            if (RayAABBIntersection(r, bufAABBList[currentLeaf.subnode_left].aabb_min, bufAABBList[currentLeaf.subnode_left].aabb_max, payLoadCurr))
            {
                if (payLoadCurr.hitDist < payLoad.hitDist)
                {
                    intersectingNodes[intersectingNodeCount] = currentLeaf.subnode_left;
                    intersectingNodeCount++;
                    currentDepth++;
                }
            }
        }
    }
    return hit;
}

bool DebugSceneBVH(Ray r, out PayLoad payLoad, float maxDist)
{
    
    float heightLerp = 1 - min(normalize(r.origin + r.direction).y, 1);
    float3 skyContrib = lerp(botColor, skyColor, heightLerp);
    payLoad.hitDist = maxDist;
    payLoad.color.rgb = skyContrib;
    payLoad.color.a = 1;

    PayLoad payLoadCurr;
    payLoadCurr.hitDist = payLoad.hitDist;
    
    BVHLeaf currentLeaf = bufAABBList[0];
    int currentDepth = 0;
    int maxDepth = 128;
    int intersectingNodeCount = 1;
    int intersectingNodes[32];
    intersectingNodes[0] = 0;
    bool hit = false;
    while (intersectingNodeCount > 0)
    {
        currentLeaf = bufAABBList[intersectingNodes[intersectingNodeCount - 1]];
        if (currentDepth > maxDepth)
        {
            return RayAABBIntersection(r, currentLeaf.aabb_min, currentLeaf.aabb_max, payLoad);
        }
        intersectingNodeCount--;
        if (currentLeaf.subnode_left > 0 && currentLeaf.subnode_right > 0)
        {
            if (RayAABBIntersection(r, bufAABBList[currentLeaf.subnode_right].aabb_min, bufAABBList[currentLeaf.subnode_right].aabb_max, payLoadCurr))
            {
                intersectingNodes[intersectingNodeCount] = currentLeaf.subnode_right;
                intersectingNodeCount++;
                currentDepth++;
            }
            if (RayAABBIntersection(r, bufAABBList[currentLeaf.subnode_left].aabb_min, bufAABBList[currentLeaf.subnode_left].aabb_max, payLoadCurr))
            {
                intersectingNodes[intersectingNodeCount] = currentLeaf.subnode_left;
                intersectingNodeCount++;
                currentDepth++;
            }
        }
    }
    return hit;
}

float rand_1_05(in float2 uv)
{
    float2 noise = (frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453));
    return abs(noise.x + noise.y) * 0.5;
}
float rand_1_06(in float2 uv)
{
    float2 noise = (frac(sin(dot(uv, float2(22.9898, 32.233) * 2.0)) * 64564.5453));
    return abs(noise.x + noise.y) * 0.5;
}
float rand_1_07(in float2 uv)
{
    float2 noise = (frac(sin(dot(uv, float2(56.9898, 85.233) * 2.0)) * 12346.5453));
    return abs(noise.x + noise.y) * 0.5;
}
float3 randomPointOnUnitSphere(float2 seed)
{
    float3 randomPointOnSphere;
    float theta = 2 * 3.14f * rand_1_05(seed.xy);
    float phi = acos(1 - 2 * rand_1_06(seed.xy));
    float x = sin(phi) * cos(theta);
    float y = sin(phi) * sin(theta);
    float z = cos(phi);
    randomPointOnSphere = normalize(float3(x, y, z));
    return randomPointOnSphere;
}

bool CastShadow(float3 pos, float4 lightPos, out PayLoad shadowPayLoad, float maxDist)
{
    if (distance(lightPos.xyz, pos) <= lightPos.w)
        return false;
    Ray shadowRay;
    shadowRay.origin = float4(pos, 0);
    shadowRay.direction = float4(normalize(lightPos.xyz - pos), 0);
    shadowRay.invdirection = float4(1.0f / shadowRay.direction.x, 1.0f / shadowRay.direction.y, 1.0f / shadowRay.direction.z, 1);

    return TraverseSceneBVH(shadowRay, shadowPayLoad, maxDist);
}



[numthreads(8, 8, 1)]
void TraceRays(uint3 DTid : SV_DispatchThreadID)
{
    float3 light = lightPos.xyz;
    float lightSize = lightPos.w;

    float3 lowerLeft = float3(-0.5f, 0.5f, 1.0f);
    float3 u = float3(1.33f, 0.0f, 0.0f);
    float3 v = float3(0.0f, -1.0f, 0.0f);
	
    Ray r;
    r.origin = camPos;
    r.direction = float4(lowerLeft + ((float) DTid.x) / screen_width  * u +
									 ((float) DTid.y) / screen_height * v, 0);
    r.direction.xyz = normalize(r.direction.xyz);
    r.invdirection = float4(1.0f / r.direction.x, 1.0f / r.direction.y, 1.0f / r.direction.z, 1);

    float dist;

    PayLoad payLoad;
    PayLoad shadowPayLoad;
    

    float4 resultColor = float4(lerp(botColor, skyColor, 1 - DTid.y / screen_height), 1);


    //if (TraverseSceneBVH(r, payLoad, 1000))
    {
        float4 hitPoint = gb0.Load(DTid); //r.origin.xyz + r.direction.xyz * payLoad.hitDist /*+ payLoad.normal * 0.01f*/;
        if (hitPoint.a<=0)
        {
            tResult[DTid.xy] = resultColor;
            return;
        }
        //tResult[DTid.xy] = (r.origin.xyz + r.direction.xyz * payLoad.hitDist).rgbb;
        //return;
            payLoad.normal = -gb1.Load(DTid);
            float3 lightSample = light + randomPointOnUnitSphere(hitPoint.xy * float2(random_a, random_b)) * lightSize;
        
        
            int maxSIter = 1;
            float shadow = 0;
            float ndl = max(dot(normalize(light - hitPoint.xyz), payLoad.normal), 0);
            while (maxSIter > 0 && ndl > 0.05)
            {
                bool inShadow = CastShadow(hitPoint.xyz + payLoad.normal * 0.01f, float4(lightSample, lightSize), shadowPayLoad, 1000.0f); //TraverseScene(shadowRay, s, shadowPayLoad);
                shadow += !inShadow;

                lightSample = light + randomPointOnUnitSphere(lightSample.xy) * lightSize;
                maxSIter--;
            }
            resultColor = float4(shadow * ndl.xxx / 1.0f, 1);
            PayLoad indirectPayLoad;
            Ray indirectRay;

            float3 direction = payLoad.normal;

            indirectRay.origin = float4(hitPoint.xyz + payLoad.normal * 0.01f, 0);
            indirectRay.direction = float4(normalize(payLoad.normal + randomPointOnUnitSphere(hitPoint.xy * float2(random_a, random_b))), 0);
            indirectRay.invdirection = float4(1.0f / indirectRay.direction.x, 1.0f / indirectRay.direction.y, 1.0f / indirectRay.direction.z, 1);
        
        // Indirect rays

            int maxAOIter = 1;
            float3 AO = 0;
            float intensity_falloff = 1.0f;
            while (maxAOIter > 0)
            {
                TraverseSceneBVH(indirectRay, indirectPayLoad, 1000);
                indirectRay.direction = float4(normalize(indirectPayLoad.normal + randomPointOnUnitSphere(indirectRay.direction.xy)), 0);
                indirectRay.invdirection = float4(1.0f / indirectRay.direction.x, 1.0f / indirectRay.direction.y, 1.0f / indirectRay.direction.z, 1);

                float3 indirectHitPoint = indirectRay.origin.xyz + indirectPayLoad.hitDist * indirectRay.direction.xyz;
                bool indirectShadow = CastShadow(indirectHitPoint + indirectPayLoad.normal * 0.001f, float4(lightSample, lightSize), shadowPayLoad, 1000);
                AO += (indirectPayLoad.color.rgb * intensity_falloff * !indirectShadow) / 3.14f;
                intensity_falloff *= 0.5;
                indirectRay.origin.xyz = float4(indirectHitPoint + payLoad.normal * 0.01f, 0);
                maxAOIter--;
            }
            resultColor += float4(AO, 0);
        
        //resultColor.rgb = payLoad.color.rgb;
/*
        int maxRIter = 1;
        float3 reflection = 0;
        direction = reflect(r.direction.xyz, payLoad.normal);
        indirectRay.direction = float4(normalize(direction + randomPointOnUnitSphere(indirectRay.direction.xy) * payLoad.color.a), 0);
        while (maxRIter > 0)
        {
            TraverseScene(indirectRay, indirectPayLoad);
            indirectRay.direction = float4(normalize(direction + randomPointOnUnitSphere(indirectRay.direction.xy) * payLoad.color.a ), 0);
            reflection += (indirectPayLoad.color.rgb /* !CastShadow(indirectRay.origin.xyz + indirectPayLoad.hitDist * indirectRay.direction.xyz + indirectPayLoad.normal * 0.001f, float4(lightSample, lightSize), shadowPayLoad)*//*) / 3.14f;*/
          /*  maxRIter--;
        }
        resultColor += float4(reflection / 1.0f, 0);*/
        

        }
    
    tResult[DTid.xy] = resultColor;
}