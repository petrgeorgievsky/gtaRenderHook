#pragma once
#include <common.h>
#include <unordered_map>

struct ModelData
{
    RpClump* object;
    float drawDist;
    float boundSphereRad;
    std::string dffname;
    std::string txdname;
};

struct ModelInfo
{
    uint32_t modelId;
    DirectX::XMMATRIX transform;
};

class GTAMapLoader 
{

private:
    std::unordered_map<uint32_t, ModelData> m_ideModelList{};
    std::vector<ModelInfo>                  m_iplModelInfoMap{};
};