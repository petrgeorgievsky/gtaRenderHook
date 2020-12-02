#include "DebugLogger.h"
#include <Windows.h>
#include <array>
#include <fstream>

using namespace rh::debug;

std::unique_ptr<rh::engine::OutFileStream> DebugLogger::m_pLogStream = nullptr;
rh::engine::String DebugLogger::m_sFileName            = TEXT( "rhdebug.log" );
LogLevel           DebugLogger::m_MinLogLevel          = LogLevel::Info;
void *             DebugLogger::g_hDebugPipeHandle     = nullptr;
void *             DebugLogger::g_hDebugPipeReadHandle = nullptr;

rh::engine::String ToRHString( const std::string &t_str )
{
#ifndef UNICODE
    return t_str;
#else
    rh::engine::String str;

    std::size_t res_size = MultiByteToWideChar( CP_ACP, 0, t_str.c_str(),
                                                t_str.size(), nullptr, 0 );

    str.resize( res_size );

    MultiByteToWideChar( CP_ACP, 0, t_str.c_str(), t_str.size(), str.data(),
                         res_size );

    return str;
#endif
}

rh::engine::String ToRHString( const std::wstring &t_str )
{
#ifdef UNICODE
    return t_str;
#else
    rh::engine::String str;
    const auto         res_size = WideCharToMultiByte( CP_ACP, 0, t_str.c_str(),
                                               static_cast<int>( t_str.size() ),
                                               nullptr, 0, nullptr, nullptr );
    str.resize( static_cast<std::size_t>( res_size ) );
    WideCharToMultiByte( CP_ACP, 0, t_str.c_str(),
                         static_cast<int>( t_str.size() ), str.data(),
                         static_cast<int>( res_size ), nullptr, nullptr );
    return str;
#endif
}

std::string FromRHString( const rh::engine::String &t_str )
{
#ifndef UNICODE
    return t_str;
#else
    rh::engine::String str;
    std::size_t        res_size = WideCharToMultiByte(
        CP_ACP, 0, t_str.c_str(), t_str.size(), nullptr, 0, nullptr, nullptr );
    str.resize( res_size );
    WideCharToMultiByte( CP_ACP, 0, t_str.c_str(), t_str.size(), str.data(),
                         res_size, nullptr, nullptr );
    return str;
#endif
}

std::wstring FromRHString_( const rh::engine::String &t_str )
{
#ifdef UNICODE
    return t_str;
#else
    std::wstring str;

    const auto res_size =
        MultiByteToWideChar( CP_ACP, 0, t_str.c_str(),
                             static_cast<int>( t_str.size() ), nullptr, 0 );

    str.resize( static_cast<std::size_t>( res_size ) );

    MultiByteToWideChar( CP_ACP, 0, t_str.c_str(),
                         static_cast<int>( t_str.size() ), str.data(),
                         static_cast<int>( res_size ) );

    return str;
#endif
}

void DebugLogger::Init( const rh::engine::String &fileName,
                        LogLevel                  minLogLevel )
{
    m_sFileName   = fileName;
    m_MinLogLevel = minLogLevel;

    if ( !m_sFileName.empty() )
        m_pLogStream =
            std::make_unique<rh::engine::OutFileStream>( m_sFileName );
}

void DebugLogger::Log( const engine::String &msg, LogLevel logLevel )
{
    if ( logLevel < m_MinLogLevel )
        return;

    if ( m_pLogStream )
    {
        if ( !m_pLogStream->is_open() )
            m_pLogStream->open( m_sFileName,
                                std::fstream::out | std::fstream::app );

        *m_pLogStream << TEXT( "LOG: " ) << msg << TEXT( "\n" );

        m_pLogStream->close();
    }

    OutputDebugString( ( msg + TEXT( "\n" ) ).c_str() );
}

void DebugLogger::Error( const engine::String &msg )
{
    if ( m_pLogStream )
    {
        if ( !m_pLogStream->is_open() )
            m_pLogStream->open( m_sFileName,
                                std::fstream::out | std::fstream::app );

        *m_pLogStream << TEXT( "ERROR: " ) << msg << TEXT( "\n" );

        m_pLogStream->close();
    }

    OutputDebugString( ( msg + TEXT( "\n" ) ).c_str() );
}

void *DebugLogger::GetDebugFileHandle()
{
    if ( g_hDebugPipeHandle == nullptr )
    {
        SECURITY_ATTRIBUTES saAttr{};
        saAttr.nLength              = sizeof( SECURITY_ATTRIBUTES );
        saAttr.bInheritHandle       = TRUE;
        saAttr.lpSecurityDescriptor = nullptr;

        if ( !CreatePipe( &g_hDebugPipeReadHandle, &g_hDebugPipeHandle, &saAttr,
                          0 ) )
            return nullptr;
        SetHandleInformation( g_hDebugPipeReadHandle, HANDLE_FLAG_INHERIT, 0 );
    }
    return g_hDebugPipeHandle;
}
void DebugLogger::SyncDebugFile()
{
    DWORD dwRead;
    CHAR  chBuf[4096];
    BOOL  bSuccess = FALSE;
    CloseHandle( g_hDebugPipeHandle );
    g_hDebugPipeHandle = nullptr;
    for ( ;; )
    {
        bSuccess =
            ReadFile( g_hDebugPipeReadHandle, chBuf, 4096, &dwRead, nullptr );
        if ( !bSuccess || dwRead == 0 )
            break;
        DebugLogger::Log( chBuf );
    }
    CloseHandle( g_hDebugPipeReadHandle );
    g_hDebugPipeReadHandle = nullptr;
}
