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
#include <thread>

namespace rh::tests
{

/**
 * @brief Windows sample parameters, used for sample wrapper initialization
 *
 */
struct WindowsSampleParams
{
    void *             instance = nullptr;
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
    static std::thread                 ms_renderingThread;
    static std::unique_ptr<TestSample> ms_pSampleImpl;

  public:
    /**
     * @brief Constructor of WindowsSampleWrapper
     *
     * @param params - sample window parameters
     * @param sampleImpl - test sample implementation
     */
    WindowsSampleWrapper( WindowsSampleParams         params,
                          std::unique_ptr<TestSample> sampleImpl );

    /**
     * @brief Initializes main window and test sample
     *
     * @return true if no problems occured
     */
    bool Init();

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
    void *InitWindow();

  private:
    void *              m_pDirectInput = nullptr;
    static void *       m_pMouse;
    WindowsSampleParams m_sParams;
    void *              m_hWnd = nullptr;
};
} // namespace rh::tests
