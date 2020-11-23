#pragma once
#include "DebugUtils/DebugLogger.h"
#include "Engine/Common/types/string_typedefs.h"
//#include <comdef.h>
#include <sstream>

/**
    @brief D3D API specific wrapper.
    D3D API specific wrapper, prints out debug message and error code in debug
   file and throws runtime error.
*/
inline bool CALL_D3D_API( long                     callResult,
                          const rh::engine::String &errorMessage )
{
    if ( callResult < 0  )
    {
        rh::engine::StringStream ss;

        rh::debug::DebugLogger::Error( errorMessage );

        //_com_error err( callResult );
        //ss << TEXT( "Error Desc:" ) << err.ErrorMessage() << std::endl;

       // MessageBox( nullptr, ss.str().c_str(), errorMessage.c_str(), 0 );

        rh::debug::DebugLogger::Log( ss.str(), rh::debug::LogLevel::Error );

        return false;
    }

    return true;
}

/**
    @brief D3D API specific wrapper.
    D3D API specific wrapper, prints out debug message and error code in debug
   file.
*/
inline bool CALL_D3D_API_SILENT( long                    callResult,
                                 const rh::engine::String &errorMessage )
{
    if (  callResult < 0 )
    {
        rh::engine::StringStream ss;

        rh::debug::DebugLogger::Log( errorMessage, rh::debug::LogLevel::Error );

        //_com_error err( callResult );
        //ss << TEXT( "ERROR_DESC:" ) << err.ErrorMessage() << std::endl;

        rh::debug::DebugLogger::Log( ss.str(), rh::debug::LogLevel::Error );

       // MessageBox( nullptr, errorMessage.c_str(),
        //            TEXT( "RUNTIME SILENT ERROR" ), 0 );

        return false;
    }

    return true;
}
