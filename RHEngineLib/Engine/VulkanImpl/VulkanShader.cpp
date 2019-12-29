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
    default: throw std::runtime_error( "not implemented" );
    }

    std::string spir_v_path = desc.mDesc.mShaderPath + ".spirv." + shader_type;
    // Compile to SPIR-V
    bool result = TranslateHLSL_to_SPIRV( desc.mDesc.mShaderPath, spir_v_path,
                                          shader_type, desc.mDesc.mEntryPoint );
    if ( !result )
        throw std::runtime_error( "failed to translate shader to spirv" );
    // Read SPIR-V in temp buffer
    std::ifstream spirv_buff;

    char dir_path[4096];
    GetModuleFileNameA( nullptr, dir_path, 4096 );
    std::filesystem::path dir_path_ =
        std::filesystem::path( dir_path ).parent_path();
    auto f_path = dir_path_ / spir_v_path;

    spirv_buff.open( f_path.generic_string(), std::ios::binary );
    spirv_buff.seekg( 0, std::ios::end );
    std::size_t buff_size = spirv_buff.tellg();
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
    start_info.cb = sizeof( start_info );
    PROCESS_INFORMATION proc_info{};
    std::stringstream   cmd_args;
    cmd_args << dir_path_ / "glslangValidator.exe"
             << " -D"
                " -e "
             << entry_point << " -S " << shader_type
             << " -V100 " // emit SPiR-V
                "-o "
             << dir_path_ / dest_path << " " << dir_path_ / path;

    auto cmd_args_str = cmd_args.str();
    char cmd_args_cstr[512];
    strcpy_s( cmd_args_cstr, cmd_args_str.c_str() );

    debug::DebugLogger::Log(
        "Running glslangValidator with following arguments:" );
    debug::DebugLogger::Log( cmd_args_str );

    bool result =
        CreateProcessA( nullptr, cmd_args_cstr, nullptr, nullptr, false, 0,
                        nullptr, nullptr, &start_info, &proc_info );
    if ( !result )
    {
        debug::DebugLogger::Error( "Failed to translate HLSL to SPIR-V" );
        std::cerr << ".\n" << GetLastError();
        return false;
    }

    // Wait until child process exits.
    WaitForSingleObject( proc_info.hProcess, INFINITE );

    // Close process and thread handles.
    CloseHandle( proc_info.hProcess );
    CloseHandle( proc_info.hThread );
    return true;
}