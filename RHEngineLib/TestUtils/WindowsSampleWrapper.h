/**
 * @brief This file contains abstraction for Windows test samples.
 *
 * @file WindowsSampleWrapper.h
 * @author Peter Georgievsky
 * @date 2018-08-05
 */
#pragma once
#include "../Engine/Definitions.h"
#include "Engine/Common/types/string_typedefs.h"
#include "TestSample.h"
#include <common.h>

namespace rh::tests {

/**
 * @brief Windows sample parameters, used for sample wrapper initialization
 *
 */
struct WindowsSampleParams
{
    HINSTANCE instance = nullptr;
    rh::engine::String sampleTitle;
    rh::engine::String windowClass;
};

/**
 * @brief Test Sample wrapper for Windows OS, manages window and sample
 * lifecycle.
 *
 */
class WindowsSampleWrapper
{
public:
    /**
   * @brief Constructor of WindowsSampleWrapper
   *
   * @param params - sample window parameters
   * @param sampleImpl - test sample implementation
   */
    WindowsSampleWrapper( WindowsSampleParams params, std::unique_ptr<TestSample> sampleImpl );

    /**
   * @brief Initializes main window and test sample
   *
   * @return true if no problems occured
   */
    BOOL Init();

    /**
   * @brief Runs test sample, will block running thread
   *
   */
    void Run();

private:
    /**
   * @brief Registers window class for test sample window
   *
   */
    void RegisterWindowClass();

    /**
   * @brief Initializes window
   *
   * @return Window handle or nullptr if error occurs
   */
    HWND InitWindow();

    /**
   * @brief Main window procedure of test sample
   *
   */
    static LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

private:
    LPDIRECTINPUT8 m_pDirectInput = nullptr;
    static IDirectInputDevice8 *m_pMouse;
    WindowsSampleParams m_sParams;
    HWND m_hWnd{};
    static std::thread ms_renderingThread;
    static std::unique_ptr<TestSample> ms_pSampleImpl;
};
} // namespace RHTests
