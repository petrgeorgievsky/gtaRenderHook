struct AABB
{
    float4 vMin;
    float4 vMax;
};

struct BLASLeaf
{
    AABB aabb;
    uint primitivesOffset;
    uint secondChildOffset;
    uint primitiveCount;
    uint axis;
};

struct TLASLeaf
{
    AABB aabb;

    uint secondChildOffset;
    uint3 lowLevel;

    uint blasBVHOffset;
    uint primitiveOffset;
    uint vertexOffset;
    uint axis;

    float4x4 world_transform;
};

struct Ray
{
    float4 origin;
    float4 direction;
    float4 invdirection;
};

struct Triangle
{
    uint2 ids;
};

struct Vertex
{
    float3 pos;
    uint color;
    float2 texcoords;
    float2 params;
};

inline void CalculateRayIntrinsics(inout Ray r)
{
    r.invdirection = float4(rcp(r.direction.xyz),0.0f);
}

inline uint4 UnpackTriangleIds(in Triangle packed_ids)
{
    return uint4( 
                    (packed_ids.ids.x & 0xFFFF),            // 1st id
                    ((packed_ids.ids.x >> 16) & 0xFFFF),    // 2nd id
                    (packed_ids.ids.y & 0xFFFF),            // 3rd id
                    ((packed_ids.ids.y >> 16) & 0xFFFF)     // mat id
                );
}