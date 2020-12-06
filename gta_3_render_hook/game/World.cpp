//
// Created by peter on 23.05.2020.
//

#include "World.h"
#include "../call_redirection_util.h"

uint16_t &World::mCurrentScanCode = *reinterpret_cast<uint16_t *>(
    GetAddressByGame( 0x95CC64, 0x95CE1C, 0x96CF5C ) );
WorldSector *World::mSectors = reinterpret_cast<WorldSector *>(
    GetAddressByGame( 0x665608, 0x665608, 0x675748 ) );
PtrList<Entity> *World::mBigBuildings = reinterpret_cast<PtrList<Entity> *>(
    GetAddressByGame( 0x6FAB60, 0x6FAB60, 0x70ACA0 ) );

void World::AdvanceScanCode()
{
    if ( ++mCurrentScanCode != 0 )
        return;

    ClearScanCodes();
    mCurrentScanCode = 1;
}

void World::ClearScanCodes()
{
    InMemoryFuncCall<void>( GetAddressByGame( 0x4B1F60, 0x4B2050, 0x4B1FE0 ) );
}

float World::GetSectorX( float f )
{
    return ( ( f - WORLD_MIN_X ) / SECTOR_SIZE_X );
}
float World::GetSectorY( float f )
{
    return ( ( f - WORLD_MIN_Y ) / SECTOR_SIZE_Y );
}
