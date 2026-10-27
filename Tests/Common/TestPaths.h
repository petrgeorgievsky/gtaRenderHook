#pragma once
#include <Windows.h>
#include <filesystem>
#include <string>

namespace rh::tests
{

// Directory of the executable of the current test.
// The engine resolves shaders/ and glslangValidator.exe relative to it.
inline std::filesystem::path ModuleDir()
{
    char path[MAX_PATH]{};
    ::GetModuleFileNameA( nullptr, path, MAX_PATH );
    return std::filesystem::path( path ).parent_path();
}

inline std::string ModuleRelative( const std::filesystem::path &absolute )
{
    return std::filesystem::relative( absolute, ModuleDir() )
        .generic_string();
}

} // namespace rh::tests
