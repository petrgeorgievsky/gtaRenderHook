//
// Created by peter on 22.05.2020.
//
#pragma once
#include "PtrList.h"
#include "World.h"
#include <ConfigUtils/ConfigBlock.h>
#include <array>

class Renderer
{
  public:
    static void     ScanWorld();
    static void     PreRender();
    static void     Render();
    static void     ScanSectorList( const WorldSector &sector );
    static int32_t  SetupEntityVisibility( Entity *ent );
    static int32_t  SetupBigBuildingVisibility( Entity *ent );
    static uint32_t mLightCount;

  private:
    static int32_t &                  mNoOfVisibleEntities;
    static Entity **                  mVisibleEntityPtrs;
    static std::array<Entity *, 8000> mVisibleEntities;
    static int32_t                    mNoOfInVisibleEntities;
    static Entity **                  mInVisibleEntityPtrs;
    static RwV3d &                    mCameraPosition;
};
class GameRendererConfigBlock : public rh::engine::ConfigBlock
{
  public:
    static GameRendererConfigBlock It;

  public:
    GameRendererConfigBlock() noexcept;

    void Reset();

    void        Deserialize( rh::engine::Serializable *serializable ) override;
    void        Serialize( rh::engine::Serializable *serializable ) override;
    std::string Name() override { return "GameRenderer"; }

  public:
    /// Properties
    float    SectorScanDistance = 400.0f;
    float    LodMultiplier      = 3.0f;
    uint32_t ModelStreamLimit   = 100;
};