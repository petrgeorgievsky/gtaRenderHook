#include "VulkanShader.h"
#include "DebugUtils/DebugLogger.h"
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace rh::engine;

VulkanShader::VulkanShader( const VulkanShaderDesc &desc )
    : mDevice( desc.mDevice )
{
    std::string shader_type;
    switch ( desc.mDesc.mShaderStage )
    {
    case ShaderStage::Vertex: shader_type = "vert"; break;
    case ShaderStage::Pixel: shader_type = "frag"; break;
    case ShaderStage::RayGen: shader_type = "rgen"; break;
    case ShaderStage::RayMiss: shader_type = "rmiss"; break;
    case ShaderStage::RayHit: shader_type = "rchit"; break;
    case ShaderStage::RayAnyHit: shader_type = "rahit"; break;
    case ShaderStage::Compute: shader_type = "comp"; break;
    default: throw std::runtime_error( "not implemented" );
    }

    std::string spir_v_path = desc.mDesc.mShaderPath + "_" +
                              desc.mDesc.mEntryPoint + ".spirv." + shader_type;
    // Compile to SPIR-V
    // TODO: Allow precompiled shaders
    bool result = TranslateHLSL_to_SPIRV( desc.mDesc.mShaderPath, spir_v_path,
                                          shader_type, desc.mDesc.mEntryPoint );
    if ( !result )
        throw std::runtime_error( "failed to translate shader to spirv" );

    // Read SPIR-V in temp buffer
    // TODO: Provide a way to load compiled shaders or shader packages
    std::ifstream spirv_buff;

    char dir_path[4096];
    GetModuleFileNameA( nullptr, dir_path, 4096 );
    std::filesystem::path dir_path_ =
        std::filesystem::path( dir_path ).parent_path();
    auto f_path = dir_path_ / spir_v_path;

    spirv_buff.open( f_path.generic_string(), std::ios::binary );
    spirv_buff.seekg( 0, std::ios::end );
    std::size_t buff_size = static_cast<size_t>( spirv_buff.tellg() );
    spirv_buff.seekg( 0, std::ios::beg );
    std::vector<uint32_t> buffer;
    buffer.resize( buff_size / 4 + 1 );
    spirv_buff.read( reinterpret_cast<char *>( buffer.data() ), buff_size );

    // Create shader module
    vk::ShaderModuleCreateInfo sm_ci{};
    sm_ci.codeSize = buff_size;
    sm_ci.pCode    = buffer.data();
    mShaderImpl    = mDevice.createShaderModule( sm_ci );
}

VulkanShader::~VulkanShader()
{
    if ( mDevice )
        mDevice.destroyShaderModule( mShaderImpl );
}

bool rh::engine::TranslateHLSL_to_SPIRV( const std::string &path,
                                         const std::string &dest_path,
                                         const std::string &shader_type,
                                         const std::string &entry_point )
{

    char dir_path[4096];
    GetModuleFileNameA( nullptr, dir_path, 4096 );
    std::filesystem::path dir_path_ =
        std::filesystem::path( dir_path ).parent_path();

    STARTUPINFOA start_info{};
    start_info.cb         = sizeof( start_info );
    start_info.hStdOutput = debug::DebugLogger::GetDebugFileHandle();
    start_info.hStdError  = debug::DebugLogger::GetDebugFileHandle();
    start_info.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION proc_info{};
    std::stringstream   cmd_args;
    if ( std::filesystem::path( path ).extension() == ".hlsl" )
        cmd_args << " -D";

    cmd_args << " -e " << entry_point << " -S " << shader_type
             << " -V100 " // emit SPiR-V
                "-o "
             << dir_path_ / dest_path << " " << dir_path_ / path;

    auto cmd_args_str = cmd_args.str();
    char cmd_args_cstr[4096];
    strcpy_s( cmd_args_cstr, cmd_args_str.c_str() );

    debug::DebugLogger::Log(
        "Running glslangValidator with following arguments:" );
    debug::DebugLogger::Log( cmd_args_str );

    auto path_to_exe = dir_path_ / "glslangValidator.exe";
    bool result = CreateProcessA( path_to_exe.string().c_str(), cmd_args_cstr,
                                  nullptr, nullptr, true, CREATE_NO_WINDOW,
                                  nullptr, nullptr, &start_info, &proc_info );
    if ( !result )
    {
        debug::DebugLogger::Error( "Failed to run SPIR-V compiler" );
        auto error = GetLastError();
        std::cerr << ".\n" << error;
        return false;
    }

    // Wait until child process exits.
    WaitForSingleObject( proc_info.hProcess, INFINITE );
    debug::DebugLogger::SyncDebugFile();
    DWORD exit_code{};
    GetExitCodeProcess( proc_info.hProcess, &exit_code );
    // Close process and thread handles.
    CloseHandle( proc_info.hProcess );
    CloseHandle( proc_info.hThread );
    return exit_code == S_OK;
}