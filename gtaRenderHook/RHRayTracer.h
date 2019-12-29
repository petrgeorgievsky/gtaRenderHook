#pragma once
#include "D3D1XShader.h"
class RHRayTracer
{
public:
    static void Init();
    static void Shutdown();
    static void Render();
private:
    static CD3D1XComputeShader* ms_pRayTraceShader;
};

