#pragma once
#include "Common/ISimple2DRenderer.h"
#include "Definitions.h"
#include "IRenderer.h"

namespace rh::engine {
class RendererFactory
{
public:
    static std::unique_ptr<IRenderer> CreateRenderer(RenderingAPI api,
                                                     HWND window,
                                                     HINSTANCE inst);
    static std::unique_ptr<ISimple2DRenderer> CreateSimple2DRenderer(RenderingAPI api);
};
} // namespace rh::engine
