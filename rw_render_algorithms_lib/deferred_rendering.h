#pragma once
namespace rh::engine {
class IGPUResource;
}
class DeferredRendering
{
public:
    DeferredRendering();

private:
    rh::engine::IGPUResource *m_pCompositeCS;
    rh::engine::IGPUResource *m_pCompositePS;
};
