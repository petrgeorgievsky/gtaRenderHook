#include "stdafx.h"
#include "RHRayTracer.h"

CD3D1XComputeShader* RHRayTracer::ms_pRayTraceShader = nullptr;

void RHRayTracer::Init()
{
    ms_pRayTraceShader = new CD3D1XComputeShader( "RayTracing.hlsl", "main" );
}

void RHRayTracer::Render()
{
    ms_pRayTraceShader->Set();
}

void RHRayTracer::Shutdown()
{
    delete ms_pRayTraceShader;
}
