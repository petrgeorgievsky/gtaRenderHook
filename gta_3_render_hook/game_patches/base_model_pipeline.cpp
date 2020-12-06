//
// Created by peter on 26.10.2020.
//

#include "base_model_pipeline.h"
#include "../call_redirection_util.h"
#include "../gta3_geometry_proxy.h"
#include <MemoryInjectionUtils/InjectorHelpers.h>
#include <ranges>
#include <render_loop.h>
#include <rw_engine/rh_backend/raster_backend.h>
#include <rw_engine/rw_frame/rw_frame.h>
#include <rw_engine/rw_macro_constexpr.h>
#include <rw_engine/rw_rh_skin_pipeline.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

using namespace rh::rw::engine;
static RpGeometryRw35 geometry_interface_35{};
struct RpSkinGlobals
{
    int       mPluginOffset;
    int       mAtomicSkinOffset;
    int       mGeometrySkinOffset;
    RwMatrix *mAlignedMatrixCache;
    int       mMatrixCacheMemory;
    int       mMatrixFreeList;
    int       padd;
    int       mInstanceCount;
    int       mPipeline;
};

auto &gSkinGlobals =
    *(RpSkinGlobals *)GetAddressByGame( 0x663C8C, 0x663C8C, 0 );

struct RxPipelineNodeParam
{
    void *                 dataParam;
    [[maybe_unused]] void *heap;
};
struct RpSkinOld
{
    int              numBones;
    int              skinToBoneMatrices;
    uint32_t *       indicies_;
    RwMatrixWeights *boneWeights_;
    int              maxNumWeightsForVertex;
    int              field_14;
    int              field_18;
};

struct RpHAnimHierarchy35
{
    int32_t flags;             /**< Flags for the hierarchy  */
    int32_t numNodes;          /**< Number of nodes in the hierarchy  */
    void *  pCurrentAnim;      /**< Current animation applied to hierarchy  */
    float   currentTime;       /**< Current animation time  */
    void *  pNextFrame;        /**< Next animation keyframe to be played  */
    void *  pAnimCallBack;     /**< Animation callback function pointer  */
    void *  pAnimCallBackData; /**< Animation callback function user data  */
    float   animCallBackTime;  /**< Trigger time for callback function  */
    void *  pAnimLoopCallBack; /**< Animation loop callback function pointer  */
    void *pAnimLoopCallBackData; /**< Animation loop callback function data  */
    RwMatrix *pMatrixArray;      /**< Pointer to node matrices*/
    void *pMatrixArrayUnaligned; /**< Pointer to memory used for node matrices
                                  * from which the aligned pMatrixArray is
                                  * allocated */
    RpHAnimNodeInfo
        *    pNodeInfo;   /**< Array of node information (push/pop flags etc) */
    RwFrame *parentFrame; /**< Pointer to the Root RwFrame of the hierarchy this
                           * RpHAnimHierarchy represents */
    int32_t maxKeyFrameSize; /**< Maximum size of keyframes usable on this
                              * hierarhcy (set at creation time) */
    int32_t
        currentKeyFrameSize; /**< Size of keyframes in the current animation */
    void *            keyFrameToMatrixCB;    /**< Internal use */
    void *            keyFrameBlendCB;       /**< Internal use */
    void *            keyFrameInterpolateCB; /**< Internal use */
    void *            keyFrameAddCB;         /**< Internal use */
    RpHAnimHierarchy *parentHierarchy;       /**< Internal use */
    int32_t           offsetInParent;        /**< Internal use */
    int32_t           rootParentOffset;      /**< Internal use */
};

class AnimHierarcyRw35 final : public IAnimHierarcy
{
  private:
    RpHAnimHierarchy35 *base{};

  public:
    void Init( void *_base ) override
    {
        base = ( static_cast<RpHAnimHierarchy35 *>( _base ) );
    }
    uint32_t         GetFlags() override { return base->flags; }
    uint32_t         GetNumNodes() override { return base->numNodes; }
    RpHAnimNodeInfo *GetNodeInfo() override { return base->pNodeInfo; }
    RwMatrix *GetSkinToBoneMatrices() override { return base->pMatrixArray; }
};

std::vector<MaterialData>
FillMaterialBuffer( const RpGeometryInterface &geometry )
{
    auto convert_material = []( const RpMesh &mesh ) {
        auto    m           = mesh.material;
        int32_t tex_id      = 0xBADF00D;
        int32_t spec_tex_id = 0xBADF00D;
        if ( m == nullptr )
            return MaterialData{ tex_id, RwRGBA{ 255, 0, 0, 255 }, spec_tex_id,
                                 0 };

        auto m_b = GetBackendMaterialExt( m );

        if ( m->texture && m->texture->raster )
        {
            auto raster = GetBackendRasterExt( m->texture->raster );
            tex_id      = raster->mImageId;
        }
        auto spec_tex = m_b->mSpecTex;
        if ( spec_tex && spec_tex->raster )
        {
            auto raster = GetBackendRasterExt( spec_tex->raster );
            spec_tex_id = raster->mImageId;
        }
        return MaterialData{ tex_id, m->color, spec_tex_id, 0 };
    };
    auto                      mesh_list = geometry.GetMeshList();
    std::vector<MaterialData> materials{};
    materials.reserve( mesh_list.size() );

    for ( const auto &mesh : mesh_list )
        materials.push_back( convert_material( mesh ) );
    return materials;
}

static int32_t SkinD3D8AtomicAllInOneNode( void * /*self*/,
                                           const RxPipelineNodeParam *params )
{
    auto *atomic = static_cast<RpAtomic *>( params->dataParam );
    auto *geom   = reinterpret_cast<RpGeometryGTA3 *>( atomic->geometry );
    geometry_interface_35.Init( geom );

    if ( RwRHInstanceSkinAtomic( atomic, &geometry_interface_35 ) !=
         RenderStatus::Instanced )
        return 0;
    //

    auto ltm = RwFrameGetLTM(
        static_cast<RwFrame *>( rwObject::GetParent( atomic ) ) );
    DrawAtomic( atomic, &geometry_interface_35,
                [&ltm, atomic]( ResEnty *res_entry ) {
                    SkinDrawCallInfo info{};
                    info.mSkinId         = reinterpret_cast<uint64_t>( atomic );
                    info.mMeshId         = res_entry->meshData;
                    info.mWorldTransform = DirectX::XMFLOAT4X3{
                        ltm->right.x, ltm->up.x, ltm->at.x, ltm->pos.x,
                        ltm->right.y, ltm->up.y, ltm->at.y, ltm->pos.y,
                        ltm->right.z, ltm->up.z, ltm->at.z, ltm->pos.z,
                    };
                    static AnimHierarcyRw35 anim{};
                    PrepareBoneMatrices( info.mBoneTransform, atomic, anim );
                    EngineClient::gSkinRendererGlobals.RecordDrawCall(
                        info, FillMaterialBuffer( geometry_interface_35 ) );
                } );
    return 1;
}

static int32_t D3D8AtomicAllInOneNode( void * /*self*/,
                                       const RxPipelineNodeParam *params )
{
    auto *atomic = static_cast<RpAtomic *>( params->dataParam );
    auto *geom   = reinterpret_cast<RpGeometryGTA3 *>( atomic->geometry );
    geometry_interface_35.Init( geom );

    if ( RwRHInstanceAtomic( atomic, &geometry_interface_35 ) !=
         RenderStatus::Instanced )
        return 0;

    auto ltm = RwFrameGetLTM(
        static_cast<RwFrame *>( rwObject::GetParent( atomic ) ) );
    DrawAtomic( atomic, &geometry_interface_35,
                [&ltm, atomic]( ResEnty *res_entry ) {
                    DrawCallInfo info{};
                    info.mDrawCallId     = reinterpret_cast<uint64_t>( atomic );
                    info.mMeshId         = res_entry->meshData;
                    info.mWorldTransform = DirectX::XMFLOAT4X3{
                        ltm->right.x, ltm->up.x, ltm->at.x, ltm->pos.x,
                        ltm->right.y, ltm->up.y, ltm->at.y, ltm->pos.y,
                        ltm->right.z, ltm->up.z, ltm->at.z, ltm->pos.z,
                    };
                    EngineClient::gRendererGlobals.RecordDrawCall(
                        info, FillMaterialBuffer( geometry_interface_35 ) );
                } );
    return 1;
}

void BaseModelPipelines::Patch()
{
    DeviceGlobals::SkinFuncs.GetVertexBoneWeights =
        []( RpSkin *skin ) -> const RwMatrixWeights * {
        auto old_skin = (RpSkinOld *)skin;
        return old_skin->boneWeights_;
    };
    DeviceGlobals::SkinFuncs.GetVertexBoneIndices =
        []( RpSkin *skin ) -> const uint32_t * {
        auto old_skin = (RpSkinOld *)skin;
        return old_skin->indicies_;
    };
    SetPointer( GetAddressByGame( 0x61B6A4, 0x61AD6C, 0 ),
                reinterpret_cast<void *>( D3D8AtomicAllInOneNode ) );
    SetPointer(
        GetAddressByGame( 0x61BBAC, 0x61B274, 0 ),
        reinterpret_cast<void *>( SkinD3D8AtomicAllInOneNode ) ); // skin
}
