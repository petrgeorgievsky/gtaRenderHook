//
// Created by peter on 17.08.2021.
//

#include "compatibility_checks.h"
#include "call_redirection_util.h"
#include <string_view>
// Windows msg
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <regex>

namespace
{
constexpr std::string_view gCompatibilityCheckFailure =
    "RenderHook: Compatibility Issue";

constexpr std::string_view gIncorrectVersionError =
    R"(Sorry, your version of GTA 3 executable is not supported by RenderHook!
Please use one of the following: 1.0, 1.1, Steam.)";

constexpr std::string_view gAsiLoaderInstallError =
    R"(Sorry, you have a problem with ASI Loader which will result in RenderHook crash later!
Please open the `scripts/global.ini` file and change the `LoadFromScriptsOnly` value to 0 and reload the game.)";
} // namespace

void ValidateStep( bool assertion, std::string_view failure_msg )
{
    if ( assertion )
        return;
    auto           current_window = GetActiveWindow();
    constexpr auto msg_box_flags  = MB_OK | MB_ICONERROR | MB_SETFOREGROUND;
    ShowWindow( current_window, SW_MINIMIZE );
    MessageBox( current_window, failure_msg.data(),
                gCompatibilityCheckFailure.data(), msg_box_flags );
    std::terminate();
}

/**
 * Checks for ASI Loader incompatibilities, returns true if found any
 *
 * @remark Incompatibility #1. With LoadFromScriptsOnly enabled in `Ultimate ASI
 * Loader`, ASI files load after engine initialization, which would not allow us
 * to inject RenderHook engine
 */
bool ValidateAsiLoader()
{
    constexpr auto load_from_scripts_key = "LoadFromScriptsOnly";
    const auto     global_ini_path =
        std::filesystem::current_path() / "scripts" / "global.ini";

    if ( !std::filesystem::exists( global_ini_path ) )
        return true;

    const std::regex ini_parser( "(^.*)=(.*[^=]*$)" );
    std::ifstream    global_ini( global_ini_path );
    for ( std::string line; std::getline( global_ini, line ); )
    {
        std::smatch base_match;
        if ( !std::regex_match( line, base_match, ini_parser ) )
            continue;

        std::string key = base_match.str( 1 ), value = base_match.str( 2 );
        if ( key == load_from_scripts_key )
            return ( value == "0" );
    }
    return true;
}

void PerformCompatChecks()
{
    /// Supported game version check
    ValidateStep( GetGameId() != Version_unknown, gIncorrectVersionError );
    /// ASI Loader compatibility check
    ValidateStep( ValidateAsiLoader(), gAsiLoaderInstallError );
}
