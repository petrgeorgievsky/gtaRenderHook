#pragma once
namespace RHEngine {
#ifndef UNICODE
    typedef std::string String;
    typedef std::stringstream StringStream;
    typedef std::ofstream OutFileStream;
#else
    typedef std::wstring String;
    typedef std::wstringstream StringStream;
    typedef std::wofstream OutFileStream;
#endif
	enum class RHRenderingAPI
	{
		DX11,
		DX12,
		Vulkan,
		OpenGL
	};
	/*
		This enum contains all possible image buffers supported by RH
	*/
	enum class RHImageBufferType 
	{
		Unknown,
		BackBuffer,
		TextureBuffer,
		DepthBuffer,
		RenderTargetBuffer
	};
	/*
		This enum contains all possible image buffer clear types
	*/
	enum class RHImageClearType
	{
		Color,
		Depth,
		Stencil,
		DepthStencil
	};
	/*
		This enum contains all possible image buffer to pipeline bindings
	*/
	enum class RHImageBindType
	{
		Unknown,
		RenderTarget,
		DepthStencilTarget,
		UnorderedAccessTarget,
		ShaderResource
	};
	/**
		This enum contains all possible requests to rendering system of RenderWare
	*/
	enum class RwRenderSystemRequest
	{
		rwDEVICESYSTEMOPEN = 0x00,
		rwDEVICESYSTEMCLOSE,
		rwDEVICESYSTEMSTART,
		rwDEVICESYSTEMSTOP,
		rwDEVICESYSTEMREGISTER,
		rwDEVICESYSTEMGETNUMMODES,
		rwDEVICESYSTEMGETMODEINFO,
		rwDEVICESYSTEMUSEMODE,
		rwDEVICESYSTEMFOCUS,
		rwDEVICESYSTEMINITPIPELINE,
		rwDEVICESYSTEMGETMODE,
		rwDEVICESYSTEMSTANDARDS,
		rwDEVICESYSTEMGETTEXMEMSIZE,
		rwDEVICESYSTEMGETNUMSUBSYSTEMS,
		rwDEVICESYSTEMGETSUBSYSTEMINFO,
		rwDEVICESYSTEMGETCURRENTSUBSYSTEM,
		rwDEVICESYSTEMSETSUBSYSTEM,
		rwDEVICESYSTEMFINALIZESTART,
		rwDEVICESYSTEMINITIATESTOP,
		rwDEVICESYSTEMGETMAXTEXTURESIZE,
		rwDEVICESYSTEMRXPIPELINEREQUESTPIPE,
		rwDEVICESYSTEMGETMETRICBLOCK,
		rwDEVICESYSTEMGETID,
	};
};
