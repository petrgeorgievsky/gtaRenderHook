//
// Created by peter on 22.05.2020.
//

#include "Renderer.h"
#include "../call_redirection_util.h"
#include "../config/GameRendererConfigBlock.h"
#include "Clock.h"
#include "ModelInfo.h"
#include "Streaming.h"
#include <cmath>
#include <injection_utils/InjectorHelpers.h>
#include <render_client/render_client.h>
#include <span>

enum Visbility
{
    VIS_INVISIBLE,
    VIS_VISIBLE,
    VIS_OFFSCREEN,
    VIS_STREAMME
};

int32_t &Renderer::mNoOfVisibleEntities = *reinterpret_cast<int32_t *>(
    GetAddressByGame( 0x940730, 0x9408E8, 0x950A28 ) );
Entity **Renderer::mVisibleEntityPtrs = reinterpret_cast<Entity **>(
    GetAddressByGame( 0x6E9920, 0x6E9920, 0x6F9A60 ) );
std::array<Entity *, 8000> Renderer::mVisibleEntities{};
RwV3d &Renderer::mCameraPosition = *reinterpret_cast<RwV3d *>(
    GetAddressByGame( 0x8E2C3C, 0x8E2CF0, 0x8F2E30 ) );
uint32_t Renderer::mLightCount = 0;

void RpAtomicSetGeometry( RpAtomic *atomic, RpGeometry *geometry, int flags )
{
    InMemoryFuncCall<void>( GetAddressByGame( 0x59EFA0, 0x59F260, 0x59F1B0 ),
                            atomic, geometry, flags );
}

void Renderer::ScanWorld()
{
    World::AdvanceScanCode();

    auto sector_x = static_cast<int>( World::GetSectorX( mCameraPosition.x ) );
    auto sector_y = static_cast<int>( World::GetSectorY( mCameraPosition.y ) );

    float view_distance = GameRendererConfigBlock::It.SectorScanDistance;
    int   sector_scan_x = ceil( view_distance / World::SECTOR_SIZE_X );
    int   sector_scan_y = ceil( view_distance / World::SECTOR_SIZE_Y );

    auto min_sector_y = ( std::max )( 0, sector_y - sector_scan_y );
    auto max_sector_y = ( std::min )( 100, sector_y + sector_scan_y );
    auto min_sector_x = ( std::max )( 0, sector_x - sector_scan_x );
    auto max_sector_x = ( std::min )( 100, sector_x + sector_scan_x );

    for ( int y = min_sector_y; y < max_sector_y; y++ )
        for ( int x = min_sector_x; x < max_sector_x; x++ )
            ScanSectorList( World::mSectors[( y * 100 + x )] );
    for ( int id = 0; id < 4; id++ )
    {
        auto &ptr_list = World::mBigBuildings[id];
        for ( auto obj : ptr_list )
        {
            /*if ( obj->mScanCode == World::mCurrentScanCode )
                continue; // already seen
            obj->mScanCode = World::mCurrentScanCode;*/

            switch ( SetupBigBuildingVisibility( obj ) )
            {
            case VIS_VISIBLE:
                if ( mNoOfVisibleEntities < mVisibleEntities.size() )
                    mVisibleEntities[mNoOfVisibleEntities++] = obj;
                break;
            case VIS_STREAMME:
                if ( Streaming::mNumModelsRequested <
                     GameRendererConfigBlock::It.ModelStreamLimit )
                    Streaming::RequestModel( obj->mModelIndex, 0 );
                break;
            case VIS_INVISIBLE: continue;
            }
        }
    }
}

inline RwV3d operator-( RwV3d a, RwV3d b )
{
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

inline float length( RwV3d a )
{
    return std::sqrt( a.x * a.x + a.y * a.y + a.z * a.z );
}

void Renderer::ScanSectorList( const WorldSector &sector )
{
    for ( const auto &obj_list : sector.mEntityList )
    {
        for ( auto obj : obj_list )
        {
            if ( obj->mScanCode == World::mCurrentScanCode )
                continue; // already seen
            obj->mScanCode = World::mCurrentScanCode;

            switch ( SetupEntityVisibility( obj ) )
            {
            case VIS_VISIBLE:
                if ( mNoOfVisibleEntities < mVisibleEntities.size() )
                    mVisibleEntities[mNoOfVisibleEntities++] = obj;
                break;
            case VIS_STREAMME:
                if ( Streaming::mNumModelsRequested <
                     GameRendererConfigBlock::It.ModelStreamLimit )
                    Streaming::RequestModel( obj->mModelIndex, 0 );
                break;
            }
        }
    }
}

int32_t Renderer::SetupEntityVisibility( Entity *ent )
{
    auto *base_mi = ModelInfo::GetModelInfo( ent->mModelIndex );
    if ( base_mi->mType != MITYPE_SIMPLE && base_mi->mType != MITYPE_TIME )
        return ent->bIsVisible ? VIS_VISIBLE : VIS_INVISIBLE;
    auto *         mi = static_cast<SimpleModelInfo *>( base_mi );
    TimeModelInfo *ti;
    int32_t        other;
    float          dist;

    // bool request = true;

    if ( mi->mType == MITYPE_TIME )
    {
        ti    = static_cast<TimeModelInfo *>( mi );
        other = ti->mOtherTimeModelID;
        if ( Clock::GetIsTimeInRange( ti->mTimeOn, ti->mTimeOff ) )
        {
            // don't fade in, or between time objects
            ti->mAlpha = 255;
        }
        else
        {
            // Hide if possible
            return VIS_INVISIBLE;
        }
    }
    /*   else
       {
           if ( mi->m_type != MITYPE_SIMPLE )
           {
               if ( FindPlayerVehicle() == ent &&
                    TheCamera.Cams[TheCamera.ActiveCam].Mode ==
                        CCam::MODE_1STPERSON )
               {
                   // Player's vehicle in first person mode
                   if ( TheCamera.Cams[TheCamera.ActiveCam].DirectionWasLooking
       == LOOKING_FORWARD || ent->GetModelIndex() == MI_RHINO ||
                        ent->GetModelIndex() == MI_COACH ||
                        TheCamera.m_bInATunnelAndABigVehicle )
                   {
                       ent->bNoBrightHeadLights = true;
                   }
                   else
                   {
                       m_pFirstPersonVehicle    = (CVehicle *)ent;
                       ent->bNoBrightHeadLights = false;
                   }
                   return VIS_OFFSCREEN;
               }
               else
               {
                   // All sorts of Clumps
                   if ( ent->m_rwObject == nil || !ent->bIsVisible )
                       return VIS_INVISIBLE;
                   if ( !ent->GetIsOnScreen() )
                       return VIS_OFFSCREEN;
                   if ( ent->bDrawLast )
                   {
                       dist = ( ent->GetPosition() - ms_vecCameraPosition )
                                  .Magnitude();
                       CVisibilityPlugins::InsertEntityIntoSortedList( ent, dist
       ); ent->bDistanceFade = false; return VIS_INVISIBLE;
                   }
                   else
                       return VIS_VISIBLE;
               }
               return VIS_INVISIBLE;
           }
           if ( ent->m_type == ENTITY_TYPE_OBJECT &&
                ( (CObject *)ent )->ObjectCreatedBy == TEMP_OBJECT )
           {
               if ( ent->mRwObject == nullptr || !ent->bIsVisible )
                   return VIS_INVISIBLE;
               return ent->GetIsOnScreen() ? VIS_VISIBLE : VIS_OFFSCREEN;
           }
       }
   */
    // Simple ModelInfo

    dist = length( ent->mMatrix.m_matrix.pos - mCameraPosition );

    // This can only happen with multi-atomic models (e.g. railtracks)
    // but why do we bump up the distance? can only be fading...
    /*if ( LOD_DISTANCE + STREAM_DISTANCE < dist &&
         dist < mi->GetLargestLodDistance() )
        dist = mi->GetLargestLodDistance();*/

    // if ( ent->mType == ENTITY_TYPE_OBJECT && ent->bRenderDamaged )
    //    mi->mIsDamaged = true;

    RpAtomic *a = mi->GetAtomicFromDistance( dist );
    if ( a )
    {
        mi->mIsDamaged = false;
        if ( ent->mRwObject == nullptr )
            ent->CreateRwObject();
        assert( ent->mRwObject );
        auto *rwobj = (RpAtomic *)ent->mRwObject;
        // Make sure our atomic uses the right geometry and not
        // that of an atomic for another draw distance.
        if ( a->geometry != rwobj->geometry )
            RpAtomicSetGeometry( rwobj, a->geometry,
                                 /*rpATOMICSAMEBOUNDINGSPHERE*/ 0x1 );
        mi->IncreaseAlpha();
        //
        if ( ent->mRwObject == nullptr || !ent->bIsVisible )
            return VIS_INVISIBLE;

        /*if ( !ent->GetIsOnScreen() )
        {
            mi->m_alpha = 255;
            return VIS_OFFSCREEN;
        }*/

        if ( mi->mAlpha != 255 )
        {
            // CVisibilityPlugins::InsertEntityIntoSortedList( ent, dist );
            ent->bDistanceFade = true;
            // return VIS_INVISIBLE;
        }

        /*if ( mi->m_drawLast || ent->bDrawLast )
        {
            CVisibilityPlugins::InsertEntityIntoSortedList( ent, dist );
            ent->bDistanceFade = false;
            return VIS_INVISIBLE;
        }*/
        return VIS_VISIBLE;
    }

    // Object is not loaded, figure out what to do

    if ( mi->mNoFade )
    {
        mi->mIsDamaged = false;
        // request model
        // if ( dist - STREAM_DISTANCE < mi->GetLargestLodDistance() && request
        // )
        return VIS_STREAMME;
        // return VIS_INVISIBLE;
    }

    // We might be fading

    a              = mi->GetAtomicFromDistance( dist /*- FADE_DISTANCE*/ );
    mi->mIsDamaged = false;
    if ( a == nullptr )
    {
        // request model
        // if ( dist - FADE_DISTANCE - STREAM_DISTANCE <
        //           mi->GetLargestLodDistance() &&
        //      request )
        return VIS_STREAMME;
        // return VIS_INVISIBLE;
    }

    if ( ent->mRwObject == nullptr )
        ent->CreateRwObject();
    assert( ent->mRwObject );
    // RpAtomic *rwobj = (RpAtomic *)ent->mRwObject;
    /*if ( RpAtomicGetGeometry( a ) != RpAtomicGetGeometry( rwobj ) )
        RpAtomicSetGeometry(
            rwobj, RpAtomicGetGeometry( a ),
            rpATOMICSAMEBOUNDINGSPHERE ); */
    mi->IncreaseAlpha();
    if ( ent->mRwObject == nullptr || !ent->bIsVisible )
        return VIS_INVISIBLE;
    return VIS_OFFSCREEN;

    /*
        if ( !ent->GetIsOnScreen() )
        {
            mi->m_alpha = 255;
            return VIS_OFFSCREEN;
        }
        else
        {
            CVisibilityPlugins::InsertEntityIntoSortedList( ent, dist );
            ent->bDistanceFade = true;
            return VIS_OFFSCREEN; // Why this?
        }*/
}

void Renderer::PreRender()
{
    Renderer::mLightCount = 0;
    for ( uint32_t id = 0; id < mNoOfVisibleEntities; id++ )
        mVisibleEntities[id]->PreRender();
}

void RenderWater()
{
    InMemoryFuncCall<void>( GetAddressByGame( 0x5554E0, 0x555610, 0x5555C0 ) );
}

void Renderer::Render()
{
    // Setup timecycle(move out of here)
    auto &sky_state = rh::rw::engine::gRenderClient->RenderState.SkyState;
    int & m_nCurrentSkyBottomBlue =
        *(int *)GetAddressByGame( 0x8F2BD0, 0x8F6414, 0x906554 );
    int &m_nCurrentSkyBottomGreen =
        *(int *)GetAddressByGame( 0x8F2BD0, 0x8F2C84, 0x902DC4 );
    int &m_nCurrentSkyBottomRed =
        *(int *)GetAddressByGame( 0x9414D0, 0x941688, 0x9517C8 );
    int &m_nCurrentSkyTopBlue =
        *(int *)GetAddressByGame( 0x8F29B8, 0x8F2A6C, 0x902BAC );
    int &m_nCurrentSkyTopGreen =
        *(int *)GetAddressByGame( 0x943074, 0x94322C, 0x95336C );
    int &m_nCurrentSkyTopRed =
        *(int *)GetAddressByGame( 0x9403C0, 0x940578, 0x9506B8 );

    float &m_fCurrentAmbientRed =
        *(float *)GetAddressByGame( 0x8F29B4, 0x8F2A68, 0x902BA8 );
    float &m_fCurrentAmbientGreen =
        *(float *)GetAddressByGame( 0x94144C, 0x941604, 0x951744 );
    float &m_fCurrentAmbientBlue =
        *(float *)GetAddressByGame( 0x942FC0, 0x943178, 0x9532B8 );

    int &current_tc_value =
        *(int *)GetAddressByGame( 0x94057C, 0x940734, 0x950874 );
    auto *vec_to_sun_arr =
        (RwV3d *)GetAddressByGame( 0x665548, 0x665548, 0x675688 );

    sky_state.mSkyTopColor[0]    = float( m_nCurrentSkyTopRed ) / 255.0f;
    sky_state.mSkyTopColor[1]    = float( m_nCurrentSkyTopGreen ) / 255.0f;
    sky_state.mSkyTopColor[2]    = float( m_nCurrentSkyTopBlue ) / 255.0f;
    sky_state.mSkyTopColor[3]    = 1.0f;
    sky_state.mSkyBottomColor[0] = float( m_nCurrentSkyBottomRed ) / 255.0f;
    sky_state.mSkyBottomColor[1] = float( m_nCurrentSkyBottomGreen ) / 255.0f;
    sky_state.mSkyBottomColor[2] = float( m_nCurrentSkyBottomBlue ) / 255.0f;
    sky_state.mSkyBottomColor[3] = 1.0f;
    sky_state.mAmbientColor[0]   = m_fCurrentAmbientRed;
    sky_state.mAmbientColor[1]   = m_fCurrentAmbientGreen;
    sky_state.mAmbientColor[2]   = m_fCurrentAmbientBlue;
    sky_state.mAmbientColor[3]   = 1.0f;
    //

    sky_state.mSunDir[0] = vec_to_sun_arr[current_tc_value].x;
    sky_state.mSunDir[1] = vec_to_sun_arr[current_tc_value].y;
    sky_state.mSunDir[2] = vec_to_sun_arr[current_tc_value].z;
    sky_state.mSunDir[3] = 1.0f;

    for ( uint32_t id = 0; id < mNoOfVisibleEntities; id++ )
        mVisibleEntities[id]->Render();
    RenderWater();
}

int32_t Renderer::SetupBigBuildingVisibility( Entity *ent )
{
    auto *base_mi = ModelInfo::GetModelInfo( ent->mModelIndex );
    if ( base_mi->mType != MITYPE_SIMPLE )
        return ent->bIsVisible ? VIS_VISIBLE : VIS_INVISIBLE;
    auto *mi   = static_cast<SimpleModelInfo *>( base_mi );
    auto  dist = length( ent->mMatrix.m_matrix.pos - mCameraPosition );
    SimpleModelInfo *nonLOD = mi->GetRelatedModel();
    // Find out whether to draw below near distance.
    // This is only the case if there is a non-LOD which is either not
    // loaded or not completely faded in yet.
    if ( dist < mi->GetNearDistance() &&
         dist < 300.0f * GameRendererConfigBlock::It.LodMultiplier )
    {
        // No non-LOD or non-LOD is completely visible.
        if ( nonLOD == nullptr || nonLOD->mAtomics[0] && nonLOD->mAlpha == 255 )
            return VIS_INVISIBLE;
    }
    RpAtomic *a = mi->GetAtomicFromDistance( dist );
    if ( a )
    {
        if ( ent->mRwObject == nullptr )
            ent->CreateRwObject();
        assert( ent->mRwObject );
        // RpAtomic *rwobj = (RpAtomic *)ent->mRwObject;
        if ( !ent->bIsVisible )
            return VIS_INVISIBLE;
        return VIS_VISIBLE;
    }
    return VIS_INVISIBLE;
}

void Renderer::Patch()
{
    RedirectJump( GetAddressByGame( 0x4A8970, 0x4A8A60, 0x4A89F0 ),
                  reinterpret_cast<void *>( Renderer::ScanWorld ) );
    RedirectJump( GetAddressByGame( 0x4A7840, 0x4A7930, 0x4A78C0 ),
                  reinterpret_cast<void *>( Renderer::PreRender ) );
    RedirectJump( GetAddressByGame( 0x48E030, 0x48E0F0, 0x48E080 ),
                  reinterpret_cast<void *>( Renderer::Render ) );
}
