//
// Created by peter on 27.02.2021.
//
#pragma once
#include <map>
#include <render_driver/render_graph/RenderGraphResource.h>

namespace rh::rw::engine
{

class RenderGraphResourcePool
{
  private:
    std::vector<std::unique_ptr<RenderGraphResource>> ResourceMap{};

  public:
    template <typename Resource>
    RenderGraphResourcePool &Create( const RendererBase &renderer )
    {
        if ( Resource::CanCreate( renderer ) )
        {
            Resource::Id = ResourceMap.size();
            ResourceMap.template emplace_back(
                std::make_unique<Resource>( renderer ) );
        }
        return *this;
    }

    template <typename Resource> Resource *Get()
    {
        if ( Resource::Id != Resource::UninitializedId )
            return static_cast<Resource *>( ResourceMap[Resource::Id].get() );
        else
            return nullptr;
    }

    void Update( const FrameState &state )
    {
        for ( auto &resource : ResourceMap )
            resource->Update( state );
    }
};

} // namespace rh::rw::engine