#include "D3D11DescriptorSetAllocator.h"
#include "D3D11DescriptorSet.h"
#include <algorithm>
using namespace rh::engine;

D3D11DescriptorSetAllocator::D3D11DescriptorSetAllocator(
    const DescriptorSetAllocatorCreateParams & /*desc*/ )
{
}

D3D11DescriptorSetAllocator::~D3D11DescriptorSetAllocator() {}

std::vector<IDescriptorSet *>
D3D11DescriptorSetAllocator::AllocateDescriptorSets(
    const DescriptorSetsAllocateParams &params )
{
    std::vector<IDescriptorSet *> result;
    result.reserve( params.mLayouts.Size() );

    std::ranges::transform( params.mLayouts.begin(), params.mLayouts.end(),
                            std::back_inserter( result ),
                            []( auto layout_ptr ) {
                                D3D11DescriptorSetCreateParams create_params{};
                                create_params.mLayout = layout_ptr;
                                return new D3D11DescriptorSet( create_params );
                            } );

    return result;
}
