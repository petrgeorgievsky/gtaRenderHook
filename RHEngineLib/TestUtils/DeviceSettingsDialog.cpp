#include "stdafx.h"
#include "DeviceSettingsDialog.h"
#include "../Engine/RwRenderEngine.h"
#include "../DebugUtils/DebugLogger.h"
#include "../resource.h"

int RHTests::DeviceSettingsDialog::m_nCurrentAdapter=0;
int RHTests::DeviceSettingsDialog::m_nCurrentMonitor=0;
int RHTests::DeviceSettingsDialog::m_nCurrentDisplayMode=0;
std::vector<RwSubSystemInfo> RHTests::DeviceSettingsDialog::m_aSubSysInfo;

void RHTests::DeviceSettingsDialog::SetSubSystemInfo(const std::vector<RwSubSystemInfo>& subSysInfo)
{
    m_aSubSysInfo = subSysInfo;
}

INT_PTR RHTests::DeviceSettingsDialog::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    {
        Init(hDlg);

        return FALSE;
    }
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDC_DEVICECB:
        {
            DeviceSelect(hDlg);

            return TRUE;
        }
        case IDC_MONITORCB:
        {
            MonitorSelect(hDlg);

            return TRUE;
        }
        case IDC_DISPLAYCB:
        {
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                HWND                wndListVideMode;
                RwInt32             vmSel;

                wndListVideMode = GetDlgItem(hDlg, IDC_DISPLAYCB);

                /* Update the selected entry */
                vmSel = SendMessage(wndListVideMode, CB_GETCURSEL, 0, 0);
                m_nCurrentDisplayMode = SendMessage(wndListVideMode, CB_GETITEMDATA, vmSel, 0);
            }

            return TRUE;
        }

        case IDOK:
        {
            if (HIWORD(wParam) == BN_CLICKED)
                EndDialog(hDlg, TRUE);
            
            return TRUE;
        }

        case IDCANCEL:
        {
            if (HIWORD(wParam) == BN_CLICKED)
                EndDialog(hDlg, FALSE);
            
            return TRUE;
        }

        default:
        {
            return FALSE;
        }
        }
    }

    default:
    {
        return FALSE;
    }
    }

    return FALSE;
}

void RHTests::DeviceSettingsDialog::AddModes(HWND wndListVideMode)
{
    RwInt32             mode, videoModeCount;
    RwVideoMode         modeInfo;

    RHEngine::g_pRWRenderEngine->GetNumModes(videoModeCount);

    /* Add the available video modes to the dialog */
    for (mode = 0; mode < videoModeCount; mode++)
    {
        int index;

        RHEngine::g_pRWRenderEngine->GetModeInfo(modeInfo, mode);

        /* We display width height and refresh rate */
        std::wstringstream ss;
        ss  << modeInfo.width	<< L"x" 
            << modeInfo.height	<< L"("
            << modeInfo.refRate << L" Hz)" << std::endl;

        /* Add name and an index so we can ID it later */
        index =
            SendMessage(wndListVideMode, CB_ADDSTRING, 0, (LPARAM)ss.str().c_str());
        SendMessage(wndListVideMode, CB_SETITEMDATA, index, (LPARAM)mode);
    }
}

void RHTests::DeviceSettingsDialog::AddMonitors(HWND wndListMonitors)
{
    RwInt32             monitor, numMonitors;
    std::wstring        monitorInfo;

    RHEngine::g_pRWRenderEngine->GetNumOutputs(m_nCurrentAdapter, numMonitors);

    /* Add the available video modes to the dialog */
    for (monitor = 0; monitor < numMonitors; monitor++)
    {
        int index;

        RHEngine::g_pRWRenderEngine->GetOutputInfo(monitorInfo, monitor);
        /* Add name and an index so we can ID it later */
        index =
            SendMessage(wndListMonitors, CB_ADDSTRING, 0, (LPARAM)monitorInfo.c_str());
        SendMessage(wndListMonitors, CB_SETITEMDATA, index, (LPARAM)monitor);
    }
}

void RHTests::DeviceSettingsDialog::Init(HWND hDlg)
{
    HWND                wndList, wndListMonitorList, wndListDisplayMode;
    RwUInt32             subSysNum;

    /* Handle the list box */
    wndList				= GetDlgItem(hDlg, IDC_DEVICECB);
    wndListMonitorList	= GetDlgItem(hDlg, IDC_MONITORCB);
    wndListDisplayMode	= GetDlgItem(hDlg, IDC_DISPLAYCB);

    /* Add the names of the sub systems to the dialog */
    for (subSysNum = 0; subSysNum < m_aSubSysInfo.size(); subSysNum++)
    {
        /* Add name and an index so we can ID it later */
        SendMessage(wndList, CB_ADDSTRING, 0,
            (LPARAM)ToRHString(m_aSubSysInfo[subSysNum].name).c_str());
        SendMessage(wndList, CB_SETITEMDATA, subSysNum, (LPARAM)subSysNum);
    }
    SendMessage(wndList, CB_SETCURSEL, m_nCurrentAdapter, 0);

    RHEngine::g_pRWRenderEngine->SetSubSystem(m_nCurrentAdapter);

    AddMonitors(wndListMonitorList);

    m_nCurrentMonitor = 0;//RwEngineGetCurrentVideoMode();
    SendMessage(wndListMonitorList, CB_SETCURSEL, m_nCurrentMonitor, 0);

    AddModes(wndListDisplayMode);

    m_nCurrentDisplayMode = 0;//RwEngineGetCurrentVideoMode();
    SendMessage(wndListDisplayMode, CB_SETCURSEL, m_nCurrentDisplayMode, 0);

    SetFocus(wndList);
}

void RHTests::DeviceSettingsDialog::DeviceSelect(HWND hDlg)
{
    HWND                wndList, wndListMonitors, wndListVideMode;
    RwInt32             selection;

    /* Handle the list box */
    wndList = GetDlgItem(hDlg, IDC_DEVICECB);
    wndListMonitors = GetDlgItem(hDlg, IDC_MONITORCB);
    wndListVideMode = GetDlgItem(hDlg, IDC_DISPLAYCB);

    /* Update the selected entry */
    selection = SendMessage(wndList, CB_GETCURSEL, 0, 0);
    if (selection != m_nCurrentAdapter)
    {
        m_nCurrentAdapter = SendMessage(wndList, CB_GETITEMDATA, selection, 0);

        RHEngine::g_pRWRenderEngine->SetSubSystem(m_nCurrentAdapter);

        wndListMonitors = GetDlgItem(hDlg, IDC_MONITORCB);
        /* changed device so update monitors listbox */
        SendMessage(wndListMonitors, CB_RESETCONTENT, 0, 0);
        /* changed device so update video modes listbox */
        SendMessage(wndListVideMode, CB_RESETCONTENT, 0, 0);

        /* display avalible modes */
        AddMonitors(wndListMonitors);

        m_nCurrentMonitor = 0;//RwEngineGetCurrentVideoMode();
        SendMessage(wndListMonitors, CB_SETCURSEL, m_nCurrentMonitor, 0);

        /* display avalible modes */
        AddModes(wndListVideMode);

        m_nCurrentDisplayMode = 0;//RwEngineGetCurrentVideoMode();
        SendMessage(wndListVideMode, CB_SETCURSEL, m_nCurrentDisplayMode, 0);
    }
}

void RHTests::DeviceSettingsDialog::MonitorSelect(HWND hDlg)
{
    HWND                wndList, wndListVideMode;
    RwInt32             selection;

    /* Handle the list box */
    wndList = GetDlgItem(hDlg, IDC_MONITORCB);
    wndListVideMode = GetDlgItem(hDlg, IDC_DISPLAYCB);

    /* Update the selected entry */
    selection = SendMessage(wndList, CB_GETCURSEL, 0, 0);
    if (selection != m_nCurrentMonitor)
    {
        m_nCurrentMonitor = SendMessage(wndList, CB_GETITEMDATA, selection, 0);

        RHEngine::g_pRWRenderEngine->SetOutput(m_nCurrentMonitor);

        wndListVideMode = GetDlgItem(hDlg, IDC_DISPLAYCB);
        /* changed device so update video modes listbox */
        SendMessage(wndListVideMode, CB_RESETCONTENT, 0, 0);

        /* display avalible modes */
        AddModes(wndListVideMode);

        m_nCurrentDisplayMode = 0;//RwEngineGetCurrentVideoMode();
        SendMessage(wndListVideMode, CB_SETCURSEL, m_nCurrentDisplayMode, 0);
    }
}
