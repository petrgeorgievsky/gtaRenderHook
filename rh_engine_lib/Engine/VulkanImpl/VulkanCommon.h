#pragma once
#include "../../DebugUtils/DebugLogger.h"

/**
 * @brief Vulkan API specific wrapper prints out debug message and error code in
 * debug file and throws runtime error
 *
 * @param callResult - result of Vulkan API call
 * @param errorMessage - message, that should be written if error occurs
 * @return true if call resulted in success
 */
inline bool CALL_VK_API( VkResult callResult, const rh::engine::String &errorMessage )
{
    rh::engine::StringStream ss;

    if ( callResult != VkResult::VK_SUCCESS ) {
        MessageBox( nullptr, errorMessage.c_str(), TEXT( "RUNTIME ERROR" ), 0 );

        rh::debug::DebugLogger::Error( errorMessage );

        // TODO: add more readable results(for ex. convert HRESULT to string, based
        // on result description)
        ss << TEXT( "ERROR_CODE:" ) << callResult << '\n';

        rh::debug::DebugLogger::Log( ss.str(), rh::debug::LogLevel::Error );

        return false;
    }

    return true;
}

/**
 * @brief Vulkan API specific wrapper prints out debug message and error code in
 * debug file and throws runtime error
 *
 * @param callResult - result of Vulkan API call
 * @param errorMessage - message, that should be written if error occurs
 * @return true if call resulted in success
 */
inline bool CALL_VK_API( vk::Result callResult, const rh::engine::String &errorMessage )
{
    rh::engine::StringStream ss;

    if ( callResult != vk::Result::eSuccess ) {
        MessageBox( nullptr, errorMessage.c_str(), TEXT( "RUNTIME ERROR" ), 0 );

        rh::debug::DebugLogger::Error( errorMessage );

        // TODO: add more readable results(for ex. convert HRESULT to string, based
        // on result description)
        ss << TEXT( "ERROR_CODE:" ) << callResult << '\n';

        rh::debug::DebugLogger::Log( ss.str(), rh::debug::LogLevel::Error );

        return false;
    }

    return true;
}
