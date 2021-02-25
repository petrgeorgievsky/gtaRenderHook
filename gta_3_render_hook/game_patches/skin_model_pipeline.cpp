//
// Created by peter on 07.01.2021.
//

#include "skin_model_pipeline.h"
#include "../call_redirection_util.h"
#include "../gta3_geometry_proxy.h"
#include <injection_utils/InjectorHelpers.h>
#include <render_client/render_client.h>
#include <rw_engine/i_anim_hierarcy.h>
#include <rw_engine/rw_frame/rw_frame.h>
#include <rw_engine/rw_macro_constexpr.h>
#include <rw_engine/rw_rh_skin_pipeline.h>
#include <rw_engine/system_funcs/rw_device_system_globals.h>

using namespace rh::rw::engine;

/// Rw 3.5 RpSkin structure
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

/// Rw 3.5 RpHAnimHierarchy structure
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

static int32_t SkinD3D8AtomicAllInOneNode( void * /*self*/,
                                           const RxPipelineNodeParam *params )
{
    static RpGeometryRw35 geometry_interface_35{};
    auto *                atomic = static_cast<RpAtomic *>( params->dataParam );
    auto *geom = reinterpret_cast<RpGeometryGTA3 *>( atomic->geometry );
    geometry_interface_35.Init( geom );

    if ( InstanceSkinAtomic( atomic, &geometry_interface_35 ) !=
         RenderStatus::Instanced )
        return 0;
    //

    auto ltm = RwFrameGetLTM(
        static_cast<RwFrame *>( rwObject::GetParent( atomic ) ) );
    DrawAtomic(
        atomic, &geometry_interface_35, [&ltm, atomic]( ResEnty *res_entry ) {
            auto &renderer = gRenderClient->RenderState.SkinMeshDrawCalls;
            static AnimHierarcyRw35 anim{};

            auto mesh_list = geometry_interface_35.GetMeshList();
            auto materials =
                renderer.AllocateDrawCallMaterials( mesh_list.size() );
            for ( auto i = 0; i < mesh_list.size(); i++ )
                materials[i] = ConvertMaterialData( mesh_list[i].material );

            SkinDrawCallInfo info{};
            info.DrawCallId     = reinterpret_cast<uint64_t>( atomic );
            info.MeshId         = res_entry->meshData;
            info.WorldTransform = DirectX::XMFLOAT4X3{
                ltm->right.x, ltm->up.x, ltm->at.x, ltm->pos.x,
                ltm->right.y, ltm->up.y, ltm->at.y, ltm->pos.y,
                ltm->right.z, ltm->up.z, ltm->at.z, ltm->pos.z,
            };
            PrepareBoneMatrices( info.BoneTransform, atomic, anim );

            renderer.RecordDrawCall( info );
        } );
    return 1;
}

void SkinModelPipeline::Patch()
{
    gRwDeviceGlobals.SkinFuncs.GetVertexBoneWeights =
        []( RpSkin *skin ) -> const RwMatrixWeights * {
        auto old_skin = reinterpret_cast<RpSkinOld *>( skin );
        return old_skin->boneWeights_;
    };
    gRwDeviceGlobals.SkinFuncs.GetVertexBoneIndices =
        []( RpSkin *skin ) -> const uint32_t * {
        auto old_skin = reinterpret_cast<RpSkinOld *>( skin );
        return old_skin->indicies_;
    };
    SetPointer(
        GetAddressByGame( 0x61BBAC, 0x61B274, 0x626F74 ),
        reinterpret_cast<void *>( SkinD3D8AtomicAllInOneNode ) ); // skin
}
