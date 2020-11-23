
// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "PBSMaterial.h"
#include "CDebug.h"
#include <filesystem>

RwTexDictionary *                               CPBSMaterialMgr::materialsTXD;
std::unordered_map<std::string, CPBSMaterial *> CPBSMaterialMgr::materials{};
bool has_suffix( const std::string &s, const std::string &suffix )
{
    return ( s.size() >= suffix.size() ) &&
           std::equal( suffix.rbegin(), suffix.rend(), s.rbegin() );
}
CPBSMaterial::CPBSMaterial( const std::string &fname ) : m_sName( fname )
{
    auto file = fopen(
        ( std::string( "materials\\" ) + m_sName + ".mat" ).c_str(), "rt" );
    char specFileName[80];
    char normalFileName[80];
    if ( file )
    {
        auto res = fscanf( file, "%79s\n", specFileName );
        if ( res == EOF )
        {
            fclose( file );
            return;
        }
        m_tSpecRoughness = _RwTexDictionaryFindNamedTexture(
            CPBSMaterialMgr::materialsTXD, specFileName );

        res = fscanf( file, "%79s\n", normalFileName );

        if ( res == EOF )
        {
            fclose( file );
            return;
        }
        m_tNormals = _RwTexDictionaryFindNamedTexture(
            CPBSMaterialMgr::materialsTXD, normalFileName );
        fclose( file );
    }
    // m_tSpecRoughness
}

void CPBSMaterialMgr::LoadMaterials()
{
    materialsTXD = CFileLoader::LoadTexDictionary( "materials\\materials.txd" );
    std::string     path( "materials/" );
    std::string     ext( ".mat" );
    std::error_code ec;
    if ( !std::filesystem::exists( path, ec ) )
    {
        g_pDebug->printMsg(
            "Materials folder not found, material rendering will be disabled.",
            0 );
        return;
    }
    
    auto dir_iter = std::filesystem::recursive_directory_iterator( path, ec );
    for ( auto &p : dir_iter )
    {
        if ( p.path().extension() == ext )
        {
            auto materialptr = new CPBSMaterial( p.path().stem().string() );
            materials[materialptr->m_sName] = materialptr;
        }
    }
}

void CPBSMaterialMgr::SetMaterial( const char *textureName ) {}
