//
// Created by peter on 23.05.2020.
//

#include "World.h"
#include "../call_redirection_util.h"

uint16_t &World::mCurrentScanCode = *reinterpret_cast<uint16_t *>( 0x95CE1C );
WorldSector *    World::mSectors  = reinterpret_cast<WorldSector *>( 0x665608 );
PtrList<Entity> *World::mBigBuildings =
    reinterpret_cast<PtrList<Entity> *>( 0x6FAB60 );

void World::AdvanceScanCode()
{
    if ( ++mCurrentScanCode != 0 )
        return;

    ClearScanCodes();
    mCurrentScanCode = 1;
}

void World::ClearScanCodes() { InMemoryFuncCall<void, 0x4B2050>(); }

float World::GetSectorX( float f )
{
    return ( ( f - WORLD_MIN_X ) / SECTOR_SIZE_X );
}
float World::GetSectorY( float f )
{
    return ( ( f - WORLD_MIN_Y ) / SECTOR_SIZE_Y );
}
