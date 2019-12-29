#pragma once
#pragma twice
#include <TestUtils\TestSample.h>
#include <Engine\D3D11Impl\ImageBuffers\D3D11Texture2D.h>
#include <Engine\D3D11Impl\Shaders\D3D11ComputeShader.h>
#include <Engine\D3D11Impl\Shaders\D3D11PixelShader.h>
#include <Engine\D3D11Impl\Shaders\D3D11VertexShader.h>
#include <Engine\D3D11Impl\Buffers\D3D11ConstantBuffer.h>
#include <Engine\D3D11Impl\Buffers\D3D11StructuredBuffer.h>
#include <Engine\D3D11Impl\Buffers\D3D11VertexBuffer.h>
#include <Engine\D3D11Impl\Buffers\D3D11IndexBuffer.h>
#include <Engine\D3D11Impl\D3D11VertexDeclaration.h>

struct SceneConstants
{
    float screen_width;
    float screen_height;
    unsigned int sphere_count;
    unsigned int raytrace_depth;
};
struct vertex4
{
    float x, y, z, w;
};
struct SceneVariables
{
    float random_a;
    float random_b;
    unsigned int random_a_ui;
    unsigned int random_b_ui;
    vertex4 camPos;
    vertex4 lightPos;
};

// Sphere object description, for compute raytracing
struct Sphere
{
    float position[3];
    float radius;
    float color[4];
};
struct vertex
{
    float x, y, z;
};

struct vertex_deferred
{
    vertex4 vert;
    vertex4 norm;
};

// Triangle object description, for compute raytracing
struct Triangle
{
    vertex v0, v1, v2;
    float color[3];
};
// Triangle object description, for compute raytracing
struct Triangle_v2
{
    unsigned int a, b, c, d;
};

struct BVHTreeNode
{
    vertex min_aabb;
    int bvh_node_id_r;
    vertex max_aabb;
    int bvh_node_id_l;
};
struct BVHLeaf
{
    unsigned int triangleId[4];
};
/*struct BVHTree
{
    BVHTreeNode root;
    BVHTreeNode node_list[256];
    BVHLeaf leaf_list[512];
};*/

/**
* @brief Simple test sample for RenderHook, write your own code as you wish
*
*/
class SimpleRayTracingSample : public RHTests::TestSample
{
public:
    SimpleRayTracingSample( RHEngine::RHRenderingAPI api, HINSTANCE inst ) : RHTests::TestSample( api, inst ) { }
    bool CustomInitialize() override;
    void CustomUpdate() override;
    void CustomRender() override;
    virtual void CustomShutdown() final;

    void DrawDeferred();
private:
    RHEngine::D3D11Texture2D* m_pRTTextures[3];
    RHEngine::D3D11Texture2D* m_pGBTextures[2];
    unsigned int currentTex=0;
    unsigned int prevTex=0;

    ID3D11SamplerState* m_pSamplerState = nullptr;
    ID3D11DepthStencilState* m_pDepthStencilState = nullptr;

    RHEngine::D3D11ComputeShader* m_pRTShader = nullptr;
    RHEngine::D3D11PixelShader* m_pAccShader = nullptr;
    RHEngine::D3D11VertexShader* m_pDeferredVS = nullptr;
    RHEngine::D3D11PixelShader* m_pDeferredPS = nullptr;

    RHEngine::D3D11ConstantBuffer<SceneConstants>* m_pConstantBuffer = nullptr;
    RHEngine::D3D11ConstantBuffer<SceneVariables>* m_pVariableBuffer = nullptr;

    RHEngine::D3D11StructuredBuffer<Sphere>* m_pSphereList = nullptr;
    RHEngine::D3D11StructuredBuffer<Triangle_v2>* m_pTriangleList = nullptr;
    RHEngine::D3D11StructuredBuffer<vertex4>* m_pVertexList = nullptr;
    RHEngine::D3D11StructuredBuffer<BVHTreeNode>* m_pBVHTree = nullptr;
    RHEngine::D3D11StructuredBuffer<unsigned int>* m_pLeafList = nullptr;
    RHEngine::D3D11IndexBuffer *m_pIndexBuffer = nullptr;
    RHEngine::D3D11VertexBuffer *m_pVertexBuffer = nullptr;
    RHEngine::D3D11VertexBuffer *m_pVertexBufferNormals = nullptr;
    RHEngine::D3D11VertexDeclaration *m_pVertexDecl = nullptr;

    std::vector<RwIm2DVertex> m_vQuad;
    std::vector<Sphere> m_aSpheres;
    std::vector<Triangle_v2> m_aTris;
    std::vector<vertex4> m_aVerts;
    std::vector<vertex_deferred> m_aVertexDeferred;
    std::vector<BVHTreeNode> m_BVHTree;
    std::vector<unsigned int> m_BVHLeafs;
    std::chrono::time_point<std::chrono::steady_clock> m_tStart = std::chrono::high_resolution_clock::now();
    float m_fTime=0;
    unsigned int max_rt_iterations=1;
};

