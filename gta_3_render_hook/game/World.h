//
// Created by peter on 23.05.2020.
//

#pragma once

#include "Entity.h"
#include "PtrList.h"
#include <cstdint>

struct WorldSector
{
    PtrList<Entity> mEntityList[10];
};

static_assert( sizeof( WorldSector ) == 0x28,
               "WorldSector is binary incompatible with game struct" );
class World
{
  public:
    static constexpr auto SECTOR_SIZE_X = ( 40.0f );
    static constexpr auto SECTOR_SIZE_Y = ( 40.0f );
    static constexpr auto WORLD_MIN_X   = ( -2000.0f );
    static constexpr auto WORLD_MIN_Y   = ( -2000.0f );

    static void  AdvanceScanCode();
    static void  ClearScanCodes();
    static float GetSectorX( float f );
    static float GetSectorY( float f );

  private:
    static uint16_t &       mCurrentScanCode;
    static WorldSector *    mSectors;
    static PtrList<Entity> *mBigBuildings;
    friend class Renderer;
};
