#pragma once
#include "SettingsHolder.h"
class CD3D1XVertexShader;
class CD3D1XPixelShader;
/*!
    \class CGrassRendererRH
    \brief Draws the game's grass instanced, into the G-buffer.

    The game draws every grass tuft as its own atomic (CGrassRenderer::DrawTriPlants), thousands of draw calls a
    frame. This replaces DrawTriPlants: tufts are placed exactly like the game does (same seeds and random
    sequence), collected per (plant mesh, texture) and drawn with one instanced draw each during the deferred pass,
    so grass gets sun light, shadows, AO and fog like the rest of the scene.
*/
class CGrassRendererRH
{
public:
    static void Patch();
    /*!
        Collects the frame's grass through CPlantMgr::Render and draws it. Call while the G-buffer is bound.
    */
    static void RenderDeferred();
    static void Shutdown();

private:
    // CPlantMgr limits: draw distance, plant triangle generation radius and pool sizes
    static bool __cdecl InitPlantLists();
    static void UpdatePlantLimits();

    struct PPTriPlant;
    struct Instance
    {
        float pos[3];
        float scaleXY;
        float scaleZ;
        float bend;
        float fade;
        RwRGBA color;
    };
    struct Mesh
    {
        ID3D11Buffer *vertexBuffer = nullptr;
        ID3D11Buffer *indexBuffer  = nullptr;
        UINT indexCount            = 0;
    };
    struct Batch
    {
        RpGeometry *geometry;
        RwTexture *texture;
        UINT first;
        UINT count;
    };

    static int __cdecl DrawTriPlants( PPTriPlant *triPlants, int numTriPlants, RpAtomic **plantModels );
    static bool Init();
    static Mesh *GetMesh( RpGeometry *geometry );

    static bool m_bInitialized;
    static bool m_bCollecting;
    static CD3D1XVertexShader *m_pVS;
    static CD3D1XPixelShader *m_pPS;
    static ID3D11InputLayout *m_pInputLayout;
    static ID3D11Buffer *m_pInstanceBuffer;
    static UINT m_nInstanceCapacity;
    static std::vector<Instance> m_aInstances;
    static std::vector<std::pair<RpGeometry *, RwTexture *>> m_aInstanceKeys;
    static std::vector<std::pair<RpGeometry *, Mesh>> m_aMeshes;
};

class GrassSettingsBlock : public SettingsBlock
{
public:
    GrassSettingsBlock()
    {
        m_sName = "Grass";
        // grass is drawn up to here, then faded out over 20 m (game: 60)
        m_aFields["DrawDistance"] = new FloatSField( "DrawDistance", false, false, false, m_sName, 200.0f, 20.0f, 800.0f, 10.0f );
        // plant triangles (grass and procedural objects) exist within this radius (game: 100)
        m_aFields["GenerationRadius"] = new FloatSField( "GenerationRadius", false, false, false, m_sName, 230.0f, 50.0f, 900.0f, 10.0f );
        // pool sizes, applied when the game starts (game: 256 triangles, 40 collision entities)
        m_aFields["TrianglePool"] = new UIntSField( "TrianglePool", true, false, false, m_sName, 8192, 256, 65536, 256 );
        m_aFields["EntityCache"] = new UIntSField( "EntityCache", true, false, false, m_sName, 256, 40, 4096, 8 );
        Reset();
    }
};
extern GrassSettingsBlock gGrassSettings;
