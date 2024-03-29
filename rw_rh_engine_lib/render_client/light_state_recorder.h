//
// Created by peter on 16.02.2021.
//

#pragma once
#include <data_desc/light_system/point_light.h>
#include <vector>

struct RwIm2DVertex;

namespace rh::rw::engine
{

class MemoryWriter;

class LightStateRecorder
{
  public:
    LightStateRecorder() noexcept;
    ~LightStateRecorder() noexcept;

    void     RecordPointLight( PointLight &&light );
    uint64_t Serialize( MemoryWriter &writer );
    void     Flush();

  private:
    std::vector<PointLight> PointLights{};
    uint32_t                PointLightCount = 0;
};

} // namespace rh::rw::engine