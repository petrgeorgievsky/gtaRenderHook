#pragma once
#include "CDebug.h"
/*!
    \brief D3D API specific wrapper.
    D3D API specific wrapper, prints out debug message and error code in debug file and throws runtime error.
 */
inline bool CALL_D3D_API( HRESULT callResult, const std::string& errorMessage )
{
    if ( FAILED( callResult ) )
    {
        MessageBox( nullptr, errorMessage.c_str(), "RUNTIME ERROR", 0 );
        g_pDebug->printMsg( errorMessage, 0 );
        // TODO: add more readable results(for ex. convert HRESULT to string, based on result description)
        g_pDebug->printError( "ERROR_CODE:" + std::to_string( callResult ) + "\n" );
        return false;
    }
    return true;
}

/*!
    \brief D3D API specific wrapper.
    D3D API specific wrapper, prints out debug message and error code in debug file.
*/
inline bool CALL_D3D_API_SILENT( HRESULT callResult, const std::string& errorMessage )
{
    if ( FAILED( callResult ) )
    {
        g_pDebug->printMsg( errorMessage, 0 );
        // TODO: add more readable results(for ex. convert HRESULT to string, based on result description)
        g_pDebug->printMsg( "ERROR_CODE:" + std::to_string( callResult ), 0 );
        MessageBox( nullptr, errorMessage.c_str(), "RUNTIME SILENT ERROR", 0 );
        return false;
    }
    return true;
}