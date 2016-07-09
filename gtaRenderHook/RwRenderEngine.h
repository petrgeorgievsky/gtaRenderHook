#include "CDebug.h"
#ifndef RW_RENDER_ENGINE_H
#define RW_RENDER_ENGINE_H

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = nullptr; } }
enum class RwRenderSystemState
{
	RW_RENDER_INIT=0,
	RW_RENDER_SHUTDOWN = 1,
	RW_CREATE_RENDER_DEVICE = 2,
	RW_SHUTDOWN_RENDER_DEVICE = 3,
};

class CIRwRenderEngine
{
public:
	/*virtual ~CIRwRenderEngine()=0;*/
	virtual bool Create(HWND*) = 0;
	virtual bool Shutdown() = 0;
	virtual bool CreateRenderDevice() = 0;
	virtual bool ShutdownRenderDevice() = 0;
	virtual bool BaseEventHandler(int State, int* a2, void* a3, int a4) = 0;
	CIRwRenderEngine(CDebug* d) :m_pDebug{ d } {};
	bool EventHandlingSystem(RwRenderSystemState State, int* a2, void* a3, int a4);
protected:
	CDebug* m_pDebug;
};

class CRwD3DEngine: public CIRwRenderEngine
{
public:
	bool BaseEventHandler(int State, int* a2, void* a3, int a4) override;
	bool Create(HWND*) override;
	bool Shutdown() override;
	bool CreateRenderDevice() override;
	bool ShutdownRenderDevice() override;
	CRwD3DEngine(CDebug* d) :CIRwRenderEngine(d) {};
private:
	const D3DFORMAT m_aBackBufferFormat[3] { D3DFMT_R5G6B5, D3DFMT_X8R8G8B8, D3DFMT_A2R10G10B10 };
	void m_setPresentParameters(const D3DDISPLAYMODE&);
};

#endif 

