#include "bvh_builder.h"
#include <DebugUtils/DebugLogger.h>
#include <Engine/Common/IRenderingContext.h>
#include <Engine/IRenderer.h>
#include <algorithm>
#include <chrono>
#include <limits>
#include <sstream>
#include <ray_tracing_texture_cache.h>
#include <rw_engine/global_definitions.h>
#include <rw_engine/rw_rh_pipeline.h>
using namespace rw_raytracing_lib;

// Expands a 10-bit integer into 30 bits
// by inserting 2 zeros after each bit.
uint32_t expandBits( uint32_t v )
{
    v = ( v * 0x00010001u ) & 0xFF0000FFu;
    v = ( v * 0x00000101u ) & 0x0F00F00Fu;
    v = ( v * 0x00000011u ) & 0xC30C30C3u;
    v = ( v * 0x00000005u ) & 0x49249249u;
    return v;
}
// Generates
uint32_t morton3D( float x, float y, float z )
{
    x = min( max( x * 1024.0f, 0.0f ), 1023.0f );
    y = min( max( y * 1024.0f, 0.0f ), 1023.0f );
    z = min( max( z * 1024.0f, 0.0f ), 1023.0f );

    uint32_t xx = expandBits( static_cast<uint32_t>( x ) );
    uint32_t yy = expandBits( static_cast<uint32_t>( y ) );
    uint32_t zz = expandBits( static_cast<uint32_t>( z ) );
    return ( ( zz << 2 ) | ( yy << 1 ) | xx );
}

constexpr auto MORTON_CODE_START = 29;

BVHBuilder::BVHBuilder() {}

static inline uint32_t getDimForMorton3D( uint32_t idx )
{
    return idx % 3;
}

constexpr auto AAC_DELTA = 20;
constexpr auto AAC_EPSILON = 0.1f; // 0.1f for HQ, 0.2 for fast
constexpr auto AAC_ALPHA = ( 0.5f - AAC_EPSILON );

static inline float AAC_C()
{
    return ( 0.5f * powf( AAC_DELTA, 0.5f + AAC_EPSILON ) );
}

static inline uint32_t AAC_F( uint32_t x )
{
    return static_cast<uint32_t>( ceil( AAC_C() * powf( x, AAC_ALPHA ) ) );
}

// Searches for the best candidate to merge i-th cluster with,
// using surface area heuristic
// O(n)
uint32_t GetClosestCluster( std::vector<BVHBuildNode *> &clusters, uint32_t i )
{
    float closestDist = INFINITY;
    uint32_t idx = i;

    for ( uint32_t j = 0; j < clusters.size(); ++j ) {
        if ( i == j )
            continue;

        BBox combined = BBox::Merge( clusters[i]->bounds, clusters[j]->bounds );
        float d = combined.SurfaceArea();
        // search for minimal surface area of combined bboxes
        if ( d < closestDist ) {
            closestDist = d;
            idx = j;
        }
    }

    return idx;
}

std::vector<BVHBuildNode *> combineCluster( std::vector<BVHBuildNode *> &clusters,
                                            uint32_t n,
                                            uint32_t &totalNodes,
                                            uint32_t dim )
{
    std::vector<uint32_t> closest( clusters.size(), 0 );

    // O(n^2)
    // Build closest clusters lookup table
    for ( uint32_t i = 0; i < clusters.size(); ++i ) {
        closest[i] = GetClosestCluster( clusters, i );
    }

    while ( clusters.size() > n ) {
        float bestDist = INFINITY;
        uint32_t leftIdx = 0;
        uint32_t rightIdx = 0;

        // Search for 2 closest clusters to merge
        for ( uint32_t i = 0; i < clusters.size(); ++i ) {
            BBox combined = BBox::Merge( clusters[i]->bounds, clusters[closest[i]]->bounds );
            float d = combined.SurfaceArea();
            if ( d < bestDist ) {
                bestDist = d;
                leftIdx = i;
                rightIdx = closest[i];
            }
        }

        totalNodes++;
        // Create new cluster node
        BVHBuildNode *node = new BVHBuildNode();

        node->InitInterior( dim, clusters[leftIdx], clusters[rightIdx] );

        clusters[leftIdx] = node;

        clusters[rightIdx] = clusters.back();
        closest[rightIdx] = closest.back();

        clusters.pop_back();
        closest.pop_back();

        closest[leftIdx] = GetClosestCluster( clusters, leftIdx );

        for ( uint32_t i = 0; i < clusters.size(); ++i ) {
            if ( closest[i] == leftIdx || closest[i] == rightIdx )
                closest[i] = GetClosestCluster( clusters, i );
            else if ( closest[i] == closest.size() ) {
                closest[i] = rightIdx;
            }
        }
    }

    return clusters;
}

uint32_t makePartition( std::vector<std::pair<uint32_t, uint32_t>> sortedMC,
                        uint32_t start,
                        uint32_t end,
                        uint32_t partitionbit )
{
    uint32_t curFind = ( 1 << partitionbit );

    if ( ( sortedMC[start].first & curFind ) == ( sortedMC[end - 1].first & curFind ) ) {
        return start + ( end - start ) / 2;
    }

    uint32_t lower = start;
    uint32_t upper = end;

    while ( lower < upper ) {
        uint32_t mid = lower + ( upper - lower ) / 2;
        if ( ( sortedMC[mid].first & curFind ) == 0 ) {
            lower = mid + 1;
        } else {
            upper = mid;
        }
    }

    return lower;
}

void BVHBuilder::RecursiveBuildAAC( std::vector<BVHPrimitive> &buildData,
                                    std::vector<std::pair<uint32_t, uint32_t>> mortonCodes,
                                    uint32_t start,
                                    uint32_t end,
                                    uint32_t &totalNodes,
                                    uint32_t partitionBit,
                                    std::vector<BVHBuildNode *> *clusterData )
{
    if ( end - start == 0 ) {
        return;
    }
    uint32_t dim = getDimForMorton3D( partitionBit );
    if ( end - start < AAC_DELTA ) {
        std::vector<BVHBuildNode *> clusters;
        totalNodes += ( end - start );
        for ( uint32_t i = start; i < end; ++i ) {
            // Create leaf _BVHBuildNode_
            BVHBuildNode *node = new BVHBuildNode();
            uint32_t primIdx = mortonCodes[i].second;
            node->InitLeaf( primIdx, 1, buildData[primIdx].boundBox );
            // deal with firstPrimOffset later with DFS
            clusters.push_back( node );
        }

        *clusterData = combineCluster( clusters, AAC_F( AAC_DELTA ), totalNodes, dim );
        return;
    }

    uint32_t splitIdx = makePartition( mortonCodes, start, end, partitionBit );

    uint32_t newPartionBit = partitionBit - 1;
    std::vector<BVHBuildNode *> leftC;
    std::vector<BVHBuildNode *> rightC;
    uint32_t rightTotalnodes = 0;

    RecursiveBuildAAC( buildData, mortonCodes, start, splitIdx, totalNodes, newPartionBit, &leftC );
    RecursiveBuildAAC( buildData,
                       mortonCodes,
                       splitIdx,
                       end,
                       rightTotalnodes,
                       newPartionBit,
                       &rightC );

    totalNodes += rightTotalnodes;

    leftC.insert( leftC.end(), rightC.begin(), rightC.end() );
    *clusterData = combineCluster( leftC, AAC_F( end - start ), totalNodes, dim );
}

std::vector<BVHPrimitive> BVHBuilder::GenerateBVHPrimitiveList(
    rh::rw::engine::RpGeometryInterface *geometry )
{
    std::vector<BVHPrimitive> result;
    result.reserve( static_cast<size_t>( geometry->GetTriangleCount() ) );
    for ( size_t i = 0; i < static_cast<size_t>( geometry->GetTriangleCount() ); i++ ) {
        auto triangle_ids = geometry->GetTrianglePtr()[i].vertIndex;

        RwV3d v1 = geometry->GetMorphTarget( 0 )->verts[triangle_ids[0]];
        RwV3d v2 = geometry->GetMorphTarget( 0 )->verts[triangle_ids[1]];
        RwV3d v3 = geometry->GetMorphTarget( 0 )->verts[triangle_ids[2]];
        BBox bbox( v1, v2, v3 );

        DirectX::XMVECTOR centroid = bbox.GetCenter();
        result.push_back( {bbox, centroid, static_cast<uint32_t>( i ), {0, 0, 0}} );
    }
    return result;
}

BBox::BBox( const RwV3d &a, const RwV3d &b, const RwV3d &c )
{
    const float min_f = -( std::numeric_limits<float>::max )();
    const float max_f = ( std::numeric_limits<float>::max )();
    DirectX::XMVECTOR max_xm = {min_f, min_f, min_f, 1.0f};
    DirectX::XMVECTOR min_xm = {max_f, max_f, max_f, 1.0f};

    min_xm = DirectX::XMVectorMin( min_xm, {a.x, a.y, a.z, 1.0f} );
    min_xm = DirectX::XMVectorMin( min_xm, {b.x, b.y, b.z, 1.0f} );
    min_xm = DirectX::XMVectorMin( min_xm, {c.x, c.y, c.z, 1.0f} );

    max_xm = DirectX::XMVectorMax( max_xm, {a.x, a.y, a.z, 1.0f} );
    max_xm = DirectX::XMVectorMax( max_xm, {b.x, b.y, b.z, 1.0f} );
    max_xm = DirectX::XMVectorMax( max_xm, {c.x, c.y, c.z, 1.0f} );

    DirectX::XMStoreFloat4( &max, max_xm );
    DirectX::XMStoreFloat4( &min, min_xm );
}

BBox::BBox()
{
    const float min_f = -( std::numeric_limits<float>::max )();
    const float max_f = ( std::numeric_limits<float>::max )();
    max = {min_f, min_f, min_f, 0};
    min = {max_f, max_f, max_f, 0};
}

void BBox::Merge( const DirectX::XMVECTOR &vec )
{
    DirectX::XMVECTOR max_xm = DirectX::XMLoadFloat4( &max );
    DirectX::XMVECTOR min_xm = DirectX::XMLoadFloat4( &min );
    min_xm = DirectX::XMVectorMin( min_xm, vec );
    max_xm = DirectX::XMVectorMax( max_xm, vec );
    DirectX::XMStoreFloat4( &max, max_xm );
    DirectX::XMStoreFloat4( &min, min_xm );
}

BBox BBox::Merge( const BBox &a, const BBox &b )
{
    BBox c;
    DirectX::XMStoreFloat4( &c.min,
                            DirectX::XMVectorMin( DirectX::XMVectorMin( a.GetMinXm(), b.GetMinXm() ),
                                                  c.GetMinXm() ) );
    DirectX::XMStoreFloat4( &c.max,
                            DirectX::XMVectorMax( DirectX::XMVectorMax( a.GetMaxXm(), b.GetMaxXm() ),
                                                  c.GetMaxXm() ) );
    return c;
}

uint32_t bvhDfs( std::vector<LinearBVHNode> &flat_tree,
                 RpTriangle *&primitives,
                 std::vector<RpTriangle> &orderedPrims,
                 BVHBuildNode *node,
                 std::vector<BVHPrimitive> &buildData,
                 uint32_t &offset )
{
    LinearBVHNode &linearNode = flat_tree[offset];
    linearNode.bounds = node->bounds;
    uint32_t myOffset = offset++;

    if ( node->nPrimitives > 0 ) {
        assert( !node->children[0] && !node->children[1] );
        uint32_t firstPrimOffset = orderedPrims.size();
        uint32_t primNum = buildData[node->firstPrimOffset].triangleId;
        orderedPrims.push_back( primitives[primNum] );
        node->firstPrimOffset = firstPrimOffset;
        linearNode.primitivesOffset = node->firstPrimOffset;
        linearNode.nPrimitives = node->nPrimitives;
    } else {
        linearNode.nPrimitives = 0;
        linearNode.axis = node->splitAxis;
        bvhDfs( flat_tree, primitives, orderedPrims, node->children[0], buildData, offset );
        linearNode.secondChildOffset
            = bvhDfs( flat_tree, primitives, orderedPrims, node->children[1], buildData, offset );
    }

    delete node;
    return myOffset;
}

uint32_t bvhDfsTlas( std::vector<LinearBVHNodeTLAS> &flat_tree,
                     const std::vector<BLAS_Instance> &instances,
                     uint32_t &inst_offset,
                     BVHBuildNode *node,
                     std::vector<BVHPrimitive> &buildData,
                     uint32_t &offset )
{
    LinearBVHNodeTLAS &linearNode = flat_tree[offset];
    linearNode.bounds = node->bounds;
    uint32_t myOffset = offset++;

    if ( node->nPrimitives > 0 ) {
        assert( !node->children[0] && !node->children[1] );
        uint32_t firstPrimOffset = inst_offset;
        uint32_t primNum = buildData[node->firstPrimOffset].triangleId;
        inst_offset++;
        node->firstPrimOffset = firstPrimOffset;
        linearNode.blasBVHOffset = primNum;
        linearNode.lowLevel_a = 1;
    } else {
        linearNode.lowLevel_a = 0;
        linearNode.axis = node->splitAxis;
        bvhDfsTlas( flat_tree, instances, inst_offset, node->children[0], buildData, offset );
        linearNode.secondChildOffset
            = bvhDfsTlas( flat_tree, instances, inst_offset, node->children[1], buildData, offset );
    }

    delete node;
    return myOffset;
}

BLAS_BVH BVHBuilder::BuildBVH( rh::rw::engine::RpGeometryInterface *geometry,
                               RayTracingTextureCache *texture_cache )
{
    RpTriangle *tri_list = geometry->GetTrianglePtr();
    auto s = std::chrono::high_resolution_clock::now();
    auto bvh_prim_list = GenerateBVHPrimitiveList( geometry );

    BBox centerBBox;
    for ( uint32_t i = 0; i < bvh_prim_list.size(); ++i )
        centerBBox.Merge( bvh_prim_list[i].centroid );

    std::vector<std::pair<uint32_t, uint32_t>> mortonCodes;
    mortonCodes.reserve( bvh_prim_list.size() );

    for ( uint32_t i = 0; i < bvh_prim_list.size(); ++i ) {
        auto c = bvh_prim_list[i].centroid;
        float newX = ( DirectX::XMVectorGetX( c ) - centerBBox.min.x )
                     / ( centerBBox.max.x - centerBBox.min.x );
        float newY = ( DirectX::XMVectorGetY( c ) - centerBBox.min.y )
                     / ( centerBBox.max.y - centerBBox.min.y );
        float newZ = ( DirectX::XMVectorGetZ( c ) - centerBBox.min.z )
                     / ( centerBBox.max.z - centerBBox.min.z );
        uint32_t mc = morton3D( newX, newY, newZ );

        mortonCodes.push_back( std::make_pair( mc, i ) );
    }
    std::sort( mortonCodes.begin(), mortonCodes.end() );

    auto bv_bs = std::chrono::high_resolution_clock::now();

    std::vector<BVHBuildNode *> clusters;
    uint32_t totalNodes = 0;
    RecursiveBuildAAC( bvh_prim_list,
                       mortonCodes,
                       0,
                       bvh_prim_list.size(),
                       totalNodes,
                       MORTON_CODE_START,
                       &clusters );

    BVHBuildNode *root = combineCluster( clusters, 1, totalNodes, 2 )[0];
    auto bv_be = std::chrono::high_resolution_clock::now();

    std::vector<LinearBVHNode> bvh_tree_flat;
    bvh_tree_flat.reserve( totalNodes );
    for ( uint32_t i = 0; i < totalNodes; ++i )
        bvh_tree_flat.push_back( {} );
    std::vector<RpTriangle> ordered_tri_list;
    uint32_t offset = 0;
    bvhDfs( bvh_tree_flat, tri_list, ordered_tri_list, root, bvh_prim_list, offset );

    auto e = std::chrono::high_resolution_clock::now();
    std::stringstream ss;
    ss << "bvh build time without morton codes: "
       << std::chrono::duration_cast<std::chrono::milliseconds>( e - bv_bs ).count() << " ms.\n"
       << "bvh build time: "
       << std::chrono::duration_cast<std::chrono::milliseconds>( e - s ).count() << " ms.\n"
       << "bvh recursive aac build time: "
       << std::chrono::duration_cast<std::chrono::milliseconds>( bv_be - bv_bs ).count()
       << " ms.\n";
    rh::debug::DebugLogger::Log( ss.str() );

    std::vector<RTVertex> verts;
    verts.reserve( static_cast<uint32_t>( geometry->GetVertexCount() ) );
    for ( auto i = 0; i < geometry->GetVertexCount(); i++ ) {
        uint32_t color = geometry->GetVertexColorPtr() != nullptr
                             ? *reinterpret_cast<uint32_t *>( &geometry->GetVertexColorPtr()[i] )
                             : 0xFF000000;
        RwTexCoords texcoords = geometry->GetTexCoordSetPtr( 0 ) != nullptr
                                    ? geometry->GetTexCoordSetPtr( 0 )[i]
                                    : RwTexCoords{0, 0};
        verts.push_back( {{geometry->GetMorphTarget( 0 )->verts[i].x,
                           geometry->GetMorphTarget( 0 )->verts[i].y,
                           geometry->GetMorphTarget( 0 )->verts[i].z},
                          color,
                          {texcoords.u, texcoords.v, 0, 0}} );
    }
    std::vector<MaterialInfo> materials;
    RpMeshHeader *header = geometry->GetMeshHeader();
    RpMesh *mesh_array = reinterpret_cast<RpMesh *>( reinterpret_cast<uint8_t *>( header )
                                                     + sizeof( RpMeshHeader ) );
    materials.reserve( header->numMeshes );
    for ( auto i = 0; i < header->numMeshes; i++ ) {
        const RpMesh &mesh = mesh_array[i];
        MaterialInfo info{};
        info.textureId = -1;
        if ( mesh.material && mesh.material->texture && mesh.material->texture->raster ) {
            info.transparency = mesh.material->color.alpha;
            auto address = texture_cache->GetTextureAddress(
                rh::rw::engine::GetInternalRaster( mesh.material->texture->raster ) );
            if ( address != nullptr ) {
                info.textureId = address->id;
                info.mipLevel = address->mip_level;
                info.xScale = address->scaleX;
                info.yScale = address->scaleY;
            }
        }
        materials.push_back( info );
    }
    return {bvh_tree_flat, ordered_tri_list, verts, materials};
}

void BVHBuilder::PackBLASBVH( PackedBLAS_BVH &result,
                              const std::vector<BLAS_BVH *> &new_blas,
                              uint32_t last_id_offset )
{
    auto s = std::chrono::high_resolution_clock::now();
    uint32_t blas_bvh_nodes_size = result.blas.nodes.size();
    uint32_t blas_triangle_lists_size = result.blas.ordered_triangles.size();
    uint32_t blas_vertex_lists_size = result.blas.vertices.size();
    uint32_t blas_material_lists_size = result.blas.materials.size();
    for ( size_t i = 0; i < new_blas.size(); i++ ) {
        result.blas_offsets_map[last_id_offset + i].blasBVHOffset = blas_bvh_nodes_size;
        result.blas_offsets_map[last_id_offset + i].primitiveOffset = blas_triangle_lists_size;
        result.blas_offsets_map[last_id_offset + i].vertexOffset = blas_vertex_lists_size;
        result.blas_offsets_map[last_id_offset + i].materialOffset = blas_material_lists_size;

        blas_bvh_nodes_size += new_blas[i]->nodes.size();
        blas_triangle_lists_size += new_blas[i]->ordered_triangles.size();
        blas_vertex_lists_size += new_blas[i]->vertices.size();
        blas_material_lists_size += new_blas[i]->materials.size();
    }

    // res.blas.ordered_triangles.reserve( blas_triangle_lists_size );
    // res.blas.nodes.reserve( blas_bvh_nodes_size );
    // res.blas.vertices.reserve( blas_vertex_lists_size );
    for ( size_t i = 0; i < new_blas.size(); i++ ) {
        result.blas.vertices.insert( result.blas.vertices.end(),
                                     new_blas[i]->vertices.begin(),
                                     new_blas[i]->vertices.end() );
        result.blas.ordered_triangles.insert( result.blas.ordered_triangles.end(),
                                              new_blas[i]->ordered_triangles.begin(),
                                              new_blas[i]->ordered_triangles.end() );
        result.blas.nodes.insert( result.blas.nodes.end(),
                                  new_blas[i]->nodes.begin(),
                                  new_blas[i]->nodes.end() );
        result.blas.materials.insert( result.blas.materials.end(),
                                      new_blas[i]->materials.begin(),
                                      new_blas[i]->materials.end() );
    }
    auto e = std::chrono::high_resolution_clock::now();
    std::stringstream ss;
    ss << "bvh pack time : "
       << std::chrono::duration_cast<std::chrono::milliseconds>( e - s ).count() << " ms.\n"
       << "bvh new node count : " << new_blas.size() << "\nbvh packed blas size: "
       << ( result.blas.materials.size() * sizeof( MaterialInfo )
            + result.blas.ordered_triangles.size() * sizeof( RpTriangle )
            + result.blas.vertices.size() * sizeof( RTVertex )
            + result.blas.nodes.size() * sizeof( LinearBVHNode ) )
       << " bytes.\n";
    rh::debug::DebugLogger::Log( ss.str() );
}

TLAS_BVH BVHBuilder::BuildTLASBVH( PackedBLAS_BVH &packed_blas,
                                   const std::vector<BLAS_BVH> &blas,
                                   const std::vector<BLAS_Instance> &instances )
{
    auto s = std::chrono::high_resolution_clock::now();
    std::vector<BVHPrimitive> bvh_prim_list;
    bvh_prim_list.reserve( instances.size() );
    for ( size_t i = 0; i < instances.size(); i++ ) {
        DirectX::XMMATRIX ws = DirectX::XMLoadFloat4x4( &instances[i].world_transform );
        BBox ws_bbox = blas[instances[i].blas_id].nodes[0].bounds.Transform( ws );
        DirectX::XMVECTOR centroid = ws_bbox.GetCenter();
        bvh_prim_list.push_back( {ws_bbox, centroid, static_cast<uint32_t>( i ), {0, 0, 0}} );
    }

    BBox centerBBox;
    for ( uint32_t i = 0; i < bvh_prim_list.size(); ++i )
        centerBBox = BBox::Merge( bvh_prim_list[i].boundBox, centerBBox );

    std::vector<std::pair<uint32_t, uint32_t>> mortonCodes;
    mortonCodes.reserve( bvh_prim_list.size() );

    for ( uint32_t i = 0; i < bvh_prim_list.size(); ++i ) {
        auto c = bvh_prim_list[i].centroid;
        float newX = ( DirectX::XMVectorGetX( c ) - centerBBox.min.x )
                     / ( centerBBox.max.x - centerBBox.min.x );
        float newY = ( DirectX::XMVectorGetY( c ) - centerBBox.min.y )
                     / ( centerBBox.max.y - centerBBox.min.y );
        float newZ = ( DirectX::XMVectorGetZ( c ) - centerBBox.min.z )
                     / ( centerBBox.max.z - centerBBox.min.z );
        uint32_t mc = morton3D( newX, newY, newZ );

        mortonCodes.push_back( std::make_pair( mc, i ) );
    }
    std::sort( mortonCodes.begin(), mortonCodes.end() );

    std::vector<BVHBuildNode *> clusters;
    uint32_t totalNodes = 0;
    RecursiveBuildAAC( bvh_prim_list,
                       mortonCodes,
                       0,
                       bvh_prim_list.size(),
                       totalNodes,
                       MORTON_CODE_START,
                       &clusters );

    BVHBuildNode *root = combineCluster( clusters, 1, totalNodes, 2 )[0];

    TLAS_BVH tlas;

    tlas.tlas.reserve( totalNodes );
    for ( uint32_t i = 0; i < totalNodes; ++i )
        tlas.tlas.push_back( {} );
    std::vector<BLAS_Instance> ordered_blas_list;
    uint32_t offset = 0;
    uint32_t inst_offset = 0;
    bvhDfsTlas( tlas.tlas, instances, inst_offset, root, bvh_prim_list, offset );

    for ( size_t i = 0; i < tlas.tlas.size(); i++ ) {
        if ( tlas.tlas[i].lowLevel_a > 0 ) {
            uint32_t blas_id = instances[tlas.tlas[i].blasBVHOffset].blas_id;
            DirectX::XMMATRIX ws = DirectX::XMLoadFloat4x4(
                &instances[tlas.tlas[i].blasBVHOffset].world_transform );
            DirectX::XMStoreFloat4x4( &tlas.tlas[i].world_transform,
                                      DirectX::XMMatrixInverse( nullptr, ws ) );
            tlas.tlas[i].blasBVHOffset = packed_blas.blas_offsets_map[blas_id].blasBVHOffset;
            tlas.tlas[i].primitiveOffset = packed_blas.blas_offsets_map[blas_id].primitiveOffset;
            tlas.tlas[i].vertexOffset = packed_blas.blas_offsets_map[blas_id].vertexOffset;
            tlas.tlas[i].materialOffset = packed_blas.blas_offsets_map[blas_id].materialOffset;
        }
    }

    auto e = std::chrono::high_resolution_clock::now();
    std::stringstream ss;
    ss << "tlas build time : "
       << std::chrono::duration_cast<std::chrono::milliseconds>( e - s ).count() << " ms.\n"
       << "tlas node count : " << tlas.tlas.size() << ".\n";
    rh::debug::DebugLogger::Log( ss.str() );
    return tlas;
}
