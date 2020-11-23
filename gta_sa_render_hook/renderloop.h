#pragma once
class RenderLoop
{
public:
    static void Render();
    static void Init();

private:
    static void RenderOpaque();
    static void RenderTransparent();
    static void RenderPostProcessing();
};
