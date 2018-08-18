#include "stdafx.h"
#include "DebugLogger.h"
using namespace RHDebug;
std::unique_ptr<RHEngine::OutFileStream> DebugLogger::m_pLogStream = nullptr;
RHEngine::String DebugLogger::m_sFileName = TEXT("rhdebug.log");
LogLevel DebugLogger::m_MinLogLevel = LogLevel::Info;

RHEngine::String ToRHString(const std::string& t_str)
{
#ifndef UNICODE
    return t_str;
#else

	//setup converter
	typedef std::codecvt_utf8<wchar_t> convert_type;
	std::wstring_convert<convert_type, wchar_t> converter;

	//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
	return converter.from_bytes(t_str);
#endif
}

std::string FromRHString(const RHEngine::String& t_str)
{
#ifndef UNICODE
    return t_str;
#else
	//setup converter
	typedef std::codecvt_utf8<wchar_t> convert_type;
	std::wstring_convert<convert_type, wchar_t> converter;

	//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
	return converter.to_bytes(t_str);
#endif
}

void DebugLogger::Init(const RHEngine::String & fileName, LogLevel minLogLevel)
{
	m_sFileName		= fileName;
	m_MinLogLevel	= minLogLevel;
	if (!m_sFileName.empty())
		m_pLogStream = std::unique_ptr<std::wofstream>(new std::wofstream(m_sFileName));
}

void DebugLogger::Log(const RHEngine::String & msg, LogLevel logLevel)
{
	if (logLevel >= m_MinLogLevel) {
		if (m_pLogStream) {
			if (!m_pLogStream->is_open())
				m_pLogStream->open(m_sFileName, std::fstream::out | std::fstream::app);
			*m_pLogStream << TEXT("LOG: ") << msg << TEXT("\n");
			m_pLogStream->close();
		}
		OutputDebugString((msg + TEXT("\n")).c_str());
	}
}

void DebugLogger::Error(const RHEngine::String & msg)
{
	if (m_pLogStream) {
		if (!m_pLogStream->is_open())
			m_pLogStream->open(m_sFileName, std::fstream::out | std::fstream::app);
		*m_pLogStream << TEXT("ERROR: ") << msg << TEXT("\n");
		m_pLogStream->close();
	}
	OutputDebugString((msg + TEXT("\n")).c_str());
}
