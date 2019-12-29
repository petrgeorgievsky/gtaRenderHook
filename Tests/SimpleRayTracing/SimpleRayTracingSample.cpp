#include "SimpleRayTracingSample.h"
#include <chrono>
#include <RWUtils\RwAPI.h>
#include <queue>
#include <Engine\Common\IRenderingContext.h>
#define _USE_MATH_DEFINES // for C++  
#include <cmath> 
const double M_PI = 3.14;
bool SimpleRayTracingSample::CustomInitialize()
{
    if ( !RHTests::TestSample::CustomInitialize() )
        return false;

    RHEngine::D3D11Texture2DCreateInfo createInfo{ };
    createInfo.width = 1280;
    createInfo.height = 720;
    createInfo.mipCount = 1;
    createInfo.format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    createInfo.allowUnorderedAccessUsage = true;
    createInfo.allowShaderUsage = true;
    createInfo.allowRenderTargetUsage = true;

    RHEngine::ImageBufferInfo createInfo_{};
    createInfo_.type = RHEngine::ImageBufferType::RenderTargetBuffer;
    createInfo_.width = 1280;
    createInfo_.height = 720;
    createInfo_.mipLevels = 1;
    createInfo_.format = RHEngine::ImageBufferFormat::RGBA16;

    ID3D11Device* device = (ID3D11Device*)RHEngine::g_pRHRenderer->GetCurrentDevice();
    RHEngine::IGPUAllocator *allocator = RHEngine::g_pRHRenderer->GetGPUAllocator();
    auto start = std::chrono::high_resolution_clock::now();

    allocator->AllocateImageBuffer( createInfo_, (void*&)m_pRTTextures[0] );
    allocator->AllocateImageBuffer( createInfo_, (void*&)m_pRTTextures[1] );
    allocator->AllocateImageBuffer( createInfo_, (void*&)m_pRTTextures[2] );

    createInfo_.format = RHEngine::ImageBufferFormat::RGBA32;

    allocator->AllocateImageBuffer( createInfo_, (void*&)m_pGBTextures[0] );
    allocator->AllocateImageBuffer( createInfo_, (void*&)m_pGBTextures[1] );

    allocator->AllocateShader( {TEXT( "shaders/d3d11/raytrace.hlsl" ),
                                TEXT( "TraceRays" ),
                                RHEngine::ShaderStage::Compute},
                               (void *&) m_pRTShader );
    allocator->AllocateShader( {TEXT( "shaders/d3d11/accumulate.hlsl" ),
                                TEXT( "Accumulate" ),
                                RHEngine::ShaderStage::Pixel},
                               (void *&) m_pAccShader );
    allocator->AllocateShader( {TEXT( "shaders/d3d11/deferred_pass.hlsl" ),
                                TEXT( "deferred_vs" ),
                                RHEngine::ShaderStage::Vertex},
                               (void *&) m_pDeferredVS );
    allocator->AllocateShader( {TEXT( "shaders/d3d11/deferred_pass.hlsl" ),
                                TEXT( "deferred_ps" ),
                                RHEngine::ShaderStage::Pixel},
                               (void *&) m_pDeferredPS );

    m_pConstantBuffer = new RHEngine::D3D11ConstantBuffer<SceneConstants>( device );
    m_pVariableBuffer = new RHEngine::D3D11ConstantBuffer<SceneVariables>( device );

    m_pSphereList = new RHEngine::D3D11StructuredBuffer<Sphere>( device, 64 );


    // Sampler desc
    // Fullscreen quad
    RwIm2DVertex vtx{};
    float w = 1280, h = 720;
    vtx.x = vtx.y = vtx.u = vtx.v = 0;
    vtx.emissiveColor = 0xFFFFFFFF;
    m_vQuad.push_back( vtx );
    vtx.x = w;
    vtx.y = h;
    vtx.u = vtx.v = 1;
    m_vQuad.push_back( vtx );
    vtx.x = vtx.u = 0;
    vtx.y = h;
    vtx.v = 1;
    m_vQuad.push_back( vtx );
    vtx.x = vtx.y = vtx.u = vtx.v = 0;
    m_vQuad.push_back( vtx );
    vtx.x = w;
    vtx.y = vtx.v = 0;
    vtx.u = 1;
    m_vQuad.push_back( vtx );
    vtx.x = w;
    vtx.y = h;
    vtx.u = vtx.v = 1;
    m_vQuad.push_back( vtx );

    Sphere sphere{};
    sphere.position[0] = 0;
    sphere.position[1] = -511;
    sphere.position[2] = 10;
    sphere.radius = 300.0f;
    sphere.color[0] = sphere.color[1] = sphere.color[2] = sphere.color[3] = 0;
    sphere.color[0] = 1;
    sphere.color[3] = 0.5;
    m_aSpheres.emplace_back( sphere );
    sphere.position[0] = 0;
    sphere.position[1] = -0.5;
    sphere.position[2] = 3.5;
    sphere.radius = 0.5f;
    sphere.color[1] = sphere.color[2] = 0;
    m_aSpheres.emplace_back( sphere );

    // --------------------------------- OBJECT FILE READING ---------------------------------------------
    
    std::ifstream obj_file( "object.obj" );
    RHDebug::DebugLogger::Log( obj_file.is_open() ? 
                               "object.obj file has opened succesfully" :
                               "couldn't open object.obj file" );
    std::string s;

    obj_file >> s;

    while ( s != "v" && !obj_file.eof() )
        obj_file >> s;

    vertex4 v{};

    while ( s == "v" && !obj_file.eof() )
    {
        obj_file >> v.x >> v.y >> v.z;

        m_aVerts.push_back( v );

        obj_file >> s;
    }

    while ( s != "f" && !obj_file.eof() )
        obj_file >> s;

    BVHTreeNode root;
    root.bvh_node_id_l = -1;
    root.bvh_node_id_r = -1;
    root.min_aabb = { 1000, 1000, 1000 };
    root.max_aabb = { -1000, -1000, -1000 };

    struct bbox
    {
        vertex min_;
        int padd_;
        vertex max_;
        int triangle_id;
    } bbox_;

    auto get_c = [] ( vertex min_, vertex max_ ) {
        return vertex{ ( min_.x + max_.x )*0.5f, ( min_.y + max_.y )*0.5f, ( min_.z + max_.z )*0.5f };
    };

    std::vector<bbox> bboxList;
    int tri_id=0;
    while ( s == "f" && !obj_file.eof() )
    {
        int a, b, c, d;

        obj_file >> a >> b >> c;

        Triangle_v2 t{};
        t.a = a - 1;
        t.b = b - 1;
        t.c = c - 1;

        bbox_.min_.x = min( min( m_aVerts[t.a].x, m_aVerts[t.b].x ), m_aVerts[t.c].x );
        bbox_.min_.y = min( min( m_aVerts[t.a].y, m_aVerts[t.b].y ), m_aVerts[t.c].y );
        bbox_.min_.z = min( min( m_aVerts[t.a].z, m_aVerts[t.b].z ), m_aVerts[t.c].z );
        bbox_.max_.x = max( max( m_aVerts[t.a].x, m_aVerts[t.b].x ), m_aVerts[t.c].x );
        bbox_.max_.y = max( max( m_aVerts[t.a].y, m_aVerts[t.b].y ), m_aVerts[t.c].y );
        bbox_.max_.z = max( max( m_aVerts[t.a].z, m_aVerts[t.b].z ), m_aVerts[t.c].z );
        bbox_.triangle_id = tri_id;

        bboxList.push_back( bbox_ );

        root.min_aabb.x = min( root.min_aabb.x, bbox_.min_.x );
        root.min_aabb.y = min( root.min_aabb.y, bbox_.min_.y );
        root.min_aabb.z = min( root.min_aabb.z, bbox_.min_.z );

        root.max_aabb.x = max( root.max_aabb.x, bbox_.max_.x );
        root.max_aabb.y = max( root.max_aabb.y, bbox_.max_.y );
        root.max_aabb.z = max( root.max_aabb.z, bbox_.max_.z );
        tri_id++;
        m_aTris.push_back( t );
         
        obj_file >> s;
    }

    root.bvh_node_id_l = 1;
    root.bvh_node_id_r = 2;
    m_BVHTree.push_back( root );
    std::queue<int> stack;
    std::vector<std::vector<bbox>> bboxList_list;
    std::vector<unsigned int> triLists;
    bboxList_list.push_back( bboxList );
    stack.push( 0 );
    int current_parent_node=0;
    int current_child_node = 1;
    int depth = 4;
    int max_per_leaf_triangles = 2;
    while ( !stack.empty() )
    {
        current_parent_node = stack.front();
        stack.pop();
        auto rootbboxc = get_c( m_BVHTree[current_parent_node].min_aabb, m_BVHTree[current_parent_node].max_aabb );

        BVHTreeNode left_leaf;
        BVHTreeNode right_leaf;
        left_leaf.min_aabb = { 1000, 1000, 1000 };
        left_leaf.max_aabb = { -1000, -1000, -1000 };
        left_leaf.bvh_node_id_l = -1;
        left_leaf.bvh_node_id_r = -1;
        right_leaf.bvh_node_id_l = -1;
        right_leaf.bvh_node_id_r = -1;
        right_leaf.min_aabb = { 1000, 1000, 1000 };
        right_leaf.max_aabb = { -1000, -1000, -1000 };

        std::vector<bbox> left[3];
        bbox left_bv[3];
        
        std::vector<bbox> right[3];
        bbox right_bv[3];

        for ( int i = 0; i < 3; i++ )
        {
            left_bv[i].max_ = { -1000, -1000, -1000 };
            left_bv[i].min_ = { 1000, 1000, 1000 };
            right_bv[i].max_ = { -1000, -1000, -1000 };
            right_bv[i].min_ = { 1000, 1000, 1000 };
        }
        for ( auto i = 0; i < bboxList_list[current_parent_node].size(); i++ )
        {
            auto bboxc = get_c( bboxList_list[current_parent_node][i].min_, bboxList_list[current_parent_node][i].max_ );
            for ( int j = 0; j < 3; j++ )
            {
                bool comparison;
                switch ( j )
                {
                case 0:
                    comparison = rootbboxc.x < bboxc.x;
                    break;
                case 1:
                    comparison = rootbboxc.y < bboxc.y;
                    break;
                default:
                    comparison = rootbboxc.z < bboxc.z;
                    break;
                }
                if ( comparison )
                {
                    left[j].push_back( bboxList_list[current_parent_node][i] );

                    left_bv[j].min_.x = min( left_bv[j].min_.x, bboxList_list[current_parent_node][i].min_.x );
                    left_bv[j].min_.y = min( left_bv[j].min_.y, bboxList_list[current_parent_node][i].min_.y );
                    left_bv[j].min_.z = min( left_bv[j].min_.z, bboxList_list[current_parent_node][i].min_.z );

                    left_bv[j].max_.x = max( left_bv[j].max_.x, bboxList_list[current_parent_node][i].max_.x );
                    left_bv[j].max_.y = max( left_bv[j].max_.y, bboxList_list[current_parent_node][i].max_.y );
                    left_bv[j].max_.z = max( left_bv[j].max_.z, bboxList_list[current_parent_node][i].max_.z );
                }
                else
                {
                    right[j].push_back( bboxList_list[current_parent_node][i] );

                    right_bv[j].min_.x = min( right_bv[j].min_.x, bboxList_list[current_parent_node][i].min_.x );
                    right_bv[j].min_.y = min( right_bv[j].min_.y, bboxList_list[current_parent_node][i].min_.y );
                    right_bv[j].min_.z = min( right_bv[j].min_.z, bboxList_list[current_parent_node][i].min_.z );

                    right_bv[j].max_.x = max( right_bv[j].max_.x, bboxList_list[current_parent_node][i].max_.x );
                    right_bv[j].max_.y = max( right_bv[j].max_.y, bboxList_list[current_parent_node][i].max_.y );
                    right_bv[j].max_.z = max( right_bv[j].max_.z, bboxList_list[current_parent_node][i].max_.z );
                }
            }
        }
        bool splitFailed[3];
        for ( size_t i = 0; i < 3; i++ )
        {
            splitFailed[i] = right[i].size() == 0 || left[i].size() == 0;
        }
        auto getLength = [&] ( int id )
        {
            switch ( id )
            {
            case 0:
                return m_BVHTree[current_parent_node].max_aabb.x - m_BVHTree[current_parent_node].min_aabb.x;
            case 1:
                return m_BVHTree[current_parent_node].max_aabb.y - m_BVHTree[current_parent_node].min_aabb.y;
            default:
                return m_BVHTree[current_parent_node].max_aabb.z - m_BVHTree[current_parent_node].min_aabb.z;
            }
        };

        auto getSAH = [&] ( int id )
        {
            float
                l_sX = left_bv[id].max_.x - left_bv[id].min_.x,
                l_sY = left_bv[id].max_.y - left_bv[id].min_.y,
                l_sZ = left_bv[id].max_.z - left_bv[id].min_.z,
                r_sX = right_bv[id].max_.x - right_bv[id].min_.x,
                r_sY = right_bv[id].max_.y - right_bv[id].min_.y,
                r_sZ = right_bv[id].max_.z - right_bv[id].min_.z;

            float sA_l = 2 * ( l_sX * l_sX + l_sY * l_sY + l_sZ * l_sZ );
            float sA_r = 2 * ( r_sX * r_sX + r_sY * r_sY + r_sZ * r_sZ );
            return sA_l * left[id].size() + sA_r * right[id].size();
        };

        std::vector<int> splitOrder;
        
        splitOrder = { 0,1,2 };

        std::sort( splitOrder.begin(), splitOrder.end(), [&] ( const int& a, const int& b ) { return getSAH( a ) > getSAH( b ); } );
        int splitId = 0;
        for ( size_t i = 0; i < 3; i++ )
        {
            if ( !splitFailed[splitOrder[i]] )
                splitId = splitOrder[i];
        }
        if ( splitFailed[splitId] )
        {
            left[splitId].clear();
            right[splitId].clear();
            for ( size_t i = 0; i < bboxList_list[current_parent_node].size()/2; i++ )
            {
                left[splitId].push_back( bboxList_list[current_parent_node][i] );
            }
            for ( size_t i = bboxList_list[current_parent_node].size() / 2; i < bboxList_list[current_parent_node].size(); i++ )
            {
                right[splitId].push_back( bboxList_list[current_parent_node][i] );
            }
        }
        left_leaf.min_aabb.x = min( left_leaf.min_aabb.x, left_bv[splitId].min_.x );
        left_leaf.min_aabb.y = min( left_leaf.min_aabb.y, left_bv[splitId].min_.y );
        left_leaf.min_aabb.z = min( left_leaf.min_aabb.z, left_bv[splitId].min_.z );

        left_leaf.max_aabb.x = max( left_leaf.max_aabb.x, left_bv[splitId].max_.x );
        left_leaf.max_aabb.y = max( left_leaf.max_aabb.y, left_bv[splitId].max_.y );
        left_leaf.max_aabb.z = max( left_leaf.max_aabb.z, left_bv[splitId].max_.z );

        right_leaf.min_aabb.x = min( right_leaf.min_aabb.x, right_bv[splitId].min_.x );
        right_leaf.min_aabb.y = min( right_leaf.min_aabb.y, right_bv[splitId].min_.y );
        right_leaf.min_aabb.z = min( right_leaf.min_aabb.z, right_bv[splitId].min_.z );

        right_leaf.max_aabb.x = max( right_leaf.max_aabb.x, right_bv[splitId].max_.x );
        right_leaf.max_aabb.y = max( right_leaf.max_aabb.y, right_bv[splitId].max_.y );
        right_leaf.max_aabb.z = max( right_leaf.max_aabb.z, right_bv[splitId].max_.z );

        if ( right[splitId].size() > 0 )
        {
            m_BVHTree[current_parent_node].bvh_node_id_r = current_child_node;
            if ( right[splitId].size() <= max_per_leaf_triangles )
            {
                right_leaf.bvh_node_id_l = -((int)m_BVHLeafs.size()+1);
                right_leaf.bvh_node_id_r = -( (int)right[splitId].size() );
                for ( size_t i = 0; i < right[splitId].size(); i++ )
                {
                    m_BVHLeafs.push_back( right[splitId][i].triangle_id );
                }
            }
            m_BVHTree.push_back( right_leaf );
            bboxList_list.push_back( right[splitId] );
            if ( right[splitId].size() > max_per_leaf_triangles )
                stack.push( current_child_node );
            current_child_node++;
        }
        if ( left[splitId].size() > 0 )
        {
            m_BVHTree[current_parent_node].bvh_node_id_l = current_child_node;
            if ( left[splitId].size() <= max_per_leaf_triangles )
            {
                left_leaf.bvh_node_id_l = -( (int)m_BVHLeafs.size()+1 );
                left_leaf.bvh_node_id_r = -( (int)left[splitId].size() );
                for ( size_t i = 0; i < left[splitId].size(); i++ )
                {
                    m_BVHLeafs.push_back( left[splitId][i].triangle_id );
                }
            }
            m_BVHTree.push_back( left_leaf );
            bboxList_list.push_back( left[splitId] );
            if ( left[splitId].size() > max_per_leaf_triangles )
                stack.push( current_child_node );
            current_child_node++;
        }
        if ( m_BVHTree[current_parent_node].bvh_node_id_l == -1 )
        {
            m_BVHTree[current_parent_node].bvh_node_id_l = m_BVHTree[current_parent_node].bvh_node_id_r;
        }
        if ( m_BVHTree[current_parent_node].bvh_node_id_r == -1 )
        {
            m_BVHTree[current_parent_node].bvh_node_id_r = m_BVHTree[current_parent_node].bvh_node_id_l;
        }
    }
    // ------------------------------------------------------------------------------------------------

    m_pTriangleList = new RHEngine::D3D11StructuredBuffer<Triangle_v2>( device, m_aTris.size() );
    m_pVertexList = new RHEngine::D3D11StructuredBuffer<vertex4>( device, m_aVerts.size() );
    m_pBVHTree = new RHEngine::D3D11StructuredBuffer<BVHTreeNode>( device, m_BVHTree.size() );
    m_pLeafList = new RHEngine::D3D11StructuredBuffer<unsigned int>( device, m_BVHLeafs.size() );
    std::vector<unsigned int> indexBuffer;

    for ( int i = 0; i < m_aVerts.size(); i++ )
    {
        vertex_deferred vert{};
        vert.vert = m_aVerts[i];
        vert.norm = { 0,0,0,1 };
        m_aVertexDeferred.push_back( vert );
    }

    for ( size_t i = 0; i < m_aTris.size(); i++ )
    {
        indexBuffer.push_back( m_aTris[i].a );
        indexBuffer.push_back( m_aTris[i].b );
        indexBuffer.push_back( m_aTris[i].c );
        vertex4 vA = m_aVertexDeferred[m_aTris[i].a].vert;
        vertex4 vB = m_aVertexDeferred[m_aTris[i].b].vert;
        vertex4 vC = m_aVertexDeferred[m_aTris[i].c].vert;
        vertex tangent = {
            vB.x - vA.x,
            vB.y - vA.y,
            vB.z - vA.z
        };
        // bitangent vector
        vertex bitangent = {
            vA.x - vC.x,
            vA.y - vC.y,
            vA.z - vC.z
        };
        // normal vector as cross product of (tangent X bitangent)
        vertex4 faceNormal = {
            ( tangent.y*bitangent.z - tangent.z*bitangent.y ),
            ( tangent.z*bitangent.x - tangent.x*bitangent.z ),
            ( tangent.x*bitangent.y - tangent.y*bitangent.x ), 0
        };
        m_aVertexDeferred[m_aTris[i].a].norm = 
        { 
            m_aVertexDeferred[m_aTris[i].a].norm.x + faceNormal.x ,
            m_aVertexDeferred[m_aTris[i].a].norm.y + faceNormal.y,
            m_aVertexDeferred[m_aTris[i].a].norm.z + faceNormal.z
        };
        m_aVertexDeferred[m_aTris[i].b].norm =
        {
            m_aVertexDeferred[m_aTris[i].b].norm.x + faceNormal.x ,
            m_aVertexDeferred[m_aTris[i].b].norm.y + faceNormal.y,
            m_aVertexDeferred[m_aTris[i].b].norm.z + faceNormal.z
        };
        m_aVertexDeferred[m_aTris[i].c].norm =
        {
            m_aVertexDeferred[m_aTris[i].c].norm.x + faceNormal.x ,
            m_aVertexDeferred[m_aTris[i].c].norm.y + faceNormal.y,
            m_aVertexDeferred[m_aTris[i].c].norm.z + faceNormal.z
        };
    }

    for ( int i = 0; i < m_aVerts.size(); i++ )
    {
        vertex4 vN = m_aVertexDeferred[i].norm;
        float vN_len = sqrtf( vN.x*vN.x + vN.y*vN.y + vN.z*vN.z );
        m_aVertexDeferred[i].norm = { vN.x / vN_len, vN.y / vN_len, vN.z / vN_len, 1 };
    }

    D3D11_SUBRESOURCE_DATA ib_res_data{};
    ib_res_data.pSysMem = indexBuffer.data();
    m_pIndexBuffer = new RHEngine::D3D11IndexBuffer( device, indexBuffer.size(), &ib_res_data );

    D3D11_SUBRESOURCE_DATA vb_res_data{};
    vb_res_data.pSysMem = m_aVerts.data();
    m_pVertexBuffer = new RHEngine::D3D11VertexBuffer( device, m_aVerts.size() * sizeof( vertex4 ), &vb_res_data );

    D3D11_SUBRESOURCE_DATA vbn_res_data{};
    vbn_res_data.pSysMem = m_aVertexDeferred.data();
    m_pVertexBufferNormals = new RHEngine::D3D11VertexBuffer( device, m_aVertexDeferred.size() * sizeof( vertex_deferred ), &vbn_res_data );
    std::vector<D3D11_INPUT_ELEMENT_DESC> layout =
    {
        { "POSITION",   0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, 0,   D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",     0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, 16,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        /*{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, 20,	D3D11_INPUT_PER_VERTEX_DATA, 0 }*/
    };
    RHEngine::RHInputLayoutInfo layout_info;
    layout_info.inputElements = 
    {
        {RHEngine::RHInputElementType::Vec4fp32,"POSITION"},
        {RHEngine::RHInputElementType::Vec4fp32,"NORMAL"},
    };
    layout_info.shaderPtr = m_pDeferredVS;

    allocator->AllocateInputLayout( layout_info, m_pVertexDecl );

    RHEngine::IRenderingContext* context = ( RHEngine::IRenderingContext*)RHEngine::g_pRHRenderer->GetCurrentContext();
    m_pConstantBuffer->data.raytrace_depth = m_aTris.size();
    m_pConstantBuffer->data.sphere_count = 1;
    m_pConstantBuffer->data.screen_width = 1280;
    m_pConstantBuffer->data.screen_height = 720;
   // context->UpdateBuffer( m_pConstantBuffer, )
    m_pConstantBuffer->Update( context );

    m_pVariableBuffer->data.random_a = ( (float)( rand() % 10000 ) ) / 10000.0f;
    m_pVariableBuffer->data.random_b = ( (float)( rand() % 10000 ) ) / 10000.0f;
    m_pVariableBuffer->data.random_a_ui = 0;
    m_pVariableBuffer->data.lightPos.x = sinf( M_PI / 3 ) * 100;
    m_pVariableBuffer->data.lightPos.y = cosf( M_PI / 3 ) * 100;
    m_pVariableBuffer->data.lightPos.w = 10;
    return true;
}
bool loaded = false;

void SimpleRayTracingSample::CustomRender()
{
    RHEngine::IRenderingContext* context = ( RHEngine::IRenderingContext*)RHEngine::g_pRHRenderer->GetCurrentContext();
    

    RwRGBA clearColor = { 128, 128, 255, 255 };

    RHEngine::g_pRWRenderEngine->CameraClear( m_pMainCamera, &clearColor, rwCAMERACLEARIMAGE | rwCAMERACLEARZ );

    m_pVariableBuffer->Update( context );

    std::vector<ID3D11Buffer*> cBuff = { m_pConstantBuffer->GetBuffer(), m_pVariableBuffer->GetBuffer() };

    context->CSSetConstantBuffers( 0, cBuff.size(), cBuff.data() );
    context->VSSetConstantBuffers( 0, cBuff.size(), cBuff.data() );
    context->PSSetConstantBuffers( 0, cBuff.size(), cBuff.data() );

    DrawDeferred();

    if(true)
    {

    ID3D11UnorderedAccessView* uav = m_pRTTextures[2]->GetUnorderedAccessView();

    std::vector <ID3D11ShaderResourceView*> srv = { m_pRTTextures[2]->GetShaderResourceView(), m_pRTTextures[1 - currentTex]->GetShaderResourceView() };

    

    if ( !loaded )
    {
        context->UpdateBuffer( m_pSphereList, (void*)m_aSpheres.data(), -1 );
        context->UpdateBuffer( m_pTriangleList, (void*)m_aTris.data(), -1 );
        context->UpdateBuffer( m_pVertexList, (void*)m_aVerts.data(), -1 );
        context->UpdateBuffer( m_pBVHTree, (void*)m_BVHTree.data(), -1 );
        context->UpdateBuffer( m_pLeafList, (void*)m_BVHLeafs.data(), -1 );
        loaded = true;
    }

    std::vector<ID3D11ShaderResourceView*> strBuffer = {
        m_pSphereList->GetShaderResourceView(),
        m_pTriangleList->GetShaderResourceView(),
        m_pVertexList->GetShaderResourceView(),
        m_pBVHTree->GetShaderResourceView(),
        m_pLeafList->GetShaderResourceView(),
        m_pGBTextures[0]->GetShaderResourceView(),
        m_pGBTextures[1]->GetShaderResourceView()
    };

    //context->
    context->CSSetShaderResources( 0, strBuffer.size(), strBuffer.data() );

    context->CSSetUnorderedAccessViews( 0, 1, &uav, nullptr );

    m_pRTShader->Set( context );
    m_pRTShader->Dispath( context, 1280 / 8, 720 / 8, 1 );

    std::vector<ID3D11ShaderResourceView*> estrbuf = { nullptr, nullptr };
    context->CSSetShaderResources( 5, 2, estrbuf.data() );

    uav = nullptr;
    context->CSSetUnorderedAccessViews( 0, 1, &uav, nullptr );

    context->PSSetShaderResources( 0, 2, srv.data() );
    context->PSSetSamplers( 0, 1, &m_pSamplerState );

    RHEngine::g_pRHRenderer->BindImageBuffers( RHEngine::RHImageBindType::RenderTarget, { { 0, (void*)m_pRTTextures[currentTex]} } );

    m_pAccShader->Set( context );
    RH_RWAPI::RwIm2DRenderPrimitive( RwPrimitiveType::rwPRIMTYPETRILIST, m_vQuad.data(), m_vQuad.size() );

    void **frameBufferInternal = reinterpret_cast<void**>( ( (RwUInt8*)m_pMainCamera->frameBuffer ) + sizeof( RwRaster ) );
    RHEngine::g_pRHRenderer->BindImageBuffers( RHEngine::RHImageBindType::RenderTarget, { { 0, *frameBufferInternal } } );

    srv = { m_pRTTextures[currentTex]->GetShaderResourceView(), m_pRTTextures[currentTex]->GetShaderResourceView() };

    context->PSSetShaderResources( 0, 2, srv.data() );

    m_pAccShader->Set( context );
    RH_RWAPI::RwIm2DRenderPrimitive( RwPrimitiveType::rwPRIMTYPETRILIST, m_vQuad.data(), m_vQuad.size() );
    srv = { nullptr, nullptr };
    context->PSSetShaderResources( 0, 2, srv.data() );

    prevTex = currentTex;
    currentTex = 1 - currentTex;
}
    //m_pRTTexture->GetUnorderedAccessView();
}

void SimpleRayTracingSample::CustomUpdate()
{
    auto current_time = std::chrono::high_resolution_clock::now();
    m_fTime = std::chrono::duration_cast<std::chrono::duration<float>>( current_time - m_tStart ).count();
    m_aSpheres[1].position[0] = sinf( m_fTime );
    m_aSpheres[1].position[1] = cosf( m_fTime * 2 )*0.5f;
    m_aSpheres[1].position[2] = cosf( m_fTime ) + 4;

    m_pVariableBuffer->data.random_a = ( (float)( rand() % 10000 ) ) / 10000.0f;
    m_pVariableBuffer->data.random_b = ( (float)( rand() % 10000 ) ) / 10000.0f;
    m_pVariableBuffer->data.random_a_ui = min(m_pVariableBuffer->data.random_a_ui + 1, max( max_rt_iterations, 2 ) );
    
    if ( GetKeyState( VK_UP ) & 0x8000 )
    {
        m_pVariableBuffer->data.lightPos.z += 0.5f;
    }
    if ( GetKeyState( VK_DOWN ) & 0x8000 )
    {
        m_pVariableBuffer->data.lightPos.z -= 0.5f;
    }
    if ( GetKeyState( VK_LEFT ) & 0x8000 )
    {
        m_pVariableBuffer->data.lightPos.x -= 0.5f;
    }
    if ( GetKeyState( VK_RIGHT ) & 0x8000 )
    {
        m_pVariableBuffer->data.lightPos.x += 0.5f;
    }
    if ( GetKeyState( VK_SHIFT ) & 0x8000 )
    {
        m_pVariableBuffer->data.lightPos.y += 0.5f;
    }
    if ( GetKeyState( VK_CONTROL ) & 0x8000 )
    {
        m_pVariableBuffer->data.lightPos.y -= 0.5f;
    }

    if ( GetKeyState( 0x57 ) & 0x8000 )
    {
        m_pVariableBuffer->data.camPos.z += 0.5f;
    }
    if ( GetKeyState( 0x53 ) & 0x8000 )
    {
        m_pVariableBuffer->data.camPos.z -= 0.5f;
    }
    if ( GetKeyState( 0x41 ) & 0x8000 )
    {
        m_pVariableBuffer->data.camPos.x -= 0.5f;
    }
    if ( GetKeyState( 0x44 ) & 0x8000 )
    {
        m_pVariableBuffer->data.camPos.x += 0.5f;
    }
    if ( GetKeyState( 0x45 ) & 0x8000 )
    {
        m_pVariableBuffer->data.camPos.y += 0.5f;
    }
    if ( GetKeyState( 0x51 ) & 0x8000 )
    {
        m_pVariableBuffer->data.camPos.y -= 0.5f;
    }
    if ( GetKeyState( 0x5A ) & 0x8000 )
    {
        m_pVariableBuffer->data.lightPos.w += 0.1f;
    }
    if ( GetKeyState( 0x58 ) & 0x8000 )
    {
        m_pVariableBuffer->data.lightPos.w -= 0.1f;
    }
    if ( GetKeyState( 0x46 ) & 0x8000 )
    {
        max_rt_iterations = max( max_rt_iterations-1, 1);
    }
    if ( GetKeyState( 0x47 ) & 0x8000 )
    {
        ++max_rt_iterations;
    }
}

void SimpleRayTracingSample::CustomShutdown()
{
    delete m_pRTShader;
    delete m_pAccShader;
    delete m_pRTTextures[1];
    delete m_pRTTextures[0];
    RHTests::TestSample::CustomShutdown();
}

void SimpleRayTracingSample::DrawDeferred()
{
    ID3D11DeviceContext* context = (ID3D11DeviceContext*)RHEngine::g_pRHRenderer->GetCurrentContext();
    float clearColor[] = { 0,0,0,0 };
    RHEngine::g_pRHRenderer->ClearImageBuffer( RHEngine::ImageClearType::Color,
                                               m_pGBTextures[0],
                                               clearColor );
    RHEngine::g_pRHRenderer->ClearImageBuffer( RHEngine::ImageClearType::Color,
                                               m_pGBTextures[1],
                                               clearColor );
    RHEngine::g_pRHRenderer->BindImageBuffers( RHEngine::RHImageBindType::RenderTarget, { { 0, (void*)m_pGBTextures[0] }, { 1, (void*)m_pGBTextures[1] } } );

    context->IASetIndexBuffer( m_pIndexBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0 );
    std::vector<ID3D11Buffer*> vb = { m_pVertexBufferNormals->GetBuffer() };
    std::vector<UINT> strides = { sizeof( vertex_deferred ) };
    std::vector<UINT> offsets = {0};
    context->IASetVertexBuffers( 0, 1, vb.data(), strides.data(), offsets.data() );
    context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    m_pVertexDecl->Set( context );
    m_pDeferredVS->Set( context );
    m_pDeferredPS->Set( context );
    context->DrawIndexed( m_aTris.size() * 3, 0, 0 );
    RHEngine::g_pRHRenderer->BindImageBuffers( RHEngine::RHImageBindType::RenderTarget, { { 0, nullptr }, { 1, nullptr } } );
}
