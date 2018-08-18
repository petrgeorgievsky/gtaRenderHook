#include "stdafx.h"
#include "WindowsSampleWrapper.h"

std::thread RHTests::WindowsSampleWrapper::ms_renderingThread;
std::unique_ptr<RHTests::TestSample> RHTests::WindowsSampleWrapper::ms_pSampleImpl = nullptr;

RHTests::WindowsSampleWrapper::WindowsSampleWrapper(const WindowsSampleParams &params, std::unique_ptr<TestSample> sampleImpl):
    m_sParams(params)
{
    ms_pSampleImpl = std::move(sampleImpl);
}

BOOL RHTests::WindowsSampleWrapper::Init()
{
    RegisterWindowClass();

    m_hWnd = InitWindow();

    if (!m_hWnd)
        return FALSE;

    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);

    if (!ms_pSampleImpl->Initialize(m_hWnd))
        return FALSE;
    return TRUE;
}

void RHTests::WindowsSampleWrapper::Run()
{
    ms_renderingThread = std::thread([&]() { ms_pSampleImpl->Update(); });
    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void RHTests::WindowsSampleWrapper::RegisterWindowClass()
{
    WNDCLASS wclass = {};

    wclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wclass.lpfnWndProc = WndProc;
    wclass.hInstance = m_sParams.instance;
    wclass.lpszClassName = m_sParams.windowClass.c_str();

    RegisterClass(&wclass);
}

HWND RHTests::WindowsSampleWrapper::InitWindow()
{
    return CreateWindow(m_sParams.windowClass.c_str(), m_sParams.sampleTitle.c_str(), WS_OVERLAPPEDWINDOW,
        200, 200, 640, 480, nullptr, nullptr, m_sParams.instance, nullptr);
}

LRESULT CALLBACK RHTests::WindowsSampleWrapper::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        ms_pSampleImpl->Stop();
        if( ms_renderingThread.joinable() )
            ms_renderingThread.join();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}