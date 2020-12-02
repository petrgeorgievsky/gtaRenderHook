//
// Created by peter on 30.11.2020.
//

#include "GameRendererConfigBlock.h"
#include <ConfigUtils/ConfigurationManager.h>
#include <ConfigUtils/Serializable.h>

GameRendererConfigBlock GameRendererConfigBlock::It{};

GameRendererConfigBlock::GameRendererConfigBlock() noexcept
{
    Reset();
    rh::engine::ConfigurationManager::Instance().AddConfigBlock(
        static_cast<rh::engine::ConfigBlock *>( this ) );
}

void GameRendererConfigBlock::Serialize(
    rh::engine::Serializable *serializable )
{
    assert( serializable != nullptr );

    serializable->Set<float>( "SectorScanDistance", SectorScanDistance );
    serializable->Set<float>( "LodMultiplier", LodMultiplier );
    serializable->Set<uint32_t>( "ModelStreamLimit", ModelStreamLimit );
}

void GameRendererConfigBlock::Deserialize(
    rh::engine::Serializable *serializable )
{
    assert( serializable != nullptr );
    SectorScanDistance = serializable->Get<float>( "SectorScanDistance" );
    LodMultiplier      = serializable->Get<float>( "LodMultiplier" );
    ModelStreamLimit   = serializable->Get<uint32_t>( "ModelStreamLimit" );
}

void GameRendererConfigBlock::Reset()
{
    SectorScanDistance = 400.0f;
    LodMultiplier      = 3.0f;
    ModelStreamLimit   = 100;
}