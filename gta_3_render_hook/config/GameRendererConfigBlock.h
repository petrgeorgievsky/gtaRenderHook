//
// Created by peter on 30.11.2020.
//

#pragma once
#include <ConfigUtils/ConfigBlock.h>

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