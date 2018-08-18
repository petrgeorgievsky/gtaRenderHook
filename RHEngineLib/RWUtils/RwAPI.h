#pragma once
#include "../stdafx.h"
namespace RH_RWAPI
{
	RwCamera * RwCameraCreate();

    void RwCameraDestroy(RwCamera * camera);

	RwRaster * RwRasterCreate(RwInt32 width, RwInt32 height, RwInt32 depth, RwInt32 flags);

    void RwRasterDestroy(RwRaster * raster);

	extern RwGlobals* g_pRHRwEngineInstance;
};