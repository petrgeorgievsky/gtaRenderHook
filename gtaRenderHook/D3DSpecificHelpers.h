#pragma once
#include "CDebug.h"
/*! \brief D3D API specific wrapper.
 * 
 *  D3D API specific wrapper, prints out debug message and error code in debug file.
 */
inline bool CALL_D3D_API(HRESULT callResult, const std::string& errorMessage) {
	if (FAILED(callResult)) {
		g_pDebug->printError(errorMessage);
		// TODO: add more readable results(for ex. convert HRESULT to string, based on result description)
		g_pDebug->printError("ERROR_CODE:"+std::to_string(callResult));
		return false;
	}
	return true;
}