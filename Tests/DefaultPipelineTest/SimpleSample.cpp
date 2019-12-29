#include "SimpleSample.h"
#include <filesystem>
#include <RWUtils/RwAPI.h>
#include <DebugUtils/DebugLogger.h>
#include <Engine/Common/IRenderingContext.h>
#include <Engine/D3D11Impl/Buffers/D3D11ConstantBuffer.h>
#include <Engine/D3D11Impl/Buffers/D3D11IndexBuffer.h>
#include <Engine/D3D11Impl/D3D11PrimitiveBatch.h>
#include <ImGUI/imgui.h>
#include "clump_read_funcs.h"
#include "rp_geometry_funcs.h"
#include "rp_material_read_funcs.h"
#include "rw_frame_funcs.h"
#include "forward_pbr_pipeline.h"
#include <Engine/D3D11Impl/D3D11RenderingContext.h>
#include <chrono>

using namespace RHEngine;
std::unordered_map<std::string, RwTexture*> g_ideTextureList;
std::vector<std::string> g_ideLoadedTextureList;
std::unordered_map<std::string, MaterialCB> g_materialBuffer;
MaterialBufferProvider g_materialBufferProvider;

const std::string dff_path = "Bank.dff";
const std::string models_path = "models_3/";
const std::string textures_path = "textures_3/";
const std::string ides_path = "ides_3/";
const std::string ipls_path = "ipls_3_fast/";
const std::string ipls_bin_path = "ipls_bin_sa/";
const int32_t ipl_version = 0;

enum RenderStatus 
{
    Failure,
    NotInstanced,
    Instanced
};


struct rwResEnty_ : RwResEntry
{
    D3D11PrimitiveBatch* modelInstance;
    uint32_t padd;
};

void MeshGetNumVerticesMinIndex( const RxVertexIndex* indices, uint32_t size,
                                   RwUInt32& numVertices,
                                   RwUInt32& min )
{
    if( size > 0 )
    {
        RwUInt32    minVert = 0xFFFFFFFF;
        RwUInt32    maxVert = 0x00000000;

        /* Find min and max vertex index */
        for( size_t i = 0; i < size; i++ )
        {
            minVert = min( minVert, indices[i] );
            maxVert = max( maxVert, indices[i] );
        }

        numVertices = ( maxVert - minVert ) + 1;
        min = minVert;
    }
    else
    {
        numVertices = 0;
        min = 0;
    }

}

struct RxTriangle 
{
    RxVertexIndex a, b, c;
};

static bool SortTriangles( const RxTriangle& pA, const RxTriangle& pB ) noexcept
{
    RwUInt32 sortedIndexA[3] = { pA.a, pA.b, pA.c };
    RwUInt32 sortedIndexB[3] = { pB.a, pB.b, pB.c };

    if( sortedIndexA[0] > sortedIndexA[1] )
        std::swap( sortedIndexA[0], sortedIndexA[1] );
    
    if( sortedIndexA[1] > sortedIndexA[2] )
    {
        std::swap( sortedIndexA[1], sortedIndexA[2] );

        if( sortedIndexA[0] > sortedIndexA[1] )
        {
            std::swap( sortedIndexA[0], sortedIndexA[1] );
        }
    }

    if( sortedIndexB[0] > sortedIndexB[1] )
    {
        std::swap( sortedIndexB[0], sortedIndexB[1] );
    }

    if( sortedIndexB[1] > sortedIndexB[2] )
    {
        std::swap( sortedIndexB[1], sortedIndexB[2] );

        if( sortedIndexB[0] > sortedIndexB[1] )
        {
            std::swap( sortedIndexB[0], sortedIndexB[1] );
        }
    }

    if( sortedIndexA[0] == sortedIndexB[0] )
    {
        if( sortedIndexA[1] == sortedIndexB[1] )
        {
            return ( sortedIndexA[2] < sortedIndexB[2] );
        }

        return ( sortedIndexA[1] < sortedIndexB[1] );
    }

    return ( sortedIndexA[0] < sortedIndexB[0] );
}

RwTexture* _RwTextureRead( const RwChar* name, const RwChar* maskName ) {
    std::string tex_name( name );
    std::string tex_name_lc( tex_name );

    std::transform( tex_name.begin(), tex_name.end(), tex_name_lc.begin(), ::tolower );
    g_materialBuffer[tex_name] = {};
    return g_ideTextureList[tex_name] ? g_ideTextureList[tex_name] : g_ideTextureList[tex_name_lc];
}

void GenerateNormals( RHVertexDescPosColorUVNormals* verticles, 
                      uint32_t vertexCount, 
                      RpTriangle* triangles, unsigned int triangleCount,
                      bool isTriStrip )
{
    // generate normal for each triangle and vertex in mesh
    for( RwUInt32 i = 0; i < triangleCount; i++ )
    {
        auto triangle = triangles[i];
        bool swapIds = isTriStrip;
        auto iA = triangle.vertIndex[0], iB = triangle.vertIndex[1], iC = triangle.vertIndex[2];

        /*if( swapIds&& i % 2 == 0 )
        {
            RwUInt16 t = iC;
            iC = iB;
            iB = t;
        }*/

        const auto vA = verticles[iA],
            vB = verticles[iB],
            vC = verticles[iC];
        // tangent vector
        RwV3d tangent = {
            vB.x - vA.x,
            vB.y - vA.y,
            vB.z - vA.z
        };
        // bitangent vector
        RwV3d bitangent = {
            vA.x - vC.x,
            vA.y - vC.y,
            vA.z - vC.z
        };
        // fix for triangle strips

        //float normalDirection = isTriStrip?(i % 2 == 0 ? 1.0f : -1.0f):1.0f;
        // normal vector as cross product of (tangent X bitangent)
        RwV3d normal = {
            ( tangent.y * bitangent.z - tangent.z * bitangent.y ),
            ( tangent.z * bitangent.x - tangent.x * bitangent.z ),
            ( tangent.x * bitangent.y - tangent.y * bitangent.x )
        };
        // increase normals of each vertex in triangle 
        
        verticles[iA].nx = verticles[iA].nx + normal.x;
        verticles[iA].ny = verticles[iA].ny + normal.y;
        verticles[iA].nz = verticles[iA].nz + normal.z;

        verticles[iB].nx = verticles[iB].nx + normal.x;
        verticles[iB].ny = verticles[iB].ny + normal.y;
        verticles[iB].nz = verticles[iB].nz + normal.z;

        verticles[iC].nx = verticles[iC].nx + normal.x;
        verticles[iC].ny = verticles[iC].ny + normal.y;
        verticles[iC].nz = verticles[iC].nz + normal.z;
    }
    // normalize normals
    for( RwUInt32 i = 0; i < vertexCount; i++ )
    {
        RwReal length = sqrt( verticles[i].nx * verticles[i].nx + verticles[i].ny * verticles[i].ny + verticles[i].nz * verticles[i].nz );
        if( length > 0.000001f )
        {
            verticles[i].nx = verticles[i].nx / length;
            verticles[i].ny = verticles[i].ny / length;
            verticles[i].nz = verticles[i].nz / length;
        }
    }
}

RwResEntry* RHInstanceAtomic( RpAtomic* atomic, void* owner, RwResEntry*& resEntryPointer,
                              const RpMeshHeader* meshHeader )
{
    
    rwResEnty_* resEntry;

    resEntry = (rwResEnty_*)malloc( sizeof( rwResEnty_ ) );
    resEntryPointer = resEntry;
    if( resEntry == nullptr )
        return nullptr;

    /* Show its not in the arena lists */
    resEntry->link.next = nullptr;
    resEntry->link.prev = nullptr;
    resEntry->size = sizeof( void* );
    resEntry->owner = nullptr;
    resEntry->ownerRef = nullptr;
    resEntry->destroyNotify = []( RwResEntry *resEntry ) noexcept
    {
        rwResEnty_ *entry = reinterpret_cast<rwResEnty_*>( resEntry );
        if( entry->modelInstance != nullptr )
            delete entry->modelInstance;
    };

    RHEngine::D3D11IndexBuffer* d3dIndexBuffer = nullptr;
    RHEngine::D3D11VertexBuffer* d3dVertexBuffer = nullptr;

    RHEngine::RHPrimitiveType primType = RHEngine::RHPrimitiveType::TriangleStrip;
    if( meshHeader->flags & rpMESHHEADERTRISTRIP )
        primType = RHEngine::RHPrimitiveType::TriangleStrip;
    else if( (meshHeader->flags & rpMESHHEADERPRIMMASK) == 0 )
        primType = RHEngine::RHPrimitiveType::TriangleList;

    const RpMesh* mesh_start = (const RpMesh*)( meshHeader + 1 );
    RxVertexIndex* indexBuffer = new RxVertexIndex[meshHeader->totalIndicesInMesh*3];
    uint32_t startIndex = 0;
    uint32_t indexCount = 0;
    
    std::vector< RHEngine::MeshSplitData > splitData;
    // Index data
    for( const RpMesh* mesh = mesh_start; mesh != mesh_start + meshHeader->numMeshes; mesh++ )
    {
        indexCount = mesh->numIndices;

        RHEngine::MeshSplitData meshData{};
        meshData.startIndex = startIndex;
        meshData.numIndex = indexCount;
        meshData.material = mesh->material;
        uint32_t minVert;

        MeshGetNumVerticesMinIndex( mesh->indices, indexCount, meshData.numVertices, minVert );

        for( size_t i = startIndex; i < startIndex + indexCount; i++ )
            indexBuffer[i] = mesh->indices[i - startIndex];

        if( primType == RHEngine::RHPrimitiveType::TriangleList ) {
            std::sort(  (RxTriangle*)( indexBuffer + startIndex ),
                        (RxTriangle*)( indexBuffer + startIndex + indexCount ),
                        SortTriangles );
        }
        startIndex += indexCount;

        splitData.push_back( meshData );
    }
    {
        D3D11_SUBRESOURCE_DATA subresData{};
        subresData.pSysMem = indexBuffer;

        RHEngine::RHIndexBufferInfo bufferInfo;
        bufferInfo.indexCount = meshHeader->totalIndicesInMesh;
        bufferInfo.isDynamic = false;
        bufferInfo.initialData = &subresData;

        RHEngine::g_pRHRenderer->GetGPUAllocator()->AllocateIndexBuffer( bufferInfo, reinterpret_cast<void*&> ( d3dIndexBuffer ) );
    }
    delete[] indexBuffer;

    // Vertex data
    RHVertexDescPosColorUVNormals* vertexData = new RHVertexDescPosColorUVNormals[atomic->geometry->numVertices];

    RwV3d* vertexPos = atomic->geometry->morphTarget[0].verts;
    RwTexCoords* vertexUV = atomic->geometry->texCoords[0];
    RwRGBA* vertexColorPtr = atomic->geometry->preLitLum;
    RwV3d* normalsPtr = atomic->geometry->morphTarget[0].normals;

    uint32_t v_id = 0;
    for( ; vertexPos != atomic->geometry->morphTarget[0].verts + atomic->geometry->numVertices; vertexPos++ )
    {
        RHVertexDescPosColorUVNormals desc{};
        desc.x = vertexPos->x; 
        desc.y = vertexPos->y; 
        desc.z = vertexPos->z;
        desc.w = 1.0f;
        if( normalsPtr ) {
            desc.nx = normalsPtr->x;
            desc.ny = normalsPtr->y;
            desc.nz = normalsPtr->z;
            normalsPtr++;
        }
        else
        {
            desc.nx = desc.ny = desc.nz = 0;
        }
        if( vertexColorPtr ) {
            desc.color = *vertexColorPtr;
            vertexColorPtr++;
        }
        else
        {
            desc.color = { 255,255,255,255 };
        }
        if( vertexUV ) {
            desc.u = vertexUV->u;
            desc.v = vertexUV->v;
            vertexUV++;
        }
        else
        {
            desc.u = 0;
            desc.v = 0;
        }
        vertexData[v_id] = desc;
        v_id++;
    }
    GenerateNormals( vertexData, atomic->geometry->numVertices, atomic->geometry->triangles, atomic->geometry->numTriangles, 
                     primType == RHEngine::RHPrimitiveType::TriangleStrip );
    {
        D3D11_SUBRESOURCE_DATA vertSubresData{};
        vertSubresData.pSysMem = vertexData;

        RHEngine::RHVertexBufferInfo vertexBufferInfo;
        vertexBufferInfo.vertexCount = atomic->geometry->numVertices;
        vertexBufferInfo.vertexSize = sizeof( RHVertexDescPosColorUVNormals );
        vertexBufferInfo.isDynamic = false;
        vertexBufferInfo.initialData = &vertSubresData;
        RHEngine::g_pRHRenderer->GetGPUAllocator()->AllocateVertexBuffer( vertexBufferInfo, reinterpret_cast<void*&> ( d3dVertexBuffer ) );
    }

    delete[] vertexData;


    resEntry->modelInstance = new D3D11PrimitiveBatch( meshHeader->serialNum, d3dIndexBuffer, d3dVertexBuffer, primType, splitData );

    return (RwResEntry*)resEntry;
}

RenderStatus RwRHInstanceAtomic( RpAtomic* atomic )
{
    RpGeometry* geometry = atomic->geometry;

    // Early return if geometry has no vertices
    if( geometry->numVertices <= 0 )
        return RenderStatus::NotInstanced;

    RpMeshHeader* meshHeader = geometry->mesh;

    if( meshHeader == nullptr )
        return RenderStatus::Failure;

    if( meshHeader->numMeshes <= 0 )
        return RenderStatus::NotInstanced;

    const RwUInt32 geomFlags = geometry->flags;

    RwResEntry* resEntry = nullptr;

    if( !( rpGEOMETRYNATIVE & geomFlags ) )
    {
        /* If the geometry has more than one morph target the resEntry in the
         * atomic is used else the resEntry in the geometry */
        if( geometry->numMorphTargets != 1 )
        {
            resEntry = atomic->repEntry;
        }
        else
        {
            resEntry = geometry->repEntry;
        }

        /* If the meshes have changed we should re-instance */
        if( resEntry )
        {
            RHEngine::D3D11PrimitiveBatch* resEntryHeader;

            resEntryHeader = (D3D11PrimitiveBatch*)( resEntry + 1 );
            if( resEntryHeader->BatchId() != meshHeader->serialNum )
            {
                /* Destroy resources to force reinstance */
                //RwResourcesFreeResEntry( resEntry );
                RHDebug::DebugLogger::Log( "test" );
                resEntry = nullptr;
            }
        }
        if( resEntry == nullptr ) 
        {
            RwResEntry*& resEntryPointer = geometry->repEntry;
            void* owner;

            meshHeader = geometry->mesh;

            if( geometry->numMorphTargets != 1 )
            {
                owner = (void*)atomic;
                resEntryPointer = atomic->repEntry;
            }
            else
            {
                owner = (void*)geometry;
                resEntryPointer = geometry->repEntry;
            }

            /*
             * Create vertex buffers and instance
             */
            resEntry = RHInstanceAtomic( atomic, owner, geometry->repEntry, meshHeader );
            if( resEntry == nullptr )
                return RenderStatus::Failure;

            /* The geometry is up to date */
            geometry->lockedSinceLastInst = 0;
        }
        return RenderStatus::Instanced;
    }
    else
        return RenderStatus::Failure;
}

void SimpleSample::RenderUI()
{
    //ImGui::ShowDemoWindow();
    ImGui::Begin( "Scene info" );
    ImGui::Text( "FPS:%f", m_fFPS );
    if( ImGui::CollapsingHeader( "Rendering time info" ) )
    {
        ImGui::Text( "Culling time:%fms\nRendering time:%fms", m_fCullingTime, m_fRenderingTime );
        for( size_t i = 0; i < m_nWorkerThreadCount; i++ )
        {
            ImGui::Text( "Thread#%u rendering time:%fms", i, m_fRenderingThreadTime[i] );
        }
        ImGui::Text( "Rendered Objects count:%u", m_nVisibleObjects );
    }
    ImGui::Checkbox( "Use constant draw distance", &m_bUseConstDrawDist );
    ImGui::SliderInt( "Thread count", &m_nWorkerThreadCount, 0, m_nMaxThreadCount );
    ImGui::SliderInt( "Min draws per thread", &m_nMinMeshPerThread, 1, 10000 );
    ImGui::SliderFloat( "Constant draw distance", &m_fMaxDrawDist, 1, 10000 );
    ImGui::SliderFloat( "Camera speed", &m_fCamSpeed, 1, 1000 );

    if( ImGui::CollapsingHeader( "Models info" ) ) {
        ImGui::InputText( "Filter", m_sModelFilter, 32 );
        ImGui::BeginChild( "Scrolling" );
        int id = 0;
        for( auto& it : m_iplModelInfoMap )
        {
            if( m_ideModelList[it.modelId].dffname.compare( 0, strlen( m_sModelFilter ), m_sModelFilter ) != 0 )
                continue;
            ImGui::PushID( id );
            //ImGui::BeginChild(  );
            if( ImGui::CollapsingHeader( m_ideModelList[it.modelId].dffname.c_str() ) )
            {
                bool selected = it.selected;
                ImGui::Checkbox( "Highlight", &selected );
                it.selected = selected;

                ImGui::Text( "DrawDist:%f" , m_ideModelList[it.modelId].drawDist );
                ImGui::Text( "Matrix:\t m00:%f\tm01 : %f\tm02 : %f"
                               "\n\t\t\tm10:%f\tm11:%f\tm12:%f"
                               "\n\t\t\tm20:%f\tm21:%f\tm22:%f"
                               "\n\t\t\tm30:%f\tm31:%f\tm32:%f",
                             it.transform.r[0].vector4_f32[0], it.transform.r[0].vector4_f32[1], it.transform.r[0].vector4_f32[2],
                             it.transform.r[1].vector4_f32[0], it.transform.r[1].vector4_f32[1], it.transform.r[1].vector4_f32[2],
                             it.transform.r[2].vector4_f32[0], it.transform.r[2].vector4_f32[1], it.transform.r[2].vector4_f32[2],
                             it.transform.r[3].vector4_f32[0], it.transform.r[3].vector4_f32[1], it.transform.r[3].vector4_f32[2] );
                float jump_dist = m_ideModelList[it.modelId].drawDist;
                if( ImGui::Button( "Jump to" ) ) 
                {
                    m_pMainCameraFrame->ltm.pos = {
                        -m_pMainCameraFrame->ltm.at.x* jump_dist + it.transform.r[3].vector4_f32[0],
                        -m_pMainCameraFrame->ltm.at.y* jump_dist + it.transform.r[3].vector4_f32[1],
                        -m_pMainCameraFrame->ltm.at.z* jump_dist + it.transform.r[3].vector4_f32[2] };
                }
                if( ImGui::Button( "Jump to, and highlight" ) ) 
                {
                    m_pMainCameraFrame->ltm.pos = {
                        -m_pMainCameraFrame->ltm.at.x * jump_dist + it.transform.r[3].vector4_f32[0],
                        -m_pMainCameraFrame->ltm.at.y * jump_dist + it.transform.r[3].vector4_f32[1],
                        -m_pMainCameraFrame->ltm.at.z * jump_dist + it.transform.r[3].vector4_f32[2] };
                    it.selected = true;
                }
            }
            ImGui::PopID();
            id++;
        }
        ImGui::EndChild();
    }
    if( ImGui::CollapsingHeader( "Materials info" ) ) 
    {
        
        ImGui::InputText( "Filter", m_sTexFilter, 32 );
        ImGui::BeginChild( "Scrolling" );
        int id = 0;
        for( auto& it : g_materialBuffer )
        {
            if( it.first.compare( 0, strlen( m_sTexFilter ), m_sTexFilter ) != 0 )
                continue;
            ImGui::PushID( id );
            if( ImGui::CollapsingHeader( it.first.c_str() ) )
            {
                ImGui::ColorEdit4( "MaterialColor", it.second.MaterialColor );
                ImGui::DragFloat( "Roughness", &it.second.Roughness, 0.01f, 0.0f, 1.0f );
                ImGui::DragFloat( "Metallness", &it.second.Metallness, 0.01f, 0.0f, 1.0f );
            }
            ImGui::PopID();
            id++;
        }
        ImGui::EndChild();
    }

    ImGui::End();
}

void SimpleSample::LoadIDE( const RHEngine::String& path )
{
    std::ifstream ide_fstream( path );
    char line[256];
    do {
        memset( line, 0, 256 );
        ide_fstream.getline( line, 256, '\n' );
    } while ( memcmp( line, "objs", 4 ) != 0 );

    memset( line, 0, 256 );
    ide_fstream.getline( line, 256, '\n' );
    while( memcmp( line, "end", 3 ) != 0 )
    {
        uint32_t model_id, obj_count;
        char dff_name[64];
        char txd_name[64];
        memset( dff_name, 0, 64 );
        memset( txd_name, 0, 64 );
        float drawDist;

        for( int i = 0; i < 256; i++ )
        {
            if( line[i] < ' ' || line[i] == ',' )
                line[i] = ' ';
        }
        sscanf( line, "%u %s %s %u %f", &model_id, dff_name, txd_name,
                &obj_count, &drawDist );

        if( m_ideModelList.find( model_id ) == m_ideModelList.end() &&
            memcmp( dff_name, "LOD", 3 ) != 0 && memcmp( dff_name, "IslandLOD", 9 ) != 0 )
        {
            m_ideModelList[model_id].object = nullptr;
            m_ideModelList[model_id].drawDist = drawDist;
            m_ideModelList[model_id].dffname = std::string( dff_name );
            m_ideModelList[model_id].txdname = std::string( txd_name );
        }
        memset( line, 0, 256 );
        ide_fstream.getline( line, 256, '\n' );
    };
    ide_fstream.close();
}

void SimpleSample::LoadIPL( const RHEngine::String& path, int32_t version )
{
    std::ifstream ipl_fstream( path );
    char line[256];
    while( memcmp(line,"inst", 4 ) != 0 ) {
        memset( line, 0, 256 );
        ipl_fstream.getline( line, 256, '\n' );
    }
    memset( line, 0, 256 );
    ipl_fstream.getline( line, 256, '\n' );
    while( memcmp( line, "end", 3 ) != 0 )
    {
        uint32_t model_id, interior, lod_id;
        char dff_name[64];
        RwV3d pos, scale{1,1,1};
        RwV4d quatRot;
        for( int i = 0; i < 256; i++ ) 
        {
            if( line[i] < ' ' || line[i] == ',' )
                line[i] = ' ';
        }
        if( version == 0 ) 
        {
            sscanf( line, "%u %s %f %f %f %f %f %f %f %f %f %f", &model_id, dff_name,
                    &pos.x, &pos.y, &pos.z,
                    &scale.x, &scale.y, &scale.z,
                    &quatRot.x, &quatRot.y, &quatRot.z, &quatRot.w );
        }
        else if( version == 1 )
        {
            sscanf( line, "%u %s %u %f %f %f %f %f %f %f %f %f %f", &model_id, dff_name, &interior,
                    &pos.x, &pos.y, &pos.z,
                    &scale.x, &scale.y, &scale.z,
                    &quatRot.x, &quatRot.y, &quatRot.z, &quatRot.w );
        }
        else if( version >= 2 )
        {
            sscanf( line, "%u %s %u %f %f %f %f %f %f %f %u", &model_id, dff_name, &interior,
                    &pos.x, &pos.y, &pos.z,
                    &quatRot.x, &quatRot.y, &quatRot.z, &quatRot.w, &lod_id );
        }
        DirectX::XMMATRIX transMat = DirectX::XMMatrixTranslation( pos.x, pos.y, pos.z );
        DirectX::XMMATRIX scaleMat = DirectX::XMMatrixScaling( scale.x, scale.y, scale.z );
        DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationQuaternion( { quatRot.x, quatRot.y, -quatRot.z, quatRot.w } );
        ModelInfo info = { model_id, rotMat * scaleMat * transMat };

        if( m_ideModelList.find( model_id ) != m_ideModelList.end() )
        {
            if( m_ideModelList[model_id].object == nullptr ) 
            {
                if( std::find( g_ideLoadedTextureList.begin(), g_ideLoadedTextureList.end(), m_ideModelList[model_id].txdname ) == g_ideLoadedTextureList.end() )
                {
                    RwTexDictionary* texDict = RH_RWAPI::GTAReadTexDict( textures_path + std::string( m_ideModelList[model_id].txdname ) + ".txd" );

                    RwTexture * result;
                    RwLLLink * cur, *end;
                    cur = rwLinkListGetFirstLLLink( &texDict->texturesInDict );
                    end = rwLinkListGetTerminator( &texDict->texturesInDict );

                    while( cur != end )
                    {
                        result = rwLLLinkGetData( cur, RwTexture, lInDictionary );

                        g_ideTextureList.insert( std::pair<std::string, RwTexture*>( std::string( result->name ), result ) );

                        cur = rwLLLinkGetNext( cur );
                    }
                    g_ideLoadedTextureList.push_back( std::string( m_ideModelList[model_id].txdname ) );
                }
                else
                    RHDebug::DebugLogger::Log( "FAILTXD" );
                if( std::experimental::filesystem::exists( models_path + std::string( m_ideModelList[model_id].dffname ) + ".dff" ) )
                {
                    RpClump* res;
                    RH_RWAPI::LoadClump( res, models_path + std::string( m_ideModelList[model_id].dffname ) + ".dff" );
                    m_ideModelList[model_id].object = res;
                }
                else
                    RHDebug::DebugLogger::Log( "FAILDFF:" + std::string( m_ideModelList[model_id].dffname ) );
            }
            if( m_ideModelList[model_id].object != nullptr )
                m_iplModelInfoMap.push_back( info );
            else
                RHDebug::DebugLogger::Log( "FAILCLMP" );
        }
        else
            RHDebug::DebugLogger::Log( "FAILIDE" );
        memset( line, 0, 256 );
        ipl_fstream.getline( line, 256, '\n' );
    };
    ipl_fstream.close();
}

void SimpleSample::LoadIPLBinary( const RHEngine::String& path )
{
    std::ifstream ipl_fstream( path, std::ios_base::binary );
    char identifier[4];
    ipl_fstream.read( identifier, 4 );

    if( memcmp( identifier, "bnry", 4 ) != 0 ) 
    {
        RHDebug::DebugLogger::Log( "incorrect binary ipl" );
        return;
    }
    struct bnry_ipl_header {
        int32_t item_inst_count, unk1_count, unk2_count, unk3_count, car_count, unk4_count;
        int32_t item_inst_offset;
    } header;
    ipl_fstream.read( (char*)&header, sizeof( header ) );
    ipl_fstream.seekg( header.item_inst_offset );
    for( int i= 0;i< header.item_inst_count;i++ )
    {
        uint32_t model_id;
        int32_t interior, lod_id;
        RwV3d pos;
        RwV4d quatRot;
        ipl_fstream.read( (char*)& pos, sizeof( pos ) );
        ipl_fstream.read( (char*)& quatRot, sizeof( quatRot ) );
        ipl_fstream.read( (char*)& model_id, sizeof( model_id ) );
        ipl_fstream.read( (char*)& interior, sizeof( interior ) );
        ipl_fstream.read( (char*)& lod_id, sizeof( lod_id ) );

        DirectX::XMMATRIX transMat = DirectX::XMMatrixTranslation( pos.x, pos.y, pos.z );
        DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationQuaternion( { quatRot.x, quatRot.y, -quatRot.z, quatRot.w } );
        ModelInfo info = { model_id,rotMat * transMat };
        if( m_ideModelList.find( model_id ) != m_ideModelList.end() )
        {
            if( m_ideModelList[model_id].object == nullptr )
            {
                if( std::find( g_ideLoadedTextureList.begin(), g_ideLoadedTextureList.end(), m_ideModelList[model_id].txdname ) == g_ideLoadedTextureList.end() )
                {
                    RwTexDictionary* texDict = RH_RWAPI::GTAReadTexDict( textures_path + std::string( m_ideModelList[model_id].txdname ) + ".txd" );

                    RwTexture * result;
                    RwLLLink * cur, *end;
                    cur = rwLinkListGetFirstLLLink( &texDict->texturesInDict );
                    end = rwLinkListGetTerminator( &texDict->texturesInDict );

                    while( cur != end )
                    {
                        result = rwLLLinkGetData( cur, RwTexture, lInDictionary );

                        g_ideTextureList.insert( std::pair<std::string, RwTexture*>( std::string( result->name ), result ) );

                        cur = rwLLLinkGetNext( cur );
                    }
                    g_ideLoadedTextureList.push_back( std::string( m_ideModelList[model_id].txdname ) );
                }
                else
                    RHDebug::DebugLogger::Log( "FAILTXD" );
                if( std::experimental::filesystem::exists( models_path + std::string( m_ideModelList[model_id].dffname ) + ".dff" ) )
                {
                    RpClump* res;
                    RH_RWAPI::LoadClump( res, models_path + std::string( m_ideModelList[model_id].dffname ) + ".dff" );
                    m_ideModelList[model_id].object = res;
                }
                else
                    RHDebug::DebugLogger::Log( "FAILDFF:" + std::string( m_ideModelList[model_id].dffname ) );
            }
            if( m_ideModelList[model_id].object != nullptr )
                m_iplModelInfoMap.push_back( info );
            else
                RHDebug::DebugLogger::Log( "FAILCLMP" );
        }
        else
            RHDebug::DebugLogger::Log( "FAIL" );
    }
    ipl_fstream.close();
}

void SimpleSample::LoadDFFForIPL( const RHEngine::String& models_path )
{
    for( auto [id, clump]: m_ideModelList ) {
        RwLLLink* cur, * end, * next;
        if( clump.object ) {
            clump.boundSphereRad = 0;
            cur = rwLinkListGetFirstLLLink( &clump.object->atomicList );
            end = rwLinkListGetTerminator( &clump.object->atomicList );

            while( cur != end )
            {
                RpAtomic* atomic = rwLLLinkGetData( cur, RpAtomic, inClumpLink );
                next = rwLLLinkGetNext( cur );

                RwRHInstanceAtomic( atomic );
                clump.boundSphereRad = max( clump.boundSphereRad, atomic->boundingSphere.radius );
                rwResEnty_* resEntry = (rwResEnty_*)atomic->geometry->repEntry;
                auto splitData = resEntry->modelInstance->SplitData();
                clump.hasAlpha = std::any_of( splitData.begin(), splitData.end(),
                                              [](const RHEngine::MeshSplitData& mesh_data ) {
                                                  if( mesh_data.material ) 
                                                  {
                                                      if( mesh_data.material->texture ) 
                                                      {
                                                          if( mesh_data.material->texture->raster )
                                                          {
                                                              return (mesh_data.material->texture->raster->cFormat& rwRASTERFORMAT8888) != 0 ||
                                                                  (mesh_data.material->texture->raster->cFormat & rwRASTERFORMAT1555) != 0 ||
                                                                  (mesh_data.material->texture->raster->cFormat & rwRASTERFORMAT4444) != 0;
                                                          }
                                                      }
                                                  }
                                                  return false;
                                              } );
                cur = next;
            }
        }
    }
}

void SimpleSample::DrawClump( RHEngine::IRenderingContext* context, RpClump* clump, bool selected, DirectX::XMMATRIX transf_mult )
{
    RwLLLink* cur, * end, * next;
    cur = rwLinkListGetFirstLLLink( &clump->atomicList );
    end = rwLinkListGetTerminator( &clump->atomicList );

    while( cur != end )
    {
        RpAtomic* atomic = rwLLLinkGetData( cur, RpAtomic, inClumpLink );
        next = rwLLLinkGetNext( cur );

        rwResEnty_* resEntry = (rwResEnty_*)atomic->geometry->repEntry;
        if( resEntry != nullptr )
        {

            const RwMatrix* ltm = RH_RWAPI::_RwFrameGetLTM ( (RwFrame*)rwObjectGetParent( atomic ) );
            DirectX::XMMATRIX objTransformMatrix = {
                ltm->right.x, ltm->right.y, ltm->right.z, 0,
                ltm->up.x, ltm->up.y, ltm->up.z, 0,
                ltm->at.x, ltm->at.y, ltm->at.z, 0,
                ltm->pos.x, ltm->pos.y, ltm->pos.z, 1
            };
            objTransformMatrix = objTransformMatrix * transf_mult;
            context->UpdateBuffer( (void*)mPerModelConstantBuffer, &objTransformMatrix, sizeof( objTransformMatrix ) );
            context->BindConstantBuffers( RHEngine::RHShaderStage::Vertex |
                                          RHEngine::RHShaderStage::Pixel,
                                          { { 0, (void*)mBaseConstantBuffer }, { 1, (void*)mPerModelConstantBuffer } } );
            m_pForwardPBRPipeline->DrawObject( context, resEntry->modelInstance, { selected } );
            //DrawMesh( context, resEntry->modelInstance, selected );
        }
        cur = next;
    }
}

void SimpleSample::RenderWorker( std::uint32_t id )
{
    while( true )
    {

        WaitForSingleObject( m_nBeginRenderingEvents[id], INFINITE );
        if( m_bThreadShouldStaph )
            return;
        using namespace std::chrono;
        auto timept = high_resolution_clock::now();
        void* frameBufferInternal = RH_RWAPI::GetInternalRaster( m_pMainCamera->frameBuffer );
        mDeferredContextList[id]->BindImageBuffers( RHImageBindType::RenderTarget, { { 0, frameBufferInternal } } );

        if( m_pMainCamera->zBuffer )
        {
            void* zBufferInternal = RH_RWAPI::GetInternalRaster( m_pMainCamera->zBuffer );
            if( zBufferInternal )
            {
                mDeferredContextList[id]->BindImageBuffers( RHImageBindType::DepthStencilTarget, { { 0, zBufferInternal } } );
            }
        }

        // Set viewport
        if( m_pMainCamera->frameBuffer )
        {
            RHViewPort vp{};
            vp.width = (float)m_pMainCamera->frameBuffer->width;
            vp.height = (float)m_pMainCamera->frameBuffer->height;
            vp.maxDepth = 1.0;
            mDeferredContextList[id]->BindViewPorts( { vp } );
        }
        mDeferredContextList[id]->BindConstantBuffers( RHEngine::RHShaderStage::Vertex |
                                               RHEngine::RHShaderStage::Pixel,
                                               { { 0, (void*)mBaseConstantBuffer }, { 1, (void*)mPerModelConstantBuffer } } );
        RHEngine::D3D11RenderingContext* d3d_ctxt = ( RHEngine::D3D11RenderingContext* )mDeferredContextList[id];
        d3d_ctxt->GetStateCache()->GetBlendStateCache()->SetBlendEnable( true, 0 );
        for( uint32_t i = m_batches[id].start; i < m_batches[id].end; i++ )
        {
            DrawClump( mDeferredContextList[id], m_vCurrentFrameRenderList[i].data.object, m_vCurrentFrameRenderList[i].info.selected, 
                       m_vCurrentFrameRenderList[i].info.transform );
        }
        m_aCommandListBuffer[id] = nullptr;
        mDeferredContextList[id]->FinishCmdList( m_aCommandListBuffer[id] );
        m_fRenderingThreadTime[id] = duration_cast<duration<float, std::milli>>( high_resolution_clock::now() - timept ).count();
        m_nCmdListCount = m_nCmdListCount - 1;
        SetEvent( m_nEndRenderingEvents[id] );
    }
}

void SimpleSample::CustomRender()
{
    RwRGBA clearColor = { 128, 128, 255, 255 };
    RHEngine::g_pRWRenderEngine->CameraClear( m_pMainCamera, &clearColor, rwCAMERACLEARIMAGE | rwCAMERACLEARZ );

    IRenderingContext* context = (IRenderingContext*)g_pRHRenderer->GetCurrentContext();
    context->UpdateBuffer( (void*)mBaseConstantBuffer, &RHEngine::g_cameraContext, sizeof( RHEngine::g_cameraContext ) );
    using namespace std::chrono;
    auto t1 = high_resolution_clock::now();
    
    g_pRWRenderEngine->RenderStateSet( rwRENDERSTATEVERTEXALPHAENABLE, 1 );

    // construct render_list
    DirectX::XMVECTOR toCam;
    for( auto it = m_iplModelInfoMap.begin(); it != m_iplModelInfoMap.end(); it++ ) {
        
        toCam = {
            m_pMainCameraFrame->ltm.pos.x - it->transform.r[3].vector4_f32[0],
            m_pMainCameraFrame->ltm.pos.y - it->transform.r[3].vector4_f32[1],
            m_pMainCameraFrame->ltm.pos.z - it->transform.r[3].vector4_f32[2], 0 };

        const float camDist = DirectX::XMVector3Length( toCam ).vector4_f32[0];
        float minDrawDist = m_bUseConstDrawDist ? m_fMaxDrawDist : m_ideModelList[it->modelId].drawDist;

        const float drawDist = minDrawDist + m_ideModelList[it->modelId].boundSphereRad;

        if( camDist < drawDist && m_ideModelList[it->modelId].object )
        {
            //if( m_ideModelList[it->modelId].hasAlpha )
            //    m_vCurrentFrameAlphaRenderList.push_back( { m_ideModelList[it->modelId], *it } );
            //else
                m_vCurrentFrameRenderList.push_back( { m_ideModelList[it->modelId], *it } );
        }
    }
    m_nVisibleObjects = m_vCurrentFrameRenderList.size(); 
    std::sort( m_vCurrentFrameRenderList.begin(), m_vCurrentFrameRenderList.end(), []( const RenderingData & a, const RenderingData & b )
               {
                   return a.data.drawDist < b.data.drawDist;
               } );
    m_fCullingTime = duration_cast<duration<float,std::milli>>( high_resolution_clock::now() - t1 ).count();
    /*std::sort( m_vCurrentFrameAlphaRenderList.begin(), m_vCurrentFrameAlphaRenderList.end(), []( const RenderingData & a, const RenderingData & b )
               {
                   return a.data.drawDist > b.data.drawDist;
               } );*/
    t1 = high_resolution_clock::now();
    uint32_t per_thread_req = max( m_vCurrentFrameRenderList.size() / max( m_nWorkerThreadCount, 1 ), m_nMinMeshPerThread );
    uint32_t cmd_buf_count = 0;
    m_nCmdListCount = 0;
    std::vector<HANDLE> event_wait_list;
    if( m_vCurrentFrameRenderList.size() >= per_thread_req )
    {
        int k;
        for( k = 1; k <= m_nWorkerThreadCount; k++ ) {
            if( per_thread_req * (k + 1) >= m_vCurrentFrameRenderList.size() ) {
                m_batches[k - 1].start = per_thread_req * k;
                m_batches[k - 1].end = m_vCurrentFrameRenderList.size();
                m_nCmdListCount++;

                SetEvent( m_nBeginRenderingEvents[k - 1] );
                event_wait_list.push_back( m_nEndRenderingEvents[k - 1] );
                break;
            }
            else 
            {
                m_batches[k - 1].start = per_thread_req * k;
                m_batches[k - 1].end = per_thread_req * ( k + 1 );
                m_nCmdListCount++;
                SetEvent( m_nBeginRenderingEvents[k-1] );
                event_wait_list.push_back( m_nEndRenderingEvents[k-1] );
            }
        }
        cmd_buf_count = k;

    }

    context->BindConstantBuffers( RHEngine::RHShaderStage::Vertex |
                                                   RHEngine::RHShaderStage::Pixel,
                                                   { { 0, (void*)mBaseConstantBuffer }, { 1, (void*)mPerModelConstantBuffer } } );

    for( uint32_t i = 0; i < min( per_thread_req, m_vCurrentFrameRenderList.size() ); i++ )
    {
        DrawClump( context, m_vCurrentFrameRenderList[i].data.object, m_vCurrentFrameRenderList[i].info.selected,
                   m_vCurrentFrameRenderList[i].info.transform );
    }
    if( event_wait_list.size() > 0 )
        WaitForMultipleObjects( event_wait_list.size(), event_wait_list.data(), TRUE, INFINITE );

    for( int i = 0; i < cmd_buf_count; i++ ) 
    {
        if( m_aCommandListBuffer[i] )
            context->ReplayCmdList( m_aCommandListBuffer[i] );
    }
    for( size_t i = 0; i < m_nMaxThreadCount; i++ )
    {
        m_aCommandListBuffer[i] = nullptr;
    }
    m_fRenderingTime = duration_cast<duration<float, std::milli>>( high_resolution_clock::now() - t1 ).count();
    /*for( auto model : m_vCurrentFrameAlphaRenderList )
    {
        mCurrentTransformMultiplier = model.info.transform;
        DrawClump( model.data.object, model.info.selected );
    }*/
    m_vCurrentFrameAlphaRenderList.clear();
    m_vCurrentFrameRenderList.clear();
    RenderUI();
    //DrawClump( mCurrentModel );
}

void SimpleSample::CustomUpdate( float dt )
{
    if( dt != 0.0f )
        m_fFPS = 1.0f / dt;
    DIMOUSESTATE mouseCurrState{};
    if( GetKeyState( VK_CONTROL ) & 0x8000 ) 
    {
        if( !mouse_aq ) {
            m_pMouse->Acquire();
            mouse_aq = true;
        }
        m_pMouse->GetDeviceState( sizeof( DIMOUSESTATE ), &mouseCurrState );
    }
    else 
    {
        if( mouse_aq ) {
            m_pMouse->Unacquire();
            mouse_aq = false;
        }
    }

    RwV2d mousePosRw = { mouseCurrState.lX * 0.005f, mouseCurrState.lY * 0.005f };

    DirectX::XMMATRIX currentRot = {
        m_pMainCameraFrame->ltm.right.x, m_pMainCameraFrame->ltm.right.y, m_pMainCameraFrame->ltm.right.z, 0,
        m_pMainCameraFrame->ltm.up.x, m_pMainCameraFrame->ltm.up.y, m_pMainCameraFrame->ltm.up.z, 0,
        m_pMainCameraFrame->ltm.at.x, m_pMainCameraFrame->ltm.at.y, m_pMainCameraFrame->ltm.at.z, 0,
        0, 0, 0, 1
    };

    DirectX::XMMATRIX rightRotAxis =
        DirectX::XMMatrixRotationAxis(
            {
                m_pMainCameraFrame->ltm.right.x,
                m_pMainCameraFrame->ltm.right.y,
                m_pMainCameraFrame->ltm.right.z
            },
            ( mousePosRw.y ) );

    DirectX::XMMATRIX upRotAxis =
        DirectX::XMMatrixRotationAxis(
            {
                -m_pMainCameraFrame->ltm.up.x,
                -m_pMainCameraFrame->ltm.up.y,
                -m_pMainCameraFrame->ltm.up.z
            },
            ( mousePosRw.x ) );

    const DirectX::XMMATRIX rotMat = currentRot * rightRotAxis * upRotAxis;
    m_pMainCameraFrame->ltm.right = { rotMat.r[0].vector4_f32[0], rotMat.r[0].vector4_f32[1], rotMat.r[0].vector4_f32[2] };
    m_pMainCameraFrame->ltm.up =    { rotMat.r[1].vector4_f32[0], rotMat.r[1].vector4_f32[1], rotMat.r[1].vector4_f32[2] };
    m_pMainCameraFrame->ltm.at =    { rotMat.r[2].vector4_f32[0], rotMat.r[2].vector4_f32[1], rotMat.r[2].vector4_f32[2] };

    RwV3d& camPos = m_pMainCameraFrame->ltm.pos;

    const float camSpeed = dt*m_fCamSpeed;
    if( GetKeyState( 0x57 ) & 0x8000 )
    {
        camPos.x += m_pMainCameraFrame->ltm.at.x * camSpeed;
        camPos.y += m_pMainCameraFrame->ltm.at.y * camSpeed;
        camPos.z += m_pMainCameraFrame->ltm.at.z * camSpeed;
    }
    if( GetKeyState( 0x53 ) & 0x8000 )
    {
        camPos.x -= m_pMainCameraFrame->ltm.at.x * camSpeed;
        camPos.y -= m_pMainCameraFrame->ltm.at.y * camSpeed;
        camPos.z -= m_pMainCameraFrame->ltm.at.z * camSpeed;
    }
    if( GetKeyState( 0x41 ) & 0x8000 )
    {
        camPos.x += m_pMainCameraFrame->ltm.right.x * camSpeed;
        camPos.y += m_pMainCameraFrame->ltm.right.y * camSpeed;
        camPos.z += m_pMainCameraFrame->ltm.right.z * camSpeed;
    }
    if( GetKeyState( 0x44 ) & 0x8000 )
    {
        camPos.x -= m_pMainCameraFrame->ltm.right.x * camSpeed;
        camPos.y -= m_pMainCameraFrame->ltm.right.y * camSpeed;
        camPos.z -= m_pMainCameraFrame->ltm.right.z * camSpeed;
    }
    if( GetKeyState( 0x45 ) & 0x8000 )
    {
        camPos.x += m_pMainCameraFrame->ltm.up.x * camSpeed;
        camPos.y += m_pMainCameraFrame->ltm.up.y * camSpeed;
        camPos.z += m_pMainCameraFrame->ltm.up.z * camSpeed;
    }
    if( GetKeyState( 0x51 ) & 0x8000 )
    {
        camPos.x -= m_pMainCameraFrame->ltm.up.x * camSpeed;
        camPos.y -= m_pMainCameraFrame->ltm.up.y * camSpeed;
        camPos.z -= m_pMainCameraFrame->ltm.up.z * camSpeed;
    }
}

bool SimpleSample::CustomInitialize() 
{
    TestSample::CustomInitialize();
    m_pForwardPBRPipeline = new ForwardPBRPipeline( &g_materialBufferProvider );
    IGPUAllocator* allocator = g_pRHRenderer->GetGPUAllocator();

    allocator->AllocateConstantBuffer( { sizeof( RHEngine::RHCameraContext ) }, (void*&)mBaseConstantBuffer );
    allocator->AllocateConstantBuffer( { sizeof( DirectX::XMMATRIX ) }, (void*&)mPerModelConstantBuffer );
    m_aCommandListBuffer.resize( m_nMaxThreadCount );
    m_batches.resize( m_nMaxThreadCount );
    m_nCmdListCount = 0;
    for( uint32_t i = 0; i < m_nMaxThreadCount; i++ ) {
        m_nBeginRenderingEvents.push_back( CreateEvent( NULL, FALSE, FALSE, NULL ) );
        m_nEndRenderingEvents.push_back( CreateEvent( NULL, FALSE, FALSE, NULL ) );
        m_fRenderingThreadTime.push_back( 0.0f );
        m_tWorkers.push_back( std::move( std::thread( &SimpleSample::RenderWorker, this, i ) ) );
    }

    for( uint32_t i = 0; i < m_nMaxThreadCount; i++ )
    {
        void* deferredctxt = nullptr;
        allocator->AllocateDeferredContext( deferredctxt );
        mDeferredContextList.push_back( ( RHEngine::IRenderingContext* )deferredctxt );
    }

    namespace fs = std::experimental::filesystem;
    for( auto& p : fs::directory_iterator( ides_path ) )
    {
        fs::path file_path = p.path();
        if( file_path.extension() == ".ide" || file_path.extension() == ".IDE" )
        {
            LoadIDE( file_path.string() );
        }
    }
    for( auto& p : fs::directory_iterator( ipls_path ) )
    {
        fs::path file_path = p.path();
        if( file_path.extension() == ".ipl" || file_path.extension() == ".IPL" )
        {
            LoadIPL( file_path.string(), ipl_version );
        }
    }
    if( ipl_version >= 3 ) {
        for( auto& p : fs::directory_iterator( ipls_bin_path ) )
        {
            fs::path file_path = p.path();
            if( file_path.extension() == ".ipl" || file_path.extension() == ".IPL" )
            {
                LoadIPLBinary( file_path.string() );
            }
        }
    }
    
    m_pMainCameraFrame->ltm.pos = { 53.6399f, -605.708f, 23.1283f };
    LoadDFFForIPL( "" );

    IRenderingContext* context = (IRenderingContext*)g_pRHRenderer->GetCurrentContext();
    context->BindConstantBuffers( RHEngine::RHShaderStage::Vertex, { { 0, (void*)mBaseConstantBuffer }, { 1, (void*)mPerModelConstantBuffer } } );
    g_pRWRenderEngine->RenderStateSet( rwRENDERSTATEVERTEXALPHAENABLE, 1 );
    return true;
}

void SimpleSample::CustomShutdown()
{
    for( uint32_t i = 0; i < m_nMaxThreadCount; i++ ) {
        m_bThreadShouldStaph = true;
        SetEvent( m_nBeginRenderingEvents[i] );
        if( m_tWorkers[i].joinable() )
            m_tWorkers[i].join();
        CloseHandle( m_nBeginRenderingEvents[i] );
        CloseHandle( m_nEndRenderingEvents[i] );
    }
}
