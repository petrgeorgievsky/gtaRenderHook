
bool RayAABBIntersection(Ray r, AABB aabb, out PayLoad payLoad)
{
    float3 center = (aabb.vMin.xyz + aabb.vMax.xyz)*0.5f;
    payLoad.hitDist = distance(r.origin.xyz, center);
    payLoad.normal = float3(0,0,0);
    payLoad.hitPosition = r.origin + r.direction * payLoad.hitDist;
    INIT_PAYLOAD_AABB_HIT(payLoad);
    
    const float3 t0 = (aabb.vMin.xyz - r.origin.xyz) * r.invdirection.xyz;
    const float3 t1 = (aabb.vMax.xyz - r.origin.xyz) * r.invdirection.xyz;
 
    const float3 tmax = max(t0, t1);
    const float3 tmin = min(t0, t1);
 
    const float a1 = min(tmax.x, min(tmax.y, tmax.z));
    const float a0 = max(max(tmin.x, tmin.y), max(tmin.z, 0.0f));
 
    if (a1 < a0)
        return false;
    payLoad.hitDist = min(a0, a1);
    payLoad.hitPosition = r.origin + r.direction * payLoad.hitDist;
    return true;
}