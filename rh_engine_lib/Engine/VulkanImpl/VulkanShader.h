#pragma once
#include "Engine/Common/IShader.h"
#include <common.h>

namespace rh::engine
{

// Translates HLSL to SPIR-V in separate process, saves resulting shader to dest
// path
bool TranslateHLSL_to_SPIRV( const std::string &path,
                             const std::string &dest_path,
                             const std::string &shader_type,
                             const std::string &entry_point );

struct VulkanShaderDesc
{
    // Dependencies...
    vk::Device mDevice;
    // Params
    ShaderDesc mDesc;
};

class VulkanShader : public IShader
{
  public:
    VulkanShader( const VulkanShaderDesc &desc );
    ~VulkanShader() override;

    operator vk::ShaderModule() { return mShaderImpl; }

  private:
    vk::ShaderModule mShaderImpl;
    vk::Device       mDevice;
};

} // namespace rh::engine
