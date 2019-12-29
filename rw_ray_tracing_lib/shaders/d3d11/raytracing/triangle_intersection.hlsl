
bool RayTriangleIntersection(Ray r, Triangle packed_triangle, out PayLoad payLoad, uint vertex_offset)
{
    uint4 triangle_ids = UnpackTriangleIds(packed_triangle);
    float3 v0 = GET_TRIANGLE_POS(triangle_ids.x + vertex_offset).xyz;
    float3 v1 = GET_TRIANGLE_POS(triangle_ids.y + vertex_offset).xyz;
    float3 v2 = GET_TRIANGLE_POS(triangle_ids.z + vertex_offset).xyz;
    
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
        return false;

    float3 qvec = cross(tvec, e1);
    float v = dot(r.direction.xyz, qvec) * inv_det;
    if (v < 0 || u + v > 1)
        return false;
    
    payLoad.hitDist = dot(e2, qvec) * inv_det;
    payLoad.normal = normalize(cross(e1, e2));
    payLoad.hitPosition = r.origin + r.direction * payLoad.hitDist;
    
    INIT_PAYLOAD_HIT_TRIANGLE(payLoad, u, v, uint4( triangle_ids.xyz+ vertex_offset.xxx,triangle_ids.w ));
    if (payLoad.hitDist < 0)
        return false;
    return true;
}