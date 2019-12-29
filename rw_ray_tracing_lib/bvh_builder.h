#pragma once
#include <common_headers.h>
#include <unordered_map>
#include <vector>
namespace rh::rw::engine {
class RpGeometryInterface;
}
namespace rw_raytracing_lib {
class RayTracingTextureCache;
struct BBox
{
    DirectX::XMFLOAT4 min, max;
    BBox( const RwV3d &a, const RwV3d &b, const RwV3d &c );
    BBox();
    void Merge( const DirectX::XMVECTOR &vec );
    static BBox Merge( const BBox &a, const BBox &b );
    DirectX::XMVECTOR GetMinXm() const { return DirectX::XMLoadFloat4( &min ); }
    DirectX::XMVECTOR GetMaxXm() const { return DirectX::XMLoadFloat4( &max ); }
    DirectX::XMVECTOR GetCenter()
    {
        DirectX::XMVECTOR min_vec = DirectX::XMLoadFloat4( &min );
        DirectX::XMVECTOR max_vec = DirectX::XMLoadFloat4( &max );
        return DirectX::XMVectorScale( DirectX::XMVectorAdd( min_vec, max_vec ), 0.5f );
    }
    float SurfaceArea() const
    {
        DirectX::XMVECTOR min_vec = DirectX::XMLoadFloat4( &min );
        DirectX::XMVECTOR max_vec = DirectX::XMLoadFloat4( &max );
        DirectX::XMVECTOR d = DirectX::XMVectorSubtract( max_vec, min_vec );
        return 2.f
               * ( DirectX::XMVectorGetX( d ) * DirectX::XMVectorGetY( d )
                   + DirectX::XMVectorGetX( d ) * DirectX::XMVectorGetZ( d )
                   + DirectX::XMVectorGetY( d ) * DirectX::XMVectorGetZ( d ) );
    }
    BBox Transform( const DirectX::XMMATRIX &m ) const
    {
        BBox res;
        DirectX::XMVECTOR boxVertices[8];
        boxVertices[0] = {min.x, min.y, min.z, 1.0};
        boxVertices[1] = {min.x, min.y, max.z, 1.0};
        boxVertices[2] = {min.x, max.y, max.z, 1.0};
        boxVertices[3] = {min.x, max.y, min.z, 1.0};
        boxVertices[4] = {max.x, min.y, min.z, 1.0};
        boxVertices[6] = {max.x, min.y, max.z, 1.0};
        boxVertices[5] = {max.x, max.y, min.z, 1.0};
        boxVertices[7] = {max.x, max.y, max.z, 1.0};
        for ( auto i = 0; i < 8; i++ ) {
            DirectX::XMVECTOR v = DirectX::XMVector4Transform( boxVertices[i], m );
            DirectX::XMVECTOR r_min = DirectX::XMLoadFloat4( &res.min );
            DirectX::XMVECTOR r_max = DirectX::XMLoadFloat4( &res.max );
            DirectX::XMStoreFloat4( &res.min, DirectX::XMVectorMin( v, r_min ) );
            DirectX::XMStoreFloat4( &res.max, DirectX::XMVectorMax( v, r_max ) );
        }

        return res;
    }
};

struct BVHPrimitive
{
    BBox boundBox;
    DirectX::XMVECTOR centroid;
    uint32_t triangleId;
    uint32_t padd[3];
};

struct LinearBVHNodeTLAS
{
    BBox bounds;
    uint32_t secondChildOffset; // interior
    uint32_t lowLevel_a;        // 0 -> interior node
    uint32_t lowLevel_b;
    uint32_t materialOffset;

    uint32_t blasBVHOffset;
    uint32_t primitiveOffset;
    uint32_t vertexOffset;
    uint32_t axis;

    DirectX::XMFLOAT4X4 world_transform;
};

struct LinearBVHNode
{
    BBox bounds;
    uint32_t primitivesOffset;  // leaf
    uint32_t secondChildOffset; // interior

    uint32_t nPrimitives; // 0 -> interior node
    uint32_t axis;
};

struct BVHBuildNode
{
    BVHBuildNode() { children[0] = children[1] = nullptr; }
    ~BVHBuildNode() {}
    void InitLeaf( uint32_t first, uint32_t n, const BBox &b )
    {
        firstPrimOffset = first;
        nPrimitives = n;
        bounds = b;
    }
    void InitInterior( uint32_t axis, BVHBuildNode *c0, BVHBuildNode *c1 )
    {
        children[0] = c0;
        children[1] = c1;
        bounds = BBox::Merge( c0->bounds, c1->bounds );
        splitAxis = axis;
        nPrimitives = 0;
    }
    BBox bounds;
    BVHBuildNode *children[2];
    uint32_t splitAxis, firstPrimOffset, nPrimitives;
};

struct RTVertex
{
    RwV3d pos;
    uint32_t color;
    RwV4d texcoords;
};

struct MaterialInfo
{
    int32_t textureId;
    uint8_t transparency;
    uint8_t mipLevel;
    uint8_t xScale;
    uint8_t yScale;
};

struct BLAS_BVH
{
    std::vector<LinearBVHNode> nodes;
    std::vector<RpTriangle> ordered_triangles;
    std::vector<RTVertex> vertices;
    std::vector<MaterialInfo> materials;
};

struct blas_offsets
{
    uint32_t blasBVHOffset;
    uint32_t primitiveOffset;
    uint32_t vertexOffset;
    uint32_t materialOffset;
};

struct PackedBLAS_BVH
{
    BLAS_BVH blas;
    std::unordered_map<uint32_t, blas_offsets> blas_offsets_map;
};

struct BLAS_Instance
{
    DirectX::XMFLOAT4X4 world_transform;
    uint32_t blas_id;
    uint32_t padd[3] = {0, 0, 0};
};

struct TLAS_BVH
{
    std::vector<LinearBVHNodeTLAS> tlas;
};

class BVHBuilder
{
public:
    BVHBuilder();
    BLAS_BVH BuildBVH( rh::rw::engine::RpGeometryInterface *geometry,
                       RayTracingTextureCache *texture_cache = nullptr );

    void PackBLASBVH( PackedBLAS_BVH &result,
                      const std::vector<BLAS_BVH *> &new_blas,
                      uint32_t last_id_offset );
    TLAS_BVH BuildTLASBVH( PackedBLAS_BVH &packed_blas,
                           const std::vector<BLAS_BVH> &blas,
                           const std::vector<BLAS_Instance> &instances );
    void RecursiveBuildAAC( std::vector<BVHPrimitive> &buildData,
                            std::vector<std::pair<uint32_t, uint32_t>> mortonCodes,
                            uint32_t start,
                            uint32_t end,
                            uint32_t &totalNodes,
                            uint32_t partitionBit,
                            std::vector<BVHBuildNode *> *clusterData );

    void DrawDebug();

private:
    std::vector<BVHPrimitive> GenerateBVHPrimitiveList( rh::rw::engine::RpGeometryInterface *geometry );
};

} // namespace rw_raytracing_lib
