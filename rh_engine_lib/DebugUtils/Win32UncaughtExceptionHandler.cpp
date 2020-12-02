//
// Created by peter on 01.12.2020.
//

#include "Win32UncaughtExceptionHandler.h"
#include "DebugLogger.h"
#include <common.h>

#include <Psapi.h>
#include <eh.h>
/**
 * Tries to write uncaught exceptions to log files
 */
LONG WINAPI Win32UncaughtExceptionFilter( _EXCEPTION_POINTERS *ExceptionInfo )
{
    rh::debug::DebugLogger::Error( "Uncaught exception:" );

    if ( ExceptionInfo && ExceptionInfo->ExceptionRecord )
    {
        auto    ex_rec = ExceptionInfo->ExceptionRecord;
        HMODULE hm;

        rh::debug::DebugLogger::ErrorFmt(
            "Address:0x%X; Code:0x%X; NumParams:%u", ex_rec->ExceptionAddress,
            ex_rec->ExceptionCode, ex_rec->NumberParameters );
        if ( GetModuleHandleEx(
                 GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                 static_cast<LPCTSTR>( ex_rec->ExceptionAddress ), &hm ) )
        {
            MODULEINFO mi{};
            GetModuleInformation( GetCurrentProcess(), hm, &mi, sizeof( mi ) );
            std::array<char, MAX_PATH> fn;
            GetModuleFileNameExA( GetCurrentProcess(), hm, fn.data(),
                                  fn.size() );
            rh::debug::DebugLogger::ErrorFmt( "ModulePath:%s;ModuleBase:0x%X",
                                              fn.data(), mi.lpBaseOfDll );
        }
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

void rh::debug::InitExceptionHandler()
{
    SetUnhandledExceptionFilter( Win32UncaughtExceptionFilter );
}
