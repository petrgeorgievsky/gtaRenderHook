#pragma once
#include "Engine/Common/IDescriptorSet.h"
#include <unordered_map>
#include <utility>
#include <vector>

// d3d11 struct forwards:
struct ID3D11DeviceChild;
struct ID3D11DeviceContext;

namespace rh::engine
{
class IDescriptorSetLayout;
class D3D11DescriptorSetLayout;
struct D3D11DescriptorSetCreateParams
{
    // Params...
    IDescriptorSetLayout *mLayout;
};

enum class D3DDescriptorType : int32_t
{
    ConstantBuffer = 0,
    ShaderResource = 1,
    Sampler        = 2,
};
struct D3D11ShaderResourceBinding
{
    DescriptorType      mDescriptorType;
    D3DDescriptorType   mResourceType;
    uint32_t            mStartId;
    std::vector<void *> mResources;
    uint32_t            mShaderStages;
};

class D3D11DescriptorSet : public IDescriptorSet
{
  public:
    D3D11DescriptorSet( const D3D11DescriptorSetCreateParams &desc );
    ~D3D11DescriptorSet() override;
    DescriptorType GetType( uint32_t binding_id ) override;
    void           BindToContext( ID3D11DeviceContext *context );
    void UpdateDescriptorBinding( uint32_t id, ID3D11DeviceChild *descriptor );

  private:
    std::vector<D3D11ShaderResourceBinding>      mBindings;
    std::unordered_map<int, std::pair<int, int>> mBindingMap;
    D3D11DescriptorSetLayout *                   mLayout;
};

} // namespace rh::engine
