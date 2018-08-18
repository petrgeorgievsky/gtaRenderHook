/**
 * @brief This file contains utilities for device settings selection
 * 
 * @file DeviceSettingsDialog.h
 * @author Peter Georgievsky
 * @date 2018-07-19
 */
#pragma once
#include "../stdafx.h"
namespace RHTests {
        /**
         * @brief Device settings dialog class
         * 
         * Contains methods that spawn device selection dialog
         */
	class DeviceSettingsDialog
	{
	public:
                /**
                 * @brief Set the Sub-System Info list
                 * 
                 * @param subSysInfo - list of sub-systems info
                 */
                static void SetSubSystemInfo(const std::vector<RwSubSystemInfo> &subSysInfo);

                /**
                 * @brief Device settings dialog procedure
                 * 
                 */
                static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

                private:

                /**
                 * @brief Adds video modes to wndListVideMode
                 * 
                 * @param wndListVideMode - video mode list handle
                 */
                static void AddModes(HWND wndListVideMode);

                /**
                 * @brief Adds monitors to wndListVideMode
                 * 
                 * @param wndListMonitors - monitor list handle
                 */
                static void AddMonitors(HWND wndListMonitors);

                /**
                 * @brief Initializes device settings dialog
                 * 
                 * @param hDlg - dialog handle
                 */
                static void Init(HWND hDlg);

                /**
                 * @brief Changes selected device
                 * 
                 * @param hDlg - dialog handle
                 */
                static void DeviceSelect(HWND hDlg);

                /**
                 * @brief Changes selected monitor
                 * 
                 * @param hDlg - dialog handle
                 */
                static void MonitorSelect(HWND hDlg);
        
	private:
		static int m_nCurrentAdapter;
		static int m_nCurrentMonitor;
		static int m_nCurrentDisplayMode;
		static std::vector<RwSubSystemInfo> m_aSubSysInfo;
	};
};