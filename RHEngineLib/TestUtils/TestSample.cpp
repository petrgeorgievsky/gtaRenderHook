#include "stdafx.h"
#include "../resource.h"
#include "TestSample.h"
#include "../DebugUtils/DebugLogger.h"
#include "../RWUtils/RwAPI.h"
#include "DeviceSettingsDialog.h"

RHTests::TestSample::TestSample(RHEngine::RHRenderingAPI api, HINSTANCE inst)
{
    RHEngine::g_pRWRenderEngine = std::unique_ptr<RHEngine::RwRenderEngine>(new RHEngine::RwRenderEngine(api));
    RH_RWAPI::g_pRHRwEngineInstance = (RwGlobals*)malloc(sizeof(RwGlobals));
    hInst = inst;
}

RHTests::TestSample::~TestSample()
{
    free(RH_RWAPI::g_pRHRwEngineInstance);
}

bool RHTests::TestSample::Initialize(HWND wnd)
{
    int gpuCount;

    // Preparation of rendering engine, initializes info about hardware that'll use this window
    if (!RHEngine::g_pRWRenderEngine->Open(wnd))
    {
        RHDebug::DebugLogger::Error(L"Failed to open RHEngine!");
        return false;
    }

    // Preparation of standard rendering callbacks 
    if (!RHEngine::g_pRWRenderEngine->Standards((int*)RH_RWAPI::g_pRHRwEngineInstance->stdFunc,
        rwSTANDARDNUMOFSTANDARD))
    {
        RHDebug::DebugLogger::Error(L"Failed to initialize RH engine standards!");
        return false;
    }

    // GPU count retrieval
    if (!RHEngine::g_pRWRenderEngine->GetNumSubSystems(gpuCount))
    {
        RHEngine::g_pRWRenderEngine->Close();
        RHDebug::DebugLogger::Error(L"Failed to get gpu count!");
        return false;
    }

    RHDebug::DebugLogger::Log(L"GPU List:");
    // Enumerate over GPUs and log all the info
    for (int i = 0; i < gpuCount; i++) {
        RwSubSystemInfo info;
        if (!RHEngine::g_pRWRenderEngine->GetSubSystemInfo(info, i))
        {
            RHEngine::g_pRWRenderEngine->Close();
            RHDebug::DebugLogger::Error(L"Failed to get gpu info!");
            return false;
        }
        m_aSubSysInfo.push_back(info);
        RHDebug::DebugLogger::Log(ToRHString(info.name));
    }

    // Show device settings dialog
    DeviceSettingsDialog::SetSubSystemInfo(m_aSubSysInfo);
    if (DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), wnd, DeviceSettingsDialog::DialogProc) <= 0)
    {
        RHDebug::DebugLogger::Error(std::to_wstring(GetLastError()));
    }

    if (!RHEngine::g_pRWRenderEngine->Start())
    {
        RHDebug::DebugLogger::Error(L"Failed to start RWEngine!");
        RHEngine::g_pRWRenderEngine->Close();
        return false;
    }
    return CustomInitialize();
}

void RHTests::TestSample::Update()
{
    using namespace std::chrono;
    high_resolution_clock::time_point t1;
    high_resolution_clock::time_point t2;
    double currentFrameTime;

    while (m_bUpdate)
    {
        t1 = high_resolution_clock::now();
        CustomUpdate();

        RHEngine::g_pRWRenderEngine->CameraBeginUpdate(m_pMainCamera);
        
        CustomRender();

        RHEngine::g_pRWRenderEngine->CameraEndUpdate(m_pMainCamera);

        RHEngine::g_pRWRenderEngine->RasterShowRaster(m_pMainCamera->frameBuffer, 0);
        t2 = high_resolution_clock::now();
        currentFrameTime = duration_cast<duration<double>>(t2 - t1).count();
        frameCount++;
        averageTimePerFrame = (averageTimePerFrame * (frameCount - 1) + currentFrameTime) / frameCount;
        //RHDebug::DebugLogger::Log(std::to_wstring(averageTimePerFrame));
    }
    RHDebug::DebugLogger::Log(std::to_wstring(averageTimePerFrame));
    CustomShutdown();
}

void RHTests::TestSample::Stop()
{
    m_bUpdate = false;
}

bool RHTests::TestSample::CustomInitialize()
{
    m_pMainCamera = RH_RWAPI::RwCameraCreate();
    m_pMainCamera->frameBuffer = RH_RWAPI::RwRasterCreate(800, 600, 32, rwRASTERTYPECAMERA);
    return true;
}

void RHTests::TestSample::CustomUpdate()
{
}

void RHTests::TestSample::CustomRender()
{

}

void RHTests::TestSample::CustomShutdown()
{
    if (m_pMainCamera)
    {
        if (m_pMainCamera->frameBuffer) 
        {
            RH_RWAPI::RwRasterDestroy(m_pMainCamera->frameBuffer);
            m_pMainCamera->frameBuffer = nullptr;
        }
        RH_RWAPI::RwCameraDestroy(m_pMainCamera);
        m_pMainCamera = nullptr;
    }
    if (RHEngine::g_pRWRenderEngine) {
        RHEngine::g_pRWRenderEngine->Stop();
        RHEngine::g_pRWRenderEngine->Close();
    }
}
