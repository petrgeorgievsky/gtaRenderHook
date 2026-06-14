//
// Created by peter on 01.12.2020.
//

#include "Win32UncaughtExceptionHandler.h"
#include "DebugLogger.h"
#include <common.h>

#include <Psapi.h>
#include <dbghelp.h>
#include <eh.h>

LPTOP_LEVEL_EXCEPTION_FILTER gOldExceptionHandler = nullptr;

constexpr size_t MAX_STACK_FRAMES = 32;

/**
 * Tries to write uncaught exceptions to log files
 */
LONG WINAPI Win32UncaughtExceptionFilter( _EXCEPTION_POINTERS *ExceptionInfo )
{
    rh::debug::DebugLogger::Error( "Uncaught exception:" );
    char            buf[sizeof( SYMBOL_INFO ) + 1024]{};
    char            module_name_last[512]{};
    char            module_name[512]{};
    void           *stack[MAX_STACK_FRAMES];
    constexpr DWORD FRAMES_TO_SKIP = 7;
    unsigned int    frames         = CaptureStackBackTrace(
        FRAMES_TO_SKIP, MAX_STACK_FRAMES, stack, nullptr );

    rh::debug::DebugLogger::Error( "--- Call Stack Trace ---" );
    auto p = GetCurrentProcess();
    for ( unsigned int i = 0; i < frames; ++i )
    {
        DWORD64      address = reinterpret_cast<DWORD64>( stack[i] );
        SYMBOL_INFO *symbol  = reinterpret_cast<SYMBOL_INFO *>( buf );
        symbol->SizeOfStruct = sizeof( SYMBOL_INFO );
        symbol->MaxNameLen   = 1024;
        DWORD64 displacement = 0;

        HMODULE hmod;
        if ( GetModuleHandleExA( GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                                 reinterpret_cast<char const *>( address ),
                                 &hmod ) != 0 )
        {
            auto size         = GetModuleFileNameA( hmod, module_name,
                                                    sizeof( module_name ) - 1 );
            module_name[size] = 0;
            if ( std::string_view{ module_name, size } !=
                 std::string_view{ module_name_last } )
            {
                std::memcpy( module_name_last, module_name, size );
                rh::debug::DebugLogger::ErrorFmt( "m: %s", module_name );
            }
        }
        ULONG_PTR rva = (ULONG_PTR)stack[i] - (ULONG_PTR)hmod;
        if ( SymFromAddr( p, address, &displacement, symbol ) )
        {
            rh::debug::DebugLogger::ErrorFmt( "%u, F:%s+%lu, A:0x%p, RVA:0x%p",
                                              i, symbol->Name, displacement,
                                              address, rva );
            IMAGEHLP_LINE line;
            line.SizeOfStruct = sizeof( IMAGEHLP_LINE );
            DWORD d           = 0;
            if ( SymGetLineFromAddr( p, address, &d, &line ) )
            {
                rh::debug::DebugLogger::ErrorFmt( "f:%s:%u", line.FileName,
                                                  line.LineNumber );
            }
        }
        else
        {
            rh::debug::DebugLogger::ErrorFmt( "%u, A:0x%p, RVA:0x%p", i,
                                              address, rva );
        }
    }

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
            static std::array<char, MAX_PATH> fn{};
            GetModuleFileNameExA( GetCurrentProcess(), hm, fn.data(),
                                  static_cast<DWORD>( fn.size() ) );
            rh::debug::DebugLogger::ErrorFmt( "ModulePath:%s;ModuleBase:0x%X",
                                              fn.data(), mi.lpBaseOfDll );
        }
    }
    SymCleanup( GetCurrentProcess() );

    if ( gOldExceptionHandler )
        return gOldExceptionHandler( ExceptionInfo );
    return EXCEPTION_EXECUTE_HANDLER;
}

void rh::debug::InitExceptionHandler()
{
    SymInitialize( GetCurrentProcess(), nullptr, TRUE );
    gOldExceptionHandler =
        SetUnhandledExceptionFilter( Win32UncaughtExceptionFilter );
}
