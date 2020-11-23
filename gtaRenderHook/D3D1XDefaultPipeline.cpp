// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "D3D1XDefaultPipeline.h"
#include "CDebug.h"
#include "D3DRenderer.h"
#include "D3D1XShader.h"
#include "D3D1XTexture.h"
#include "D3D1XStateManager.h"
#include "D3D1XEnumParser.h"
#include "RwD3D1XEngine.h"
#include "D3D1XRenderBuffersManager.h"
#include "D3D1XVertexDeclarationManager.h"
#include "D3D1XVertexDeclaration.h"
#include "D3D1XVertexBufferManager.h"
#include "D3D1XVertexBuffer.h"
#include "D3D1XIndexBuffer.h"
#include "RwVectorMath.h"

CD3D1XDefaultPipeline::CD3D1XDefaultPipeline() :
    CD3D1XPipeline( "RwMain" )
{
}


CD3D1XDefaultPipeline::~CD3D1XDefaultPipeline()
{
}

bool CD3D1XDefaultPipeline::Instance( void *object, RxD3D9ResEntryHeader *resEntryHeader, RwBool reinstance ) const
{
    RpAtomic* atomic = static_cast<RpAtomic*>( object );
    RpGeometry* geom = atomic->geometry;
    resEntryHeader->totalNumVertex = geom->numVertices;
    // create vertex declaration
    // TODO: add more robust vertex declaration generation
    auto vdeclPtr = CD3D1XVertexDeclarationManager::AddNew( m_pVS, geom->flags | rpGEOMETRYNORMALS | rpGEOMETRYPRELIT |
                   rpGEOMETRYTEXTURED | rpGEOMETRYPOSITIONS | 0x00000100 );
    resEntryHeader->vertexDeclaration = vdeclPtr->getInputLayout();
     
    // copy vertex data to data structure that represents vertex in shader
    // TODO: add ability to create custom verticle types on fly,
    // currently even if mesh doesn't have colors/textures they are still filled, that could be potentional performance hit.
    SimpleVertex* vertexData = new SimpleVertex[resEntryHeader->totalNumVertex];

    bool	hasNormals = geom->morphTarget[0].normals != nullptr, 
        hasTexCoords = geom->texCoords[0] != nullptr,
        hasColors = geom->preLitLum && geom->flags&rpGEOMETRYPRELIT;
    
    for ( RwUInt32 i = 0; i < resEntryHeader->totalNumVertex; i++ )
    {
        vertexData[i].pos = geom->morphTarget[0].verts[i];
        vertexData[i].normal = ( hasNormals &&
                                 geom->morphTarget[0].normals[i].x != NAN &&
                                 geom->morphTarget[0].normals[i].y != NAN &&
                                 geom->morphTarget[0].normals[i].z != NAN ) ? geom->morphTarget[0].normals[i] : RwV3d{ 0, 0, 0 };
        vertexData[i].uv = hasTexCoords ? geom->texCoords[0][i] : RwTexCoords{ 0, 0 };
        vertexData[i].color =
            hasColors ? geom->preLitLum[i] : RwRGBA{255, 255, 255, 255};
        vertexData[i].tangent   = {0.0f, 0.0f, 0.0f};
        vertexData[i].bitangent = {0.0f, 0.0f, 0.0f};
    }
    // if mesh doesn't have normals generate them on the fly
    if ( !hasNormals )
        GenerateNormals( vertexData, resEntryHeader->totalNumVertex,
                         geom->triangles, geom->numTriangles, false );
    else
        GenerateTangentsOnly( vertexData, resEntryHeader->totalNumVertex,
                              geom->triangles, geom->numTriangles, false );

    D3D11_SUBRESOURCE_DATA InitData;

    InitData.SysMemPitch = 0;
    InitData.SysMemSlicePitch = 0;
    InitData.pSysMem = vertexData; 

    auto buffer = new CD3D1XVertexBuffer( sizeof( SimpleVertex ) * resEntryHeader->totalNumVertex, &InitData );
    resEntryHeader->vertexStream[0].vertexBuffer = buffer;
    CD3D1XVertexBufferManager::AddNew( buffer );

    delete[] vertexData;

    return true;
}

bool CD3D1XDefaultPipeline::Instance( void * object, RxD3D9ResEntryHeader * resEntryHeader, RwBool reinstance, const std::vector<RxVertexIndex>& indexBuffer ) const
{
    RpAtomic* atomic = static_cast<RpAtomic*>( object );
    RpGeometry* geom = atomic->geometry;
    resEntryHeader->totalNumVertex = geom->numVertices;
    // create vertex declaration
    // TODO: add more robust vertex declaration generation
    auto vdeclPtr = CD3D1XVertexDeclarationManager::AddNew( m_pVS, geom->flags | rpGEOMETRYNORMALS | rpGEOMETRYPRELIT |
                   rpGEOMETRYTEXTURED | rpGEOMETRYPOSITIONS | 0x00000100 );
    resEntryHeader->vertexDeclaration = vdeclPtr->getInputLayout();
    
    // copy vertex data to data structure that represents vertex in shader
    // TODO: add ability to create custom verticle types on fly,
    // currently even if mesh doesn't have colors/textures they are still filled, that could be potentional performance hit.
    SimpleVertex* vertexData = new SimpleVertex[resEntryHeader->totalNumVertex];

    bool	hasNormals = geom->morphTarget[0].normals != nullptr,
        hasTexCoords = geom->texCoords[0] != nullptr,
        hasColors = geom->preLitLum && geom->flags&rpGEOMETRYPRELIT;

    for ( RwUInt32 i = 0; i < resEntryHeader->totalNumVertex; i++ )
    {
        vertexData[i].pos = geom->morphTarget[0].verts[i];
        vertexData[i].normal = ( hasNormals &&
                                 geom->morphTarget[0].normals[i].x != NAN &&
                                 geom->morphTarget[0].normals[i].y != NAN &&
                                 geom->morphTarget[0].normals[i].z != NAN ) ? geom->morphTarget[0].normals[i] : RwV3d{ 0, 0, 0 };
        vertexData[i].uv =
            hasTexCoords ? geom->texCoords[0][i] : RwTexCoords{0, 0};
        vertexData[i].color =
            hasColors ? geom->preLitLum[i] : RwRGBA{255, 255, 255, 255};
        vertexData[i].tangent   = {0.0f, 0.0f, 0.0f};
        vertexData[i].bitangent = {0.0f, 0.0f, 0.0f};

    }
    // if mesh doesn't have normals generate them on the fly
    if ( !hasNormals )
        GenerateNormals( resEntryHeader, vertexData,
                         resEntryHeader->totalNumVertex, indexBuffer );

    D3D11_SUBRESOURCE_DATA InitData;

    InitData.SysMemPitch = 0;
    InitData.SysMemSlicePitch = 0;
    InitData.pSysMem = vertexData;

    auto buffer = new CD3D1XVertexBuffer( sizeof( SimpleVertex ) * resEntryHeader->totalNumVertex, &InitData );
    resEntryHeader->vertexStream[0].vertexBuffer = buffer;
    CD3D1XVertexBufferManager::AddNew( buffer );

    delete[] vertexData;

    return true;
}

void CD3D1XDefaultPipeline::Render( RwResEntry * repEntry, void * object, RwUInt8 type, RwUInt32 flags )
{
    //RpAtomic* atomic = static_cast<RpAtomic*>(object);
    RxInstanceData* entryData = static_cast<RxInstanceData*>( repEntry );
    // early return
    if ( entryData->header.totalNumIndex == 0 )
        return;
    // initialize mesh states
    // TODO: reduce casts
    g_pStateMgr->SetInputLayout( static_cast<ID3D11InputLayout*>( entryData->header.vertexDeclaration ) );
    g_pStateMgr->SetVertexBuffer( ( (CD3D1XVertexBuffer*)entryData->header.vertexStream[0].vertexBuffer )->getBuffer(), sizeof( SimpleVertex ), 0 );
    g_pStateMgr->SetIndexBuffer( ( (CD3D1XIndexBuffer*)entryData->header.indexBuffer )->getBuffer() );
    g_pStateMgr->SetPrimitiveTopology( CD3D1XEnumParser::ConvertPrimTopology( (RwPrimitiveType)entryData->header.primType ) );
    m_pVS->Set();
    m_pPS->Set();

    // iterate over materials and draw them
    RwUInt8 alphaBlend;
    RxD3D9InstanceData* model;
    RpMaterial* material;
    for ( RwUInt32 i = 0; i < static_cast<size_t>( entryData->header.numMeshes ); i++ )
    {
        model = &GetModelsData( entryData )[i];
        material = model->material;
        if ( material->color.alpha == 0 ) continue;
        alphaBlend = material->color.alpha != 255 || model->vertexAlpha;

        // set albedo(diffuse) color
        g_pRenderBuffersMgr->UpdateMaterialDiffuseColor( material->color );
        // set texture
        if ( material->texture )
        {
            alphaBlend |= GetD3D1XRaster( material->texture->raster )->alpha;
            g_pRwCustomEngine->SetTexture( material->texture, 0 );
        }
        // set alpha blending
        g_pStateMgr->SetAlphaBlendEnable( alphaBlend > 0 );
        // flush and draw
        g_pStateMgr->FlushStates();
        g_pRenderBuffersMgr->FlushMaterialBuffer();
        GET_D3D_RENDERER->DrawIndexed( model->numIndex, model->startIndex, model->minVert );
    }
}

void CD3D1XDefaultPipeline::GenerateNormals( SimpleVertex * verticles, unsigned int vertexCount, RpTriangle* triangles, unsigned int triangleCount, bool isTriStrip )
{
    auto fast_abs = []( float x ) { return x > 0 ? x : -x; };
    // generate normal for each triangle and vertex in mesh
    for ( RwUInt32 i = 0; i < triangleCount; i++ )
    {
        auto triangle = triangles[i];
        bool swapIds = isTriStrip;
        auto iA = triangle.vertIndex[0], iB = triangle.vertIndex[1], iC = triangle.vertIndex[2];

        if ( swapIds&&i % 2 == 0 )
        {
            RwUInt16 t = iC;
            iC = iB;
            iB = t;
        }

        auto vA = verticles[iA],
            vB = verticles[iB],
            vC = verticles[iC];
        // tangent vector
        RwV3d edge1 = {
            vB.pos.x - vA.pos.x,
            vB.pos.y - vA.pos.y,
            vB.pos.z - vA.pos.z
        };
        // bitangent vector
        RwV3d edge2 = {
            vC.pos.x - vA.pos.x,
            vC.pos.y - vA.pos.y,
            vC.pos.z - vA.pos.z
        };

        RwTexCoords tex0      = verticles[iA].uv;
        RwTexCoords tex1      = verticles[iB].uv;
        RwTexCoords tex2      = verticles[iC].uv;

        RwV2d uv1 = { tex1.u - tex0.u, tex1.v - tex0.v};
        RwV2d uv2  = {tex2.u - tex0.u, tex2.v - tex0.v};
        float diff = ( uv1.x * uv2.y - uv1.y * uv2.x );
        RW::V3d edge01, edge02, cp;
        edge01 = {edge1.x, uv1.x, uv1.y};
        edge02 = {edge2.x, uv2.x, uv2.y};
        cp     = edge01.cross( edge02 );
        constexpr auto SMALL_FLOAT = 1e-12f;

        if ( (RwReal)fast_abs( cp.getX() ) > SMALL_FLOAT )
        {
            const RwReal invcpx = 1.f / cp.getX();

            verticles[iA].tangent.x += -cp.getY() * invcpx;

            verticles[iB].tangent.x += -cp.getY() * invcpx;

            verticles[iC].tangent.x += -cp.getY() * invcpx;
        }

        /* y, s, t */
        edge01 = {edge1.y, uv1.x, uv1.y};

        edge02 = {edge2.y, uv2.x, uv2.y};

        cp = edge01.cross( edge02 );
        if ( (RwReal)fast_abs( cp.getX() ) > SMALL_FLOAT )
        {
            const RwReal invcpx = 1.f / cp.getX();

            verticles[iA].tangent.y += -cp.getY() * invcpx;

            verticles[iB].tangent.y += -cp.getY() * invcpx;

            verticles[iC].tangent.y += -cp.getY() * invcpx;
        }

        /* z, s, t */
        edge01 = {edge1.z, uv1.x, uv1.y};

        edge02 = {edge2.z, uv2.x, uv2.y};

        cp = edge01.cross( edge02 );
        if ( (RwReal)fast_abs( cp.getX() ) > SMALL_FLOAT )
        {
            const RwReal invcpx = 1.f / cp.getX();

            verticles[iA].tangent.z += -cp.getY() * invcpx;
                                           
            verticles[iB].tangent.z += -cp.getY() * invcpx;
                                           
            verticles[iC].tangent.z += -cp.getY() * invcpx;
        }

        RwV3d normal = {( edge1.y * edge2.z - edge1.z * edge2.y ),
                        ( edge1.z * edge2.x - edge1.x * edge2.z ),
                        ( edge1.x * edge2.y - edge1.y * edge2.x )};

        // increase normals of each vertex in triangle 
        verticles[iA].normal = {
            verticles[iA].normal.x + normal.x,
            verticles[iA].normal.y + normal.y,
            verticles[iA].normal.z + normal.z
        };
        verticles[iB].normal = {
            verticles[iB].normal.x + normal.x,
            verticles[iB].normal.y + normal.y,
            verticles[iB].normal.z + normal.z
        };
        verticles[iC].normal = {
            verticles[iC].normal.x + normal.x,
            verticles[iC].normal.y + normal.y,
            verticles[iC].normal.z + normal.z
        };
    }

    for ( RwUInt32 i = 0; i < vertexCount; i++ )
    {
        RwReal length = sqrt( verticles[i].normal.x * verticles[i].normal.x +
                              verticles[i].normal.y * verticles[i].normal.y +
                              verticles[i].normal.z * verticles[i].normal.z );
        if ( length > 0.001f )
            verticles[i].normal = {verticles[i].normal.x / length,
                                   verticles[i].normal.y / length,
                                   verticles[i].normal.z / length};
        RW::V3d t = verticles[i].tangent;
        /*if ( tangentsRemapping[i] == i )
        {*/
        t.normalize();
        verticles[i].tangent = t.getRWVector();
    }
    /*
    std::vector<RwUInt32> tangentsRemapping;
    tangentsRemapping.reserve( vertexCount );
    for ( RwUInt32 i = 0; i < vertexCount; i++ )
    {
        tangentsRemapping.push_back( i );
    }
    for ( RwUInt32 i = 0; i < vertexCount; i++ )
    {
        auto &currentTangent = verticles[i].tangent;

        for ( RwUInt32 j = i + 1; j < vertexCount; j++ )
        {
            if ( tangentsRemapping[j] > i )
            {
                if ( fabs( verticles[i].pos.x - verticles[j].pos.x ) <=
                         0.000001f &&
                     fabs( verticles[i].pos.y - verticles[j].pos.y ) <=
                         0.000001f &&
                     fabs( verticles[i].pos.z - verticles[j].pos.z ) <=
                         0.000001f )
                {
                    // Check same normal 
                    RwReal cosangle;
                    cosangle = RW::V3d( verticles[i].normal )
                                   .dot( RW::V3d( verticles[j].normal ) );

                    if ( cosangle >= 0.9999f )
                    {
                        // Check similar tangents 
                        cosangle = RW::V3d( currentTangent )
                                       .dot( RW::V3d( verticles[j].tangent ) );
                        cosangle /= RW::V3d( currentTangent ).length();
                        cosangle /= RW::V3d( verticles[j].tangent ).length();

                        if ( cosangle > 0.7f )
                        {
                            // accumulate 
                            currentTangent.x += verticles[j].tangent.x;
                            currentTangent.y += verticles[j].tangent.y;
                            currentTangent.z += verticles[j].tangent.z;

                            tangentsRemapping[j] = i;
                        }
                    }
                }
            }
        }
    }
    */
    /* normalize normals
    for ( RwUInt32 i = 0; i < vertexCount; i++ )
    {
        RW::V3d t = verticles[i].tangent;
        if ( tangentsRemapping[i] == i )
        {/
            t.normalize();
            verticles[i].tangent = t.getRWVector();
        }
        else
        {
            verticles[i].tangent = verticles[tangentsRemapping[i]].tangent;
        }
    }*/
}

void CD3D1XDefaultPipeline::GenerateTangentsOnly( SimpleVertex *verticles,
                                                  unsigned int  vertexCount,
                                                  RpTriangle *  triangles,
                                                  unsigned int  triangleCount,
                                                  bool          isTriStrip )
{
    // generate normal for each triangle and vertex in mesh
    for ( RwUInt32 i = 0; i < triangleCount; i++ )
    {
        auto triangle = triangles[i];
        bool swapIds  = isTriStrip;
        auto iA = triangle.vertIndex[0], iB = triangle.vertIndex[1],
             iC = triangle.vertIndex[2];

        if ( swapIds && i % 2 == 0 )
        {
            RwUInt16 t = iC;
            iC         = iB;
            iB         = t;
        }

        auto vA = verticles[iA], vB = verticles[iB], vC = verticles[iC];
        // tangent vector
        RwV3d edge1 = {vB.pos.x - vA.pos.x, vB.pos.y - vA.pos.y,
                       vB.pos.z - vA.pos.z};
        // bitangent vector
        RwV3d edge2 = {vC.pos.x - vA.pos.x, vC.pos.y - vA.pos.y,
                       vC.pos.z - vA.pos.z};

        RwTexCoords tex0 = verticles[iA].uv;
        RwTexCoords tex1 = verticles[iB].uv;
        RwTexCoords tex2 = verticles[iC].uv;

        RwV2d uv1  = {tex1.u - tex0.u, tex1.v - tex0.v};
        RwV2d uv2  = {tex2.u - tex0.u, tex2.v - tex0.v};
        float diff = ( uv1.x * uv2.y - uv1.y * uv2.x );
        if ( diff < 0.00001F && diff > -0.00001F )
           continue;

        float r = 1.0f / diff;

        RwV3d tangent{( ( edge1.x * uv2.y ) - ( edge2.x * uv1.y ) ) * r,
                      ( ( edge1.y * uv2.y ) - ( edge2.y * uv1.y ) ) * r,
                      ( ( edge1.z * uv2.y ) - ( edge2.z * uv1.y ) ) * r};

        RwV3d bitangent{( ( edge2.x * uv1.x ) - ( edge1.x * uv2.x ) ) * r,
                        ( ( edge2.y * uv1.x ) - ( edge1.y * uv2.x ) ) * r,
                        ( ( edge2.z * uv1.x ) - ( edge1.z * uv2.x ) ) * r}; 

        verticles[iA].tangent = {verticles[iA].tangent.x + tangent.x,
                                 verticles[iA].tangent.y + tangent.y,
                                 verticles[iA].tangent.z + tangent.z};
        verticles[iB].tangent = {verticles[iB].tangent.x + tangent.x,
                                 verticles[iB].tangent.y + tangent.y,
                                 verticles[iB].tangent.z + tangent.z};
        verticles[iC].tangent = {verticles[iC].tangent.x + tangent.x,
                                 verticles[iC].tangent.y + tangent.y,
                                 verticles[iC].tangent.z + tangent.z};

        verticles[iA].bitangent = {verticles[iA].bitangent.x + bitangent.x,
                                   verticles[iA].bitangent.y + bitangent.y,
                                   verticles[iA].bitangent.z + bitangent.z};
        verticles[iB].bitangent = {verticles[iB].bitangent.x + bitangent.x,
                                   verticles[iB].bitangent.y + bitangent.y,
                                   verticles[iB].bitangent.z + bitangent.z};
        verticles[iC].bitangent = {verticles[iC].bitangent.x + bitangent.x,
                                   verticles[iC].bitangent.y + bitangent.y,
                                   verticles[iC].bitangent.z + bitangent.z};
    }
    // normalize normals
    for ( RwUInt32 i = 0; i < vertexCount; i++ )
    {
        RW::V3d n = verticles[i].normal;
        RW::V3d t = verticles[i].tangent;
        RW::V3d b = verticles[i].bitangent;

        // make tangent orthoganal to normal
        t = t - n * n.dot( t );
        t.normalize();

        if ( n.cross( t ).dot( b ) < 0.0f )
        {
            t = t * -1.0f; 
        }
        verticles[i].tangent = {t.getX(), t.getY(), t.getZ()};
    }
}

void CD3D1XDefaultPipeline::GenerateNormals(
    RxD3D9ResEntryHeader *resEntryHeader, SimpleVertex *verticles,
    unsigned int vertexCount, const std::vector<RxVertexIndex> &indexBuffer )
{// generate normal for each triangle and vertex in mesh
    for ( unsigned int i = 0; i < resEntryHeader->numMeshes; i++ )
    {
        auto &       meshData = GetModelsData2( resEntryHeader )[i];
        unsigned int id_start = meshData.startIndex;
        auto         numIndices = meshData.numIndex;
        if ( resEntryHeader->primType == D3DPT_TRIANGLESTRIP )
        {
            numIndices -= 2;
        }
        int n = 0;
        while ( n < numIndices ) {
            int iA, iB, iC;
            if ( resEntryHeader->primType == D3DPT_TRIANGLELIST )
            {
                iA = indexBuffer[id_start + n + 0] + meshData.minVert;
                iB = indexBuffer[id_start + n + 1] + meshData.minVert;
                iC = indexBuffer[id_start + n + 2] + meshData.minVert;
                n += 3;
            }
            else /* if (meshHeader->primType == D3DPT_TRIANGLESTRIP) */
            {
                if ( i & 0x01 )
                {
                    iA = indexBuffer[id_start + n + 2] + meshData.minVert;
                    iB = indexBuffer[id_start + n + 1] + meshData.minVert;
                    iC = indexBuffer[id_start + n + 0] + meshData.minVert;
                }
                else
                {
                    iA = indexBuffer[id_start + n + 0] + meshData.minVert;
                    iB = indexBuffer[id_start + n + 1] + meshData.minVert;
                    iC = indexBuffer[id_start + n + 2] + meshData.minVert;
                }
                n++;

                if ( iA == iB || iA == iC || iB == iC )
                {
                    continue;
                }
            }

            auto vA = verticles[iA], vB = verticles[iB], vC = verticles[iC];

             // tangent vector
            RwV3d edge1 = {vB.pos.x - vA.pos.x, vB.pos.y - vA.pos.y,
                           vB.pos.z - vA.pos.z};
            // bitangent vector
            RwV3d edge2 = {vC.pos.x - vA.pos.x, vC.pos.y - vA.pos.y,
                           vC.pos.z - vA.pos.z};

            // normal vector as cross product of (tangent X bitangent)
            RwV3d normal = {
                ( edge1.y * edge2.z - edge1.z * edge2.y ),
                ( edge1.z * edge2.x - edge1.x * edge2.z ),
                ( edge1.x * edge2.y - edge1.y * edge2.x )};
            // increase normals of each vertex in triangle       
            // increase normals of each vertex in triangle 
            verticles[iA].normal = {verticles[iA].normal.x + normal.x,
                                    verticles[iA].normal.y + normal.y,
                                    verticles[iA].normal.z + normal.z};
            verticles[iB].normal = {verticles[iB].normal.x + normal.x,
                                    verticles[iB].normal.y + normal.y,
                                    verticles[iB].normal.z + normal.z};
            verticles[iC].normal = {verticles[iC].normal.x + normal.x,
                                    verticles[iC].normal.y + normal.y,
                                    verticles[iC].normal.z + normal.z};
        }
    }
    // normalize normals
    for ( RwUInt32 i = 0; i < vertexCount; i++ )
    {
        RwReal length = sqrt( verticles[i].normal.x*verticles[i].normal.x + verticles[i].normal.y*verticles[i].normal.y + verticles[i].normal.z*verticles[i].normal.z );
        verticles[i].normal = { verticles[i].normal.x / length, verticles[i].normal.y / length, verticles[i].normal.z / length };
    }
    // TANGENTs
}
