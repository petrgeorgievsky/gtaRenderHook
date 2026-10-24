// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "GrassRendererRH.h"
#include "CDebug.h"
#include "D3D1XShader.h"
#include "D3D1XStateManager.h"
#include "D3D1XRenderBuffersManager.h"
#include "RwD3D1XEngine.h"
#include "D3DRenderer.h"

// Game data used by CGrassRenderer::DrawTriPlants (0x5DAD00)
#define CPlantMgr__Render() ( (void( __cdecl * )())0x5DBAE0 )()
#define CGrassRenderer__m_closeDist ( *(float *)0xC02DBC )
#define CGrassRenderer__m_farDist ( *(float *)0x8D132C )
#define CGrassRenderer__m_windBending ( *(float *)0xC02DB8 )
#define CGrassRenderer__m_vecCameraPos ( (float *)0xC02DDC )
#define g_fx__m_nFxQuality ( *(int *)( 0xA9AE00 + 0x54 ) )
#define game_rand() ( (int( __cdecl * )())0x821B1E )()
#define game_srand( seed ) ( (void( __cdecl * )( unsigned int ))0x821B11 )( seed )

static const float kRandScale = 1.0f / 32767.0f;

GrassSettingsBlock gGrassSettings;

// CPlantMgr pools are free lists, only the list init (0x5DD780) knows the static arrays
#define CPlantMgr__InitLists() ( (bool( __cdecl * )())0x5DD780 )()
#define CPlantMgr__m_UnusedLocTriListHead ( *(uint8_t **)0xC03984 )
#define CPlantMgr__m_UnusedColEntListHead ( *(uint8_t **)0xC03998 )
static const UINT kLocTriSize = 0x54, kLocTriNext = 0x4C, kLocTriPrev = 0x50, kGameLocTris = 256;
static const UINT kColEntSize = 0x14, kColEntNext = 0x0C, kColEntPrev = 0x10, kGameColEnts = 40;
static std::vector<uint8_t> s_aExtraLocTris, s_aExtraColEnts;

// Distances read by the patched CPlantMgr code
static float s_fGenerationRadiusSq = 100.0f * 100.0f;
static float s_fCacheRadius        = 340.0f;
static float s_fCacheRadiusSq      = 340.0f * 340.0f;
static float s_fSectionRadius      = 100.0f;
// fcomp dword ptr [0x859AA4] (10000 = 100 m squared): 7 in tri removal, 7 in tri creation
static const INT_PTR kGenerationRadiusSqOps[] = { 0x5DC0D0, 0x5DC103, 0x5DC130, 0x5DC15D, 0x5DC18A, 0x5DC1B3, 0x5DC1DC,
                                                  0x5DCB88, 0x5DCBBD, 0x5DCBEA, 0x5DCC17, 0x5DCC40, 0x5DCC69, 0x5DCC92 };
// fsub/fadd dword ptr [0x85F074] (340), fcomp dword ptr [0x86BF20] (340 squared): collision entity cache
static const INT_PTR kCacheRadiusOps[]   = { 0x5DC5B3, 0x5DC5C4, 0x5DC5D9, 0x5DC5E7 };
static const INT_PTR kCacheRadiusSqOps[] = { 0x5DC569, 0x5DC82C };
// push 100.0: sphere tested against collision sections before their triangles are scanned
static const INT_PTR kSectionRadiusPush = 0x5DCE78;

static void PatchOperand( INT_PTR instruction, UINT32 expected, const void *value )
{
    if ( *(UINT32 *)( instruction + 2 ) != expected )
    {
        g_pDebug->printMsg( "Grass: unexpected code at " + std::to_string( instruction ) + ", not patched", 0 );
        return;
    }
    SetPointer( instruction + 2, const_cast<void *>( value ) );
}

static void LinkNodes( std::vector<uint8_t> &storage, UINT count, UINT size, UINT next, UINT prev, uint8_t *&head )
{
    if ( storage.size() != count * size )
        storage.assign( count * size, 0 );
    else
        std::fill( storage.begin(), storage.end(), 0 );
    for ( UINT i = 0; i < count; i++ )
    {
        uint8_t *node                       = storage.data() + i * size;
        *reinterpret_cast<uint8_t **>( node + next ) = head;
        if ( head )
            *reinterpret_cast<uint8_t **>( head + prev ) = node;
        head = node;
    }
}
static const UINT kMinInstanceCapacity = 16384;

struct CGrassRendererRH::PPTriPlant
{
    RwV3d V1, V2, V3;
    RwV3d center;
    RwUInt16 model_id;
    RwUInt16 num_plants;
    float scale_xy;
    float scale_z;
    RwTexture *texture;
    RwRGBA color;
    RwUInt8 intensity;
    RwUInt8 intensity_var;
    RwUInt16 pad;
    float seed;
    float scale_var_xy;
    float scale_var_z;
    float wind_bend_scale;
    float wind_bend_var;
};
static_assert( sizeof( RwV3d ) == 12, "RwV3d" );

struct GrassVertex
{
    RwV3d pos;
    RwTexCoords uv;
    RwV3d normal;
};

bool CGrassRendererRH::m_bInitialized                = false;
bool CGrassRendererRH::m_bCollecting                 = false;
CD3D1XVertexShader *CGrassRendererRH::m_pVS          = nullptr;
CD3D1XPixelShader *CGrassRendererRH::m_pPS           = nullptr;
ID3D11InputLayout *CGrassRendererRH::m_pInputLayout  = nullptr;
ID3D11Buffer *CGrassRendererRH::m_pInstanceBuffer    = nullptr;
UINT CGrassRendererRH::m_nInstanceCapacity           = 0;
std::vector<CGrassRendererRH::Instance> CGrassRendererRH::m_aInstances;
std::vector<std::pair<RpGeometry *, RwTexture *>> CGrassRendererRH::m_aInstanceKeys;
std::vector<std::pair<RpGeometry *, CGrassRendererRH::Mesh>> CGrassRendererRH::m_aMeshes;

void CGrassRendererRH::Patch()
{
    RedirectJump( 0x5DAD00, DrawTriPlants );
    RedirectCall( 0x5DD913, InitPlantLists );
    for ( INT_PTR op : kGenerationRadiusSqOps )
        PatchOperand( op, 0x859AA4, &s_fGenerationRadiusSq );
    for ( INT_PTR op : kCacheRadiusOps )
        PatchOperand( op, 0x85F074, &s_fCacheRadius );
    for ( INT_PTR op : kCacheRadiusSqOps )
        PatchOperand( op, 0x86BF20, &s_fCacheRadiusSq );

    // CPlantMgr::Render darkens each plant triangle's colour by the collision triangle's baked day/night lighting,
    // the only lighting unlit grass had. Grass is lit by the deferred pass now, so the factor is kept at 255:
    // call _ftol (0x5DBD27) -> fstp st(0); mov al, 0FFh; nop
    if ( *(BYTE *)0x5DBD27 == 0xE8 && 0x5DBD2C + *(INT32 *)0x5DBD28 == 0x821B40 )
    {
        const BYTE fullLight[] = { 0xDD, 0xD8, 0xB0, 0xFF, 0x90 };
        ::Patch( (void *)0x5DBD27, (void *)fullLight, sizeof( fullLight ) );
    }
}

bool __cdecl CGrassRendererRH::InitPlantLists()
{
    if ( !CPlantMgr__InitLists() )
        return false;
    UINT tris    = gGrassSettings.GetUInt( "TrianglePool" );
    UINT entries = gGrassSettings.GetUInt( "EntityCache" );
    if ( tris > kGameLocTris )
        LinkNodes( s_aExtraLocTris, tris - kGameLocTris, kLocTriSize, kLocTriNext, kLocTriPrev, CPlantMgr__m_UnusedLocTriListHead );
    if ( entries > kGameColEnts )
        LinkNodes( s_aExtraColEnts, entries - kGameColEnts, kColEntSize, kColEntNext, kColEntPrev, CPlantMgr__m_UnusedColEntListHead );
    UpdatePlantLimits();
    return true;
}

void CGrassRendererRH::UpdatePlantLimits()
{
    float drawDistance = gGrassSettings.GetFloat( "DrawDistance" );
    // triangles must exist a bit past where their grass fades out
    float radius = max( gGrassSettings.GetFloat( "GenerationRadius" ), drawDistance + 30.0f );
    s_fGenerationRadiusSq = radius * radius;
    // the collision entity cache has to reach past the generation radius
    s_fCacheRadius   = max( 340.0f, radius + 60.0f );
    s_fCacheRadiusSq = s_fCacheRadius * s_fCacheRadius;
    if ( s_fSectionRadius != radius )
    {
        s_fSectionRadius = radius;
        if ( *(BYTE *)kSectionRadiusPush == 0x68 )
            ::Patch( (void *)( kSectionRadiusPush + 1 ), &s_fSectionRadius, sizeof( float ) );
    }
    CGrassRenderer__m_farDist = drawDistance;
}

/*!
    Same placement as CGrassRenderer::DrawTriPlants: per triangle srand(seed), per tuft two randoms for the position
    in the triangle and three for scale and wind bend, tufts closer than m_closeDist - 2 skipped, the triangle faded
    out over 20 m past the far distance.
*/
int __cdecl CGrassRendererRH::DrawTriPlants( PPTriPlant *triPlants, int numTriPlants, RpAtomic **plantModels )
{
    if ( !m_bCollecting )
        return 1;
    const float *cam   = CGrassRenderer__m_vecCameraPos;
    const float farDist = g_fx__m_nFxQuality >= 2 ? CGrassRenderer__m_farDist : CGrassRenderer__m_farDist * 0.5f;
    const float fadeEnd = farDist + 20.0f;
    const float closeDist = CGrassRenderer__m_closeDist - 2.0f;

    for ( int t = 0; t < numTriPlants; t++ )
    {
        const PPTriPlant &plant = triPlants[t];
        float dx = plant.center.x - cam[0], dy = plant.center.y - cam[1], dz = plant.center.z - cam[2];
        float dist = sqrtf( dx * dx + dy * dy + dz * dz );
        if ( dist > fadeEnd )
            continue;
        RpAtomic *atomic = plantModels[plant.model_id];
        if ( atomic == nullptr || atomic->geometry == nullptr )
            continue;

        game_srand( (unsigned int)(int)plant.seed );

        Instance inst{};
        inst.fade  = min( ( fadeEnd - dist ) / 20.0f, 1.0f );
        int intensity = plant.intensity;
        inst.color = { (RwUInt8)( ( plant.color.red * intensity ) >> 8 ), (RwUInt8)( ( plant.color.green * intensity ) >> 8 ),
                       (RwUInt8)( ( plant.color.blue * intensity ) >> 8 ), plant.color.alpha };
        const std::pair<RpGeometry *, RwTexture *> key( atomic->geometry, plant.texture );

        for ( int p = 0; p < plant.num_plants; p++ )
        {
            float a = game_rand() * kRandScale;
            float b = game_rand() * kRandScale;
            if ( a + b > 1.0f )
            {
                a = 1.0f - a;
                b = 1.0f - b;
            }
            float w = 1.0f - a - b;
            RwV3d pos = { w * plant.V1.x + a * plant.V2.x + b * plant.V3.x,
                          w * plant.V1.y + a * plant.V2.y + b * plant.V3.y,
                          w * plant.V1.z + a * plant.V2.z + b * plant.V3.z };
            float px = pos.x - cam[0], py = pos.y - cam[1], pz = pos.z - cam[2];
            float plantDist = sqrtf( px * px + py * py + pz * pz );
            if ( closeDist > plantDist )
            {
                // keep the random sequence in step with the game
                game_rand();
                game_rand();
                game_rand();
                continue;
            }
            inst.pos[0]  = pos.x;
            inst.pos[1]  = pos.y;
            inst.pos[2]  = pos.z;
            inst.scaleXY = game_rand() * kRandScale * plant.scale_var_xy + plant.scale_xy;
            inst.scaleZ  = game_rand() * kRandScale * plant.scale_var_z + plant.scale_z;
            inst.bend    = ( game_rand() * kRandScale * plant.wind_bend_var + 1.0f ) *
                        ( CGrassRenderer__m_windBending * plant.wind_bend_scale );
            m_aInstances.push_back( inst );
            m_aInstanceKeys.push_back( key );
        }
    }
    return 1;
}

bool CGrassRendererRH::Init()
{
    if ( m_bInitialized )
        return m_pInputLayout != nullptr;
    m_bInitialized = true;
    m_pVS          = new CD3D1XVertexShader( "shaders/Grass.hlsl", "VS" );
    m_pPS          = new CD3D1XPixelShader( "shaders/Grass.hlsl", "PS" );
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 5, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "TEXCOORD", 6, DXGI_FORMAT_R32G32B32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 1, 28, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    };
    ID3DBlob *vsBlob = m_pVS->getBlob();
    if ( vsBlob == nullptr ||
         FAILED( GET_D3D_DEVICE->CreateInputLayout( layout, ARRAYSIZE( layout ), vsBlob->GetBufferPointer(),
                                                    vsBlob->GetBufferSize(), &m_pInputLayout ) ) )
    {
        g_pDebug->printError( "Grass: failed to create input layout" );
        m_pInputLayout = nullptr;
        return false;
    }
    return true;
}

CGrassRendererRH::Mesh *CGrassRendererRH::GetMesh( RpGeometry *geometry )
{
    for ( auto &entry : m_aMeshes )
        if ( entry.first == geometry )
            return entry.second.indexCount ? &entry.second : nullptr;

    Mesh mesh;
    const RwV3d *verts   = geometry->morphTarget ? geometry->morphTarget[0].verts : nullptr;
    const RwV3d *normals = geometry->morphTarget ? geometry->morphTarget[0].normals : nullptr;
    const RwTexCoords *uvs = geometry->texCoords[0];
    if ( verts && geometry->triangles && geometry->numVertices > 0 && geometry->numTriangles > 0 )
    {
        std::vector<GrassVertex> vertices( geometry->numVertices );
        for ( int i = 0; i < geometry->numVertices; i++ )
        {
            vertices[i].pos    = verts[i];
            vertices[i].uv     = uvs ? uvs[i] : RwTexCoords{ 0, 0 };
            vertices[i].normal = normals ? normals[i] : RwV3d{ 0, 0, 1 };
        }
        std::vector<RwUInt16> indices;
        indices.reserve( geometry->numTriangles * 3 );
        for ( int i = 0; i < geometry->numTriangles; i++ )
            for ( int v = 0; v < 3; v++ )
                indices.push_back( geometry->triangles[i].vertIndex[v] );

        D3D11_BUFFER_DESC bd{};
        bd.Usage     = D3D11_USAGE_IMMUTABLE;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.ByteWidth = (UINT)( vertices.size() * sizeof( GrassVertex ) );
        D3D11_SUBRESOURCE_DATA data{ vertices.data(), 0, 0 };
        HRESULT hr = GET_D3D_DEVICE->CreateBuffer( &bd, &data, &mesh.vertexBuffer );
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.ByteWidth = (UINT)( indices.size() * sizeof( RwUInt16 ) );
        data.pSysMem = indices.data();
        if ( SUCCEEDED( hr ) && SUCCEEDED( GET_D3D_DEVICE->CreateBuffer( &bd, &data, &mesh.indexBuffer ) ) )
            mesh.indexCount = (UINT)indices.size();
    }
    else
        g_pDebug->printError( "Grass: plant geometry has no vertex data" );
    m_aMeshes.emplace_back( geometry, mesh );
    return mesh.indexCount ? &m_aMeshes.back().second : nullptr;
}

void CGrassRendererRH::RenderDeferred()
{
    if ( !Init() )
        return;

    UpdatePlantLimits();
    m_aInstances.clear();
    m_aInstanceKeys.clear();
    m_bCollecting = true;
    CPlantMgr__Render();
    m_bCollecting = false;
    if ( m_aInstances.empty() )
        return;

    // group the instances by (mesh, texture); stable so the game's order is kept inside a batch
    std::vector<UINT> order( m_aInstances.size() );
    for ( UINT i = 0; i < order.size(); i++ )
        order[i] = i;
    std::stable_sort( order.begin(), order.end(), []( UINT l, UINT r ) { return m_aInstanceKeys[l] < m_aInstanceKeys[r]; } );

    auto context = GET_D3D_CONTEXT;
    if ( m_nInstanceCapacity < m_aInstances.size() )
    {
        if ( m_pInstanceBuffer )
            m_pInstanceBuffer->Release();
        m_pInstanceBuffer   = nullptr;
        m_nInstanceCapacity = max( kMinInstanceCapacity, (UINT)m_aInstances.size() * 3 / 2 );
        D3D11_BUFFER_DESC bd{};
        bd.Usage          = D3D11_USAGE_DYNAMIC;
        bd.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bd.ByteWidth      = m_nInstanceCapacity * sizeof( Instance );
        if ( FAILED( GET_D3D_DEVICE->CreateBuffer( &bd, nullptr, &m_pInstanceBuffer ) ) )
        {
            g_pDebug->printError( "Grass: failed to create instance buffer" );
            m_nInstanceCapacity = 0;
            return;
        }
    }

    std::vector<Batch> batches;
    D3D11_MAPPED_SUBRESOURCE mapped{};
    if ( FAILED( context->Map( m_pInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped ) ) )
        return;
    Instance *dst = static_cast<Instance *>( mapped.pData );
    for ( UINT i = 0; i < order.size(); i++ )
    {
        const auto &key = m_aInstanceKeys[order[i]];
        dst[i]          = m_aInstances[order[i]];
        if ( batches.empty() || batches.back().geometry != key.first || batches.back().texture != key.second )
            batches.push_back( { key.first, key.second, i, 0 } );
        batches.back().count++;
    }
    context->Unmap( m_pInstanceBuffer, 0 );

    g_pRenderBuffersMgr->SetMatrixBuffer();
    g_pStateMgr->SetInputLayout( m_pInputLayout );
    g_pStateMgr->SetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    g_pStateMgr->SetAlphaBlendEnable( FALSE );
    g_pStateMgr->SetDepthEnable( TRUE );
    g_pStateMgr->SetZWriteEnable( true );
    g_pStateMgr->SetCullMode( rwCULLMODECULLNONE );
    g_pStateMgr->SetTextureAdressUV( rwTEXTUREADDRESSWRAP );
    g_pStateMgr->SetTextureFilterMode( rwFILTERLINEARMIPLINEAR );
    m_pVS->Set();
    m_pPS->Set();

    const UINT instanceStride = sizeof( Instance ), instanceOffset = 0;
    for ( const Batch &batch : batches )
    {
        Mesh *mesh = GetMesh( batch.geometry );
        if ( mesh == nullptr )
            continue;
        g_pStateMgr->SetVertexBuffer( mesh->vertexBuffer, sizeof( GrassVertex ), 0 );
        g_pStateMgr->SetIndexBuffer( mesh->indexBuffer );
        g_pStateMgr->SetRaster( batch.texture ? batch.texture->raster : nullptr );
        g_pStateMgr->FlushStates();
        context->IASetVertexBuffers( 1, 1, &m_pInstanceBuffer, &instanceStride, &instanceOffset );
        context->DrawIndexedInstanced( mesh->indexCount, batch.count, 0, 0, batch.first );
    }
    ID3D11Buffer *none = nullptr;
    const UINT zero    = 0;
    context->IASetVertexBuffers( 1, 1, &none, &zero, &zero );
    g_pStateMgr->SetCullMode( rwCULLMODECULLBACK );
}

void CGrassRendererRH::Shutdown()
{
    for ( auto &entry : m_aMeshes )
    {
        if ( entry.second.vertexBuffer )
            entry.second.vertexBuffer->Release();
        if ( entry.second.indexBuffer )
            entry.second.indexBuffer->Release();
    }
    m_aMeshes.clear();
    if ( m_pInstanceBuffer )
        m_pInstanceBuffer->Release();
    if ( m_pInputLayout )
        m_pInputLayout->Release();
    delete m_pVS;
    delete m_pPS;
    m_pInstanceBuffer = nullptr;
    m_pInputLayout    = nullptr;
    m_pVS             = nullptr;
    m_pPS             = nullptr;
    m_bInitialized    = false;
}
