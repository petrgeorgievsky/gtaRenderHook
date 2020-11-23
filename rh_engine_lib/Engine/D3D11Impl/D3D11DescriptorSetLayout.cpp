#include "D3D11DescriptorSetLayout.h"

rh::engine::D3D11DescriptorSetLayout::D3D11DescriptorSetLayout(
    const DescriptorSetLayoutCreateParams &desc )
{
    mDescription.reserve( desc.mBindings.Size() );
    for ( std::size_t i = 0; i < desc.mBindings.Size(); i++ )
        mDescription.push_back( desc.mBindings[i] );
}

rh::engine::D3D11DescriptorSetLayout::~D3D11DescriptorSetLayout() {}
