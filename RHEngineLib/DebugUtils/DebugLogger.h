/**
 * @brief This file conatains logging utility for RenderHook rendering engine
 * 
 * @file DebugLogger.h
 * @author Peter Georgievsky
 * @date 2018-07-20
 * 
 * We use static class DebugLogger to write logs and errors to rhdebug.log file(by default) and IDE console.
 */
#pragma once
#include "../stdafx.h"
#include "../Engine/Definitions.h"

/// converts from string to wide string
RHEngine::String ToRHString(const std::string& t_str);
/// converts from wide string to string
std::string FromRHString(const RHEngine::String& t_str);

namespace RHDebug
{
	/**
	 * @brief Logging level enum
	 * 
	 */
	enum class LogLevel
	{
		Info=0,
		Warning,
		Error
	};

	/**
	 * @brief Debug logger class
	 * 
	 * Contains static methods for logging debug info 
	 */
	class DebugLogger
	{
	public:
		/**
		 * @brief Initializes DebugLogger
		 * 
		 * @param fileName - path to log file
		 * @param minLogLevel - minimum logging level
		 * 
		 * *If path is empty log won't be written to file.
		 */
		static void Init(const RHEngine::String &fileName, LogLevel minLogLevel);

		/**
		 * @brief Prints Log message to console and debug file if min log level is greater than logLevel
		 * 
		 * @param msg - log message
		 * @param logLevel - message logging level
		 */
		static void Log(const RHEngine::String &msg, LogLevel logLevel = LogLevel::Info);

		/**
		 * @brief Prints Error message to console and debug file
		 * 
		 * @param msg - log message
		 */
		static void Error(const RHEngine::String &msg);

	private:
		static std::unique_ptr<RHEngine::OutFileStream> m_pLogStream;
		static RHEngine::String m_sFileName;
		static LogLevel m_MinLogLevel;
	};
}

/**
    @brief Vulkan API specific wrapper.
    Vulkan API specific wrapper, prints out debug message and error code in debug file and throws runtime error.
*/
inline bool CALL_VK_API(VkResult callResult, const RHEngine::String& errorMessage) {
    if (callResult != VkResult::VK_SUCCESS) {
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