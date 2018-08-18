/**
    @file RwRenderEngine.h
    @author Peter Georgievsky
    @date 07.07.2018
    @version 1.0.0

    @brief This file conatains common interface declaration for RenderWare wrapper for RenderHook rendering engine.

	Render Hook engine is based mostly on RenderWare engine,
	but it doesn't follow philosophy of RW engine fully, 
	so it could be used as rendering engine on it's own without conveing to old 32bit standards of RW.
	This file contains the wrapper interface of common RW rendering functions.
*/

#pragma once
#include "../stdafx.h"
#include "Definitions.h"
#include "IRenderer.h"
namespace RHEngine
{
	struct RwStandard
	{
		RwInt32             nStandard;
		RwStandardFunc      fpStandard;
    };

    /**
     * @brief Converts RwRenderSystemRequest to wstring
     * @param req - system request.
     * @return RH string representation of system request
     */
	static String ToRHString(RwRenderSystemRequest req);

    /**
     * @class RwRenderEngine
     * @brief A wrapper between RenderWare and graphics API-specific stuff.
     * *This wrapper contains all the needed methods to wrap around all the methods of RenderWare engine
     */
	class RwRenderEngine
	{
    public:

		/**
		 * @brief Opens rendering engine
		 * 
		 * @return true - if rendering engine opens correctly
		 * @return false - if error occured
		 * 
		 * Assigns window handle and enumerates hardware info(videocards, monitors and display modes).
		 */
        bool Open(HWND);

        /**
         * @brief Closes rendering engine, releasing all used resources
         * @return true if rendering engine closes correctly, false otherwise
         */
        bool Close();

        /**
         * @brief Creates logical device, binded to selected hardware
         * @return true if device is created, false otherwise
         */
        bool Start();

        /**
         * @brief Destroys logical device and all resources allocated by that device
         * @return true , false otherwise
         */
        bool Stop();

		/**
		 * @brief Get the count of display modes avaliable for selected monitor
		 * 
		 * @return true - if display mode count retrieved correctly
		 * @return false - if error occured
		 */
		bool GetNumModes(int&);
		/*
			Returns info about display mode
		*/
		bool GetModeInfo(RwVideoMode&, int);
		/*
			Selects current display mode.
		*/
		bool UseMode(int);
		/*
			Pocus
		*/
		bool Focus(bool);
		/*
			Returns currently selected display mode.
		*/
		bool GetMode(int&);

		bool Standards(int*, int);

		bool GetTexMemSize(int&);
		/*
			Returns subsystems(fancy name for GPUs or adapters) count
		*/
		bool GetNumSubSystems(int&);
		/*
			Returns subsystem info(name to be more exact)
		*/
		bool GetSubSystemInfo(RwSubSystemInfo&, int);
		/*
			Returns currently selected subsystem
		*/
		bool GetCurrentSubSystem(int&);
		/*
			Selects current subsystem.
		*/
		bool SetSubSystem(int);

		/*
			Returns output(monitor) count
		*/
		bool GetNumOutputs(int, int&);
		/*
			Returns output info
		*/
		bool GetOutputInfo(std::wstring& info, unsigned int n);
		/*
			Returns currently selected output
		*/
		bool GetCurrentOutput(int&);
		/*
			Selects current output.
		*/
		bool SetOutput(int);

		bool GetMaxTextureSize(int&);

		// Base event handler to handle events that doesn't use any API-dependent methods. TODO: remove this method and add few more(implement custom renderware RwDevice handle return)
		bool BaseEventHandler(int State, int* a2, void* a3, int a4);

		// State get-set methods.
		void	SetMultiSamplingLevels(int);
		int		GetMaxMultiSamplingLevels();
		bool	RenderStateSet(RwRenderState, UINT);
		bool	RenderStateGet(RwRenderState, UINT&);

		// Raster/Texture/Camera methods
		bool	RasterCreate(RwRaster *raster, UINT flags);
		bool	RasterDestroy(RwRaster *raster);
		bool	RasterLock(RwRaster *raster, UINT flags, void** data);
		bool	RasterUnlock(RwRaster *raster);
		bool	RasterShowRaster(RwRaster *raster, UINT flags);
		bool	NativeTextureRead(RwStream *stream, RwTexture** tex);
		bool	CameraClear(RwCamera *camera, RwRGBA *color, RwInt32 flags);
		bool	CameraBeginUpdate(RwCamera *camera);
		bool	CameraEndUpdate(RwCamera *camera);
		void	SetRenderTargets(RwRaster **rasters, RwRaster *zBuffer, RwUInt32 rasterCount);

		// Immediate mode render methods.
		bool	Im2DRenderPrimitive(RwPrimitiveType primType, RwIm2DVertex *vertices, RwUInt32 numVertices);
		bool	Im2DRenderIndexedPrimitive(RwPrimitiveType primType, RwIm2DVertex *vertices, RwUInt32 numVertices, RwImVertexIndex *indices, RwInt32 numIndices);
		RwBool	Im3DSubmitNode();

		void		SetTexture(RwTexture* tex, int Stage);

		bool AtomicAllInOneNode(RxPipelineNode *self, const RxPipelineNodeParam *params);
		bool SkinAllInOneNode(RxPipelineNode *self, const RxPipelineNodeParam *params);

		void	DefaultRenderCallback(RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags);
		RwBool	DefaultInstanceCallback(void *object, RxD3D9ResEntryHeader *resEntryHeader, RwBool reinstance);

		RwRenderEngine(RHRenderingAPI api) {
			m_renderingAPI = api;
        }
        ~RwRenderEngine() {  }

		// Render engine event system.
		bool EventHandlingSystem(RwRenderSystemRequest request, int * pOut, void * pInOut, int nIn);
	private:
		std::unique_ptr<IRenderer> m_pRenderer = nullptr;
		RHRenderingAPI m_renderingAPI;
	};
	extern std::unique_ptr<RwRenderEngine> g_pRWRenderEngine;
};
