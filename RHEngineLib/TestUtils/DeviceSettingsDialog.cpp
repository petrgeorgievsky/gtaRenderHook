#include "DeviceSettingsDialog.h"
#include "../DebugUtils/DebugLogger.h"
#include "../resource.h"

unsigned int rh::tests::DeviceSettingsDialog::m_nCurrentAdapter = 0;
int rh::tests::DeviceSettingsDialog::m_nCurrentMonitor = 0;
int rh::tests::DeviceSettingsDialog::m_nCurrentDisplayMode = 0;
std::vector<RwSubSystemInfo> rh::tests::DeviceSettingsDialog::m_aSubSysInfo;

void rh::tests::DeviceSettingsDialog::SetSubSystemInfo( const std::vector<RwSubSystemInfo>& subSysInfo )
{
    m_aSubSysInfo = subSysInfo;
}

INT_PTR rh::tests::DeviceSettingsDialog::DialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM /*lParam*/ )
{
    switch ( message )
    {
    case WM_INITDIALOG:
    {
        Init( hDlg );

        return FALSE;
    }
    case WM_COMMAND:
    {
        switch ( LOWORD( wParam ) )
        {
        case IDC_DEVICECB:
        {
            DeviceSelect( hDlg );

            return TRUE;
        }
        case IDC_MONITORCB:
        {
            MonitorSelect( hDlg );

            return TRUE;
        }
        case IDC_DISPLAYCB:
        {
            if ( HIWORD( wParam ) == CBN_SELCHANGE )
            {
                HWND                wndListVideMode;
                RwInt32             vmSel;

                wndListVideMode = GetDlgItem( hDlg, IDC_DISPLAYCB );

                /* Update the selected entry */
                vmSel = static_cast<RwInt32> ( SendMessage( wndListVideMode, CB_GETCURSEL, 0, 0 ) );
                m_nCurrentDisplayMode = static_cast<RwInt32> ( SendMessage( wndListVideMode, CB_GETITEMDATA, vmSel, 0 ) );
            }

            return TRUE;
        }

        case IDOK:
        {
            if ( HIWORD( wParam ) == BN_CLICKED )
                EndDialog( hDlg, TRUE );

            return TRUE;
        }

        case IDCANCEL:
        {
            if ( HIWORD( wParam ) == BN_CLICKED )
                EndDialog( hDlg, FALSE );

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
}

void rh::tests::DeviceSettingsDialog::AddModes( HWND wndListVideMode )
{
    RwInt32 mode;
    RwInt32 videoModeCount;
    RwVideoMode         modeInfo;

    rh::engine::g_pRWRenderEngine->GetNumModes( videoModeCount );

    /* Add the available video modes to the dialog */
    for ( mode = 0; mode < videoModeCount; mode++ )
    {
        int index;

        rh::engine::g_pRWRenderEngine->GetModeInfo( modeInfo, mode );

        /* We display width height and refresh rate */
        std::wstringstream ss;
        ss << modeInfo.width << L"x"
            << modeInfo.height << L"("
            << modeInfo.refRate << L" Hz)" << std::endl;

        /* Add name and an index so we can ID it later */
        index = static_cast<int>( SendMessage( wndListVideMode,
                                               CB_ADDSTRING,
                                               0,
                                               reinterpret_cast<LPARAM>( ss.str().c_str() ) ) );
        SendMessage( wndListVideMode, CB_SETITEMDATA, index, static_cast<LPARAM>( mode ) );
    }
}

void rh::tests::DeviceSettingsDialog::AddMonitors( HWND wndListMonitors )
{
    RwUInt32 monitor;
    RwInt32 numMonitors;
    std::wstring        monitorInfo;

    rh::engine::g_pRWRenderEngine->GetNumOutputs( m_nCurrentAdapter, numMonitors );

    /* Add the available video modes to the dialog */
    for ( monitor = 0; monitor < static_cast<RwUInt32>( numMonitors ); monitor++ ) {
        int index;

        rh::engine::g_pRWRenderEngine->GetOutputInfo( monitorInfo, monitor );

        /* Add name and an index so we can ID it later */
        index = static_cast<int>( SendMessage( wndListMonitors,
                                               CB_ADDSTRING,
                                               0,
                                               reinterpret_cast<LPARAM>( monitorInfo.c_str() ) ) );
        SendMessage( wndListMonitors, CB_SETITEMDATA, index, static_cast<LPARAM>( monitor ) );
    }
}

void rh::tests::DeviceSettingsDialog::Init( HWND hDlg )
{
    HWND wndList;
    HWND wndListMonitorList;
    HWND                wndListDisplayMode;
    RwUInt32             subSysNum;

    /* Handle the list box */
    wndList = GetDlgItem( hDlg, IDC_DEVICECB );
    wndListMonitorList = GetDlgItem( hDlg, IDC_MONITORCB );
    wndListDisplayMode = GetDlgItem( hDlg, IDC_DISPLAYCB );

    /* Add the names of the sub systems to the dialog */
    for ( subSysNum = 0; subSysNum < m_aSubSysInfo.size(); subSysNum++ )
    {
        /* Add name and an index so we can ID it later */
        SendMessage( wndList,
                     CB_ADDSTRING,
                     0,
                     reinterpret_cast<LPARAM>(
                         ToRHString( m_aSubSysInfo[subSysNum].name ).c_str() ) );
        SendMessage( wndList, CB_SETITEMDATA, subSysNum, static_cast<LPARAM>( subSysNum ) );
    }
    SendMessage( wndList, CB_SETCURSEL, m_nCurrentAdapter, 0 );

    rh::engine::g_pRWRenderEngine->SetSubSystem( m_nCurrentAdapter );

    AddMonitors( wndListMonitorList );

    m_nCurrentMonitor = 0;//RwEngineGetCurrentVideoMode();
    SendMessage( wndListMonitorList, CB_SETCURSEL, m_nCurrentMonitor, 0 );

    AddModes( wndListDisplayMode );

    m_nCurrentDisplayMode = 0;//RwEngineGetCurrentVideoMode();
    SendMessage( wndListDisplayMode, CB_SETCURSEL, m_nCurrentDisplayMode, 0 );

    SetFocus( wndList );
}

void rh::tests::DeviceSettingsDialog::DeviceSelect( HWND hDlg )
{
    HWND wndList;
    HWND wndListMonitors;
    HWND                wndListVideMode;/* Handle the list box */
    wndList = GetDlgItem( hDlg, IDC_DEVICECB );
    wndListMonitors = GetDlgItem( hDlg, IDC_MONITORCB );
    wndListVideMode = GetDlgItem( hDlg, IDC_DISPLAYCB );

    /* Update the selected entry */
    auto selection = static_cast<RwUInt32>( SendMessage( wndList, CB_GETCURSEL, 0, 0 ) );

    if ( selection != m_nCurrentAdapter )
    {
        m_nCurrentAdapter = static_cast<RwUInt32>(
            SendMessage( wndList, CB_GETITEMDATA, selection, 0 ) );

        rh::engine::g_pRWRenderEngine->SetSubSystem( m_nCurrentAdapter );

        wndListMonitors = GetDlgItem( hDlg, IDC_MONITORCB );

        /* changed device so update monitors listbox */
        SendMessage( wndListMonitors, CB_RESETCONTENT, 0, 0 );

        /* changed device so update video modes listbox */
        SendMessage( wndListVideMode, CB_RESETCONTENT, 0, 0 );

        /* display avalible modes */
        AddMonitors( wndListMonitors );

        m_nCurrentMonitor = 0;//RwEngineGetCurrentVideoMode();
        SendMessage( wndListMonitors, CB_SETCURSEL, m_nCurrentMonitor, 0 );

        /* display avalible modes */
        AddModes( wndListVideMode );

        m_nCurrentDisplayMode = 0;//RwEngineGetCurrentVideoMode();
        SendMessage( wndListVideMode, CB_SETCURSEL, m_nCurrentDisplayMode, 0 );
    }
}

void rh::tests::DeviceSettingsDialog::MonitorSelect( HWND hDlg )
{
    HWND wndList;
    HWND wndListVideMode;
    wndList = GetDlgItem( hDlg, IDC_MONITORCB );
    wndListVideMode = GetDlgItem( hDlg, IDC_DISPLAYCB );

    /* Update the selected entry */
    auto selection = static_cast<RwInt32>( SendMessage( wndList, CB_GETCURSEL, 0, 0 ) );

    if ( selection != m_nCurrentMonitor )
    {
        m_nCurrentMonitor = static_cast<int>( SendMessage( wndList, CB_GETITEMDATA, selection, 0 ) );

        rh::engine::g_pRWRenderEngine->SetOutput( static_cast<RwUInt32>( m_nCurrentMonitor ) );

        wndListVideMode = GetDlgItem( hDlg, IDC_DISPLAYCB );

        /* changed device so update video modes listbox */
        SendMessage( wndListVideMode, CB_RESETCONTENT, 0, 0 );

        /* display avalible modes */
        AddModes( wndListVideMode );

        m_nCurrentDisplayMode = 0;//RwEngineGetCurrentVideoMode();
        SendMessage( wndListVideMode, CB_SETCURSEL, m_nCurrentDisplayMode, 0 );
    }
}
