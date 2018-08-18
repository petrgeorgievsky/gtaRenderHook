#include "stdafx.h"
#include "D3D12Renderer.h"

RHEngine::D3D12Renderer::D3D12Renderer(HWND window, HINSTANCE inst):IRenderer(window, inst)
{

}

RHEngine::D3D12Renderer::~D3D12Renderer()
{
}

bool RHEngine::D3D12Renderer::InitDevice()
{
	return false;
}

bool RHEngine::D3D12Renderer::ShutdownDevice()
{
	return false;
}

bool RHEngine::D3D12Renderer::GetAdaptersCount(int &)
{
	return false;
}

bool RHEngine::D3D12Renderer::GetAdapterInfo(unsigned int n, std::wstring &)
{
	return false;
}

bool RHEngine::D3D12Renderer::GetOutputCount(unsigned int adapterId, int &)
{
	return false;
}

bool RHEngine::D3D12Renderer::SetCurrentOutput(unsigned int id)
{
	return false;
}

bool RHEngine::D3D12Renderer::GetOutputInfo(unsigned int n, std::wstring &)
{
	return false;
}

bool RHEngine::D3D12Renderer::SetCurrentAdapter(unsigned int n)
{
	return false;
}

bool RHEngine::D3D12Renderer::GetDisplayModeCount(unsigned int outputId, int &)
{
	return false;
}

bool RHEngine::D3D12Renderer::SetCurrentDisplayMode(unsigned int id)
{
	return false;
}

bool RHEngine::D3D12Renderer::GetDisplayModeInfo(unsigned int id, DisplayModeInfo &)
{
	return false;
}

bool RHEngine::D3D12Renderer::Present(void*)
{
	return false;
}

bool RHEngine::D3D12Renderer::GetCurrentAdapter(int &)
{
	return false;
}

bool RHEngine::D3D12Renderer::GetCurrentOutput(int &)
{
	return false;
}

bool RHEngine::D3D12Renderer::GetCurrentDisplayMode(int &)
{
	return false;
}

void * RHEngine::D3D12Renderer::AllocateImageBuffer(unsigned int width, unsigned int height, RHImageBufferType type)
{
	return nullptr;
}

bool RHEngine::D3D12Renderer::FreeImageBuffer(void * buffer, RHImageBufferType type)
{
	return false;
}

bool RHEngine::D3D12Renderer::BindImageBuffers(RHImageBindType bindType, const std::unordered_map<int, void*>& buffers)
{
	return false;
}

bool RHEngine::D3D12Renderer::ClearImageBuffer(RHImageClearType clearType, void* buffer, const float clearColor[4])
{
	return false;
}

bool RHEngine::D3D12Renderer::BeginCommandList(void * cmdList)
{
	return false;
}

bool RHEngine::D3D12Renderer::EndCommandList(void * cmdList)
{
	return false;
}

bool RHEngine::D3D12Renderer::RequestSwapChainImage(void * frameBuffer)
{
	return true;
}

bool RHEngine::D3D12Renderer::PresentSwapChainImage(void * frameBuffer)
{
	return true;
}
