#include "stdafx.h"
#include "RwAPI.h"

RwGlobals *RH_RWAPI::g_pRHRwEngineInstance;
RwCamera * RH_RWAPI::RwCameraCreate()
{
	RwCamera * camera = (RwCamera *)malloc(sizeof(RwCamera));
	if (camera == nullptr) return nullptr;

	/* Set up the defaults for the camera */
	camera->viewWindow.x = camera->viewWindow.y = 1.0f;
	camera->recipViewWindow.x = camera->recipViewWindow.y = 1.0f;
	camera->viewOffset.x = camera->viewOffset.y = 0.0f;

	/* Clipping planes */
	camera->nearPlane = (RwReal)((0.05));
	camera->farPlane = (RwReal)((10.0));
	camera->fogPlane = (RwReal)((5.0));

	/* Render destination rasters */
	camera->frameBuffer = (RwRaster *)NULL;
	camera->zBuffer = (RwRaster *)NULL;

	/* Set up projection type */
	camera->projectionType = rwPERSPECTIVE;
	return camera;
}

void RH_RWAPI::RwCameraDestroy(RwCamera * camera)
{
    if (camera) 
    {
        free(camera);
    }
}

RwRaster * RH_RWAPI::RwRasterCreate(RwInt32 width, RwInt32 height, RwInt32 depth, RwInt32 flags)
{
	RwRaster * raster = (RwRaster *)malloc(sizeof(RwRaster)+sizeof(void*));
	if (raster == nullptr)
		return nullptr;

	raster->privateFlags = 0;
	raster->cFlags = 0;
	raster->width = width;
	raster->height = height;
	raster->nOffsetX = 0;
	raster->nOffsetY = 0;
	raster->depth = depth;
	raster->parent = raster; /* It contains its own pixels */
	raster->cpPixels = (unsigned char *)NULL;
	raster->palette = (unsigned char *)NULL;
	const RwStandardFunc RasterCreateFunc =
		g_pRHRwEngineInstance->stdFunc[rwSTANDARDRASTERCREATE];

	if (!RasterCreateFunc(NULL, raster, flags))
	{
		free(raster);

		return nullptr;
	}

	return raster;
}

void RH_RWAPI::RwRasterDestroy(RwRaster * raster)
{   
    if (raster == nullptr)
        return;

    const RwStandardFunc RasterDestroyFunc =
        g_pRHRwEngineInstance->stdFunc[rwSTANDARDRASTERDESTROY];
    RasterDestroyFunc(nullptr, raster, 0);

    free(raster);
}
