#pragma once
#include "../../stdafx.h"
#include "../../DebugUtils/DebugLogger.h"
#include "../Definitions.h"

/**
    @brief D3D API specific wrapper.
    D3D API specific wrapper, prints out debug message and error code in debug file and throws runtime error.
*/
inline bool CALL_D3D_API(HRESULT callResult, const RHEngine::String& errorMessage) {
    if (FAILED(callResult)) {
        MessageBox(nullptr, errorMessage.c_str(), TEXT("RUNTIME ERROR"), 0);
        RHDebug::DebugLogger::Error(errorMessage);
        // TODO: add more readable results(for ex. convert HRESULT to string, based on result description)
        RHEngine::StringStream ss;
        ss << TEXT("ERROR_CODE:") << callResult << std::endl;
        RHDebug::DebugLogger::Log(ss.str(), RHDebug::LogLevel::Error);
        return false;
    }
    return true;
}

/**
    @brief D3D API specific wrapper.
    D3D API specific wrapper, prints out debug message and error code in debug file.
*/
inline bool CALL_D3D_API_SILENT(HRESULT callResult, const RHEngine::String& errorMessage) {
    if (FAILED(callResult)) {
        RHDebug::DebugLogger::Log(errorMessage, RHDebug::LogLevel::Error);
        // TODO: add more readable results(for ex. convert HRESULT to string, based on result description)
        RHEngine::StringStream ss;
        ss << TEXT("ERROR_CODE:") << callResult << std::endl;
        RHDebug::DebugLogger::Log(ss.str(), RHDebug::LogLevel::Error);
        MessageBox(nullptr, errorMessage.c_str(), TEXT("RUNTIME SILENT ERROR"), 0);
        return false;
    }
    return true;
}