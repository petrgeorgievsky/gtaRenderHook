#pragma once
#include "../stdafx.h"
#include "../Engine/Definitions.h"
#include "../Engine/RwRenderEngine.h"

namespace RHTests {
	class TestSample
	{
	public:
		TestSample(RHEngine::RHRenderingAPI api, HINSTANCE inst);
		~TestSample();
		bool Initialize(HWND wnd);
		void Update();
		void Stop();
		virtual bool CustomInitialize();
		virtual void CustomUpdate();
        virtual void CustomRender();
        virtual void CustomShutdown();
    protected:
        RwCamera * m_pMainCamera;
	private:
		std::vector<RwSubSystemInfo> m_aSubSysInfo;
        long double averageTimePerFrame = -1;
        unsigned long frameCount=0;
		bool m_bUpdate = true;
		HINSTANCE hInst;
	};
};
