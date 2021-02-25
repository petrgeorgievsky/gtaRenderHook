//
// Created by peter on 22.05.2020.
//

#include "Renderer.h"
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

int32_t &Renderer::mNoOfVisibleEntities =
    *reinterpret_cast<int32_t *>( 0xA0D1E4 );
Entity **Renderer::mVisibleEntityPtrs = reinterpret_cast<Entity **>( 0x7D54F8 );
std::array<Entity *, 8000> Renderer::mVisibleEntities{};
RwV3d &  Renderer::mCameraPosition = *reinterpret_cast<RwV3d *>( 0x975398 );
uint32_t Renderer::mLightCount     = 0;

void RpAtomicSetGeometry( RpAtomic *atomic, RpGeometry *geometry, int flags )
{
    InMemoryFuncCall<void>( 0x640ED0, atomic, geometry, flags );
}

void Renderer::ScanWorld()
{
    mNoOfVisibleEntities = 0;
    World::AdvanceScanCode();

    auto sector_x = static_cast<int>( World::GetSectorX( mCameraPosition.x ) );
    auto sector_y = static_cast<int>( World::GetSectorY( mCameraPosition.y ) );

    float view_distance =
        500.0f; // GameRendererConfigBlock::It.SectorScanDistance;
    int sector_scan_x = ceil( view_distance / World::SECTOR_SIZE_X );
    int sector_scan_y = ceil( view_distance / World::SECTOR_SIZE_Y );

    auto min_sector_y = ( std::max )( 0, sector_y - sector_scan_y );
    auto max_sector_y = ( std::min )( 80, sector_y + sector_scan_y );
    auto min_sector_x = ( std::max )( 0, sector_x - sector_scan_x );
    auto max_sector_x = ( std::min )( 80, sector_x + sector_scan_x );

    for ( int y = min_sector_y; y < max_sector_y; y++ )
        for ( int x = min_sector_x; x < max_sector_x; x++ )
            ScanSectorList( World::mSectors[( y * 80 + x )] );
    for ( int id = 0; id < 3; id++ )
    {
        auto &ptr_list = World::mBigBuildings[id];
        for ( auto obj : ptr_list )
        {
            if ( obj->mScanCode == World::mCurrentScanCode )
                continue; // already seen
            obj->mScanCode  = World::mCurrentScanCode;
            obj->bOffscreen = false;

            switch ( SetupBigBuildingVisibility( obj ) )
            {
            case VIS_VISIBLE:
                if ( mNoOfVisibleEntities < 1000 )
                    mVisibleEntities[mNoOfVisibleEntities++] = obj;
                break;
            case VIS_STREAMME:
                if ( Streaming::mNumModelsRequested <
                     /*GameRendererConfigBlock::It.ModelStreamLimit*/ 100 )
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
            obj->mScanCode  = World::mCurrentScanCode;
            obj->bOffscreen = false;

            switch ( SetupEntityVisibility( obj ) )
            {
            case VIS_VISIBLE:
                if ( mNoOfVisibleEntities < 1000 )
                    mVisibleEntities[mNoOfVisibleEntities++] = obj;
                break;
            case VIS_STREAMME:
                if ( Streaming::mNumModelsRequested <
                     /*GameRendererConfigBlock::It.ModelStreamLimit*/ 100 )
                    Streaming::RequestModel( obj->mModelIndex, 0 );
                break;
            }
        }
    }
}

int32_t Renderer::SetupEntityVisibility( Entity *ent )
{
    auto *base_mi = ModelInfo::GetModelInfo( ent->mModelIndex );
    if ( ent->bDrawLast )
        return VIS_INVISIBLE;
    if ( base_mi->mType != MITYPE_SIMPLE && base_mi->mType != MITYPE_TIME )
    {
        if ( ent->mRwObject == nullptr )
            return VIS_STREAMME;
        return ent->bIsVisible ? VIS_VISIBLE : VIS_INVISIBLE;
    }
    auto *           mi = static_cast<SimpleModelInfo *>( base_mi );
    TimeModelInfo *  ti;
    SimpleModelInfo *other;
    float            dist;

    // bool request = true;

    if ( mi->mType == MITYPE_TIME )
    {
        ti    = static_cast<TimeModelInfo *>( mi );
        other = ti->mOtherTimeModel;
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
    else
    {
        /*if ( mi->m_type != MITYPE_SIMPLE )
        {
            if ( FindPlayerVehicle() == ent &&
                 TheCamera.Cams[TheCamera.ActiveCam].Mode ==
                     CCam::MODE_1STPERSON )
            {
                // Player's vehicle in first person mode
                if ( TheCamera.Cams[TheCamera.ActiveCam].DirectionWasLooking ==
                         LOOKING_FORWARD ||
                     ent->GetModelIndex() == MI_RHINO ||
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
                    CVisibilityPlugins::InsertEntityIntoSortedList( ent, dist );
                    ent->bDistanceFade = false;
                    return VIS_INVISIBLE;
                }
                else
                    return VIS_VISIBLE;
            }
            return VIS_INVISIBLE;
        }*/
        /*if ( ent->m_type == ENTITY_TYPE_OBJECT &&
             ( (CObject *)ent )->ObjectCreatedBy == TEMP_OBJECT )
        {
            if ( ent->mRwObject == nullptr || !ent->bIsVisible )
                return VIS_INVISIBLE;
            return ent->GetIsOnScreen() ? VIS_VISIBLE : VIS_OFFSCREEN;
        }*/
        if ( ent->bDontStream )
        {
            if ( ent->mRwObject == nullptr || !ent->bIsVisible )
                return VIS_INVISIBLE;
            return VIS_VISIBLE;
        }
    }

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
        // if ( dist - STREAM_DISTANCE < mi->GetLargestLodDistance() &&
        // request
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
    {
        // assert( mVisibleEntities[id] != nullptr );
        mVisibleEntities[id]->PreRender();
    }
}

void RenderWater()
{
    InMemoryFuncCall<void>( 0x5C1710 );
    InMemoryFuncCall<void>( 0x5BFF00 );
}

void Renderer::Render()
{
    // Setup timecycle(move out of here)
    auto &sky_state = rh::rw::engine::gRenderClient->RenderState.SkyState;
    int & m_nCurrentSkyBottomBlue  = *(int *)0x9B6DF4;
    int & m_nCurrentSkyBottomGreen = *(int *)0x97F208;
    int & m_nCurrentSkyBottomRed   = *(int *)0xA0D958;
    int & m_nCurrentSkyTopBlue     = *(int *)0x978D1C;
    int & m_nCurrentSkyTopGreen    = *(int *)0xA0FD70;
    int & m_nCurrentSkyTopRed      = *(int *)0xA0CE98;

    float &m_fCurrentAmbientRed   = *(float *)0x94DBC8;
    float &m_fCurrentAmbientGreen = *(float *)0x9B6AA4;
    float &m_fCurrentAmbientBlue  = *(float *)0x9B6AA0;

    auto *vec_to_sun_arr   = (RwV3d *)0x792C70;
    int & current_tc_value = *(int *)0xA0CFF8;

    sky_state.mSkyTopColor[0]    = float( m_nCurrentSkyTopRed ) / 255.0f;
    sky_state.mSkyTopColor[1]    = float( m_nCurrentSkyTopGreen ) / 255.0f;
    sky_state.mSkyTopColor[2]    = float( m_nCurrentSkyTopBlue ) / 255.0f;
    sky_state.mSkyTopColor[3]    = 1.0f;
    sky_state.mSkyBottomColor[0] = float( m_nCurrentSkyBottomRed ) / 255.0f;
    sky_state.mSkyBottomColor[1] = float( m_nCurrentSkyBottomGreen ) / 255.0f;
    sky_state.mSkyBottomColor[2] = float( m_nCurrentSkyBottomBlue ) / 255.0f;
    sky_state.mSkyBottomColor[3] = 1.0f;
    sky_state.mAmbientColor[0] =
        m_fCurrentAmbientRed; // float( m_nCurrentSkyTopRed ) / 255.0f;
    sky_state.mAmbientColor[1] =
        m_fCurrentAmbientGreen; // float( m_nCurrentSkyTopGreen ) / 255.0f;
    sky_state.mAmbientColor[2] =
        m_fCurrentAmbientBlue; // float( m_nCurrentSkyTopBlue ) / 255.0f;
    sky_state.mAmbientColor[3] = 1.0f;

    sky_state.mSunDir[0] = vec_to_sun_arr[current_tc_value].x;
    sky_state.mSunDir[1] = vec_to_sun_arr[current_tc_value].y;
    sky_state.mSunDir[2] = vec_to_sun_arr[current_tc_value].z;
    sky_state.mSunDir[3] = 1.0f;

    // For no obvious reason rendering non-buildings before buildings "fixes"
    // some bugs, e.g. lack of textures on palms
    // TODO: Investigate why some textures reset rasters after loading
    for ( uint32_t id = 0; id < mNoOfVisibleEntities; id++ )
    {
        assert( mVisibleEntities[id] != nullptr );
        // InMemoryFuncCall<void>( 0x4C9DA0, mVisibleEntities[id] );
        if ( mVisibleEntities[id]->mType != 1 )
            mVisibleEntities[id]->Render();
    }
    for ( uint32_t id = 0; id < mNoOfVisibleEntities; id++ )
    {
        assert( mVisibleEntities[id] != nullptr );
        // InMemoryFuncCall<void>( 0x4C9DA0, mVisibleEntities[id] );
        if ( mVisibleEntities[id]->mType == 1 )
            mVisibleEntities[id]->Render();
    }
    // Render clouds
    InMemoryFuncCall<void>( 0x53FC50 );
    RenderWater();
    // Render rain
    InMemoryFuncCall<void>( 0x57BF40 );
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
         dist < 300.0f * /*GameRendererConfigBlock::It.LodMultiplier*/ 4.0f )
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
    RedirectJump( 0x4C85E0, reinterpret_cast<void *>( Renderer::ScanWorld ) );
    RedirectJump( 0x4CA1F0, reinterpret_cast<void *>( Renderer::PreRender ) );
    RedirectJump( 0x4A6570, reinterpret_cast<void *>( Renderer::Render ) );
}
