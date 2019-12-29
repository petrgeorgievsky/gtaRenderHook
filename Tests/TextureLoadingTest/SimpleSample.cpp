#include "SimpleSample.h"
#include <Engine\D3D11Impl\ImageBuffers\D3D11Texture2D.h>
#include <filesystem>
#include <rw_engine/rw_camera/rw_camera.h>
#include <rw_engine/rw_im2d/rw_im2d.h>
#include <rw_engine/rw_macro_constexpr.h>
#include <rw_engine/rw_tex_dict/rw_tex_dict.h>
#include <rw_engine/rw_texture/rw_texture.h>

bool SimpleSample::CustomInitialize()
{
    if ( !rh::rw::engine::RwTestSample::CustomInitialize() )
        return false;
    namespace fs = std::filesystem;
    for ( auto &p : fs::directory_iterator( "textures" ) ) {
        fs::path file_path = p.path();
        if ( file_path.extension() == ".txd" || file_path.extension() == ".TXD" ) {
            m_vTexDictList.push_back( file_path.generic_string() );
        }
    }

    if ( m_vTexDictList.size() > m_nTXDId )
        LoadTXD( m_vTexDictList[m_nTXDId] );

    return true;
}

void SimpleSample::GenerateQuad( float w, float h )
{
    m_vQuad.clear();

    RwIm2DVertex vtx{};
    w = fmin( fmax( w, 8.0f ), 1024.0f );
    h = fmin( fmax( h, 8.0f ), 1024.0f );
    vtx.x = vtx.y = vtx.u = vtx.v = 0;
    vtx.emissiveColor = 0xFFFFFFFF;
    m_vQuad.push_back( vtx );
    vtx.x = w;
    vtx.y = h;
    vtx.u = vtx.v = 1;
    m_vQuad.push_back( vtx );
    vtx.x = vtx.u = 0;
    vtx.y = h;
    vtx.v = 1;
    m_vQuad.push_back( vtx );
    vtx.x = vtx.y = vtx.u = vtx.v = 0;
    m_vQuad.push_back( vtx );
    vtx.x = w;
    vtx.y = vtx.v = 0;
    vtx.u = 1;
    m_vQuad.push_back( vtx );
    vtx.x = w;
    vtx.y = h;
    vtx.u = vtx.v = 1;
    m_vQuad.push_back( vtx );
}

void SimpleSample::CustomShutdown()
{
    if ( m_currentTexDict ) {
        m_vTextureList.clear();
        rh::rw::engine::RwTexDictionaryDestroy( m_currentTexDict );
        m_currentTexDict = nullptr;
    }
    TestSample::CustomShutdown();
}

void SimpleSample::CustomRender()
{
    RwRGBA clearColor = {128, 128, 255, 255};

    rh::rw::engine::RwCameraClear( m_pMainCamera, &clearColor, rwCAMERACLEARIMAGE );

    if ( m_vTextureList.size() > m_nTexId ) {
        RwTexture *tex = m_vTextureList[m_nTexId];
        RwRaster *raster = tex->raster;
        GenerateQuad( raster->width, raster->height );

        rh::rw::engine::g_pRwRenderEngine->RenderStateSet( rwRENDERSTATETEXTURERASTER,
                                                         reinterpret_cast<uint32_t>( raster ) );

        rh::rw::engine::RwIm2DRenderPrimitive( RwPrimitiveType::rwPRIMTYPETRILIST,
                                             m_vQuad.data(),
                                             static_cast<int32_t>( m_vQuad.size() ) );
    }
}

void SimpleSample::CustomUpdate( float )
{
    if ( GetKeyState( VK_LEFT ) & 0x8000 ) {
        if ( button_toggled[0] ) {
            int prevId = static_cast<int>( m_nTexId ) - 1;
            if ( prevId < 0 )
                prevId += m_vTextureList.size();
            m_nTexId = static_cast<uint32_t>( prevId );
            button_toggled[0] = false;
        }
    } else {
        button_toggled[0] = true;
    }
    if ( GetKeyState( VK_RIGHT ) & 0x8000 ) {
        if ( button_toggled[1] ) {
            m_nTexId = !m_vTextureList.empty() ? ( m_nTexId + 1 ) % m_vTextureList.size() : 0;
            button_toggled[1] = false;
        }
    } else {
        button_toggled[1] = true;
    }

    if ( GetKeyState( VK_UP ) & 0x8000 ) {
        if ( button_toggled[2] ) {
            m_nTXDId = ( m_nTXDId + 1 ) % m_vTexDictList.size();
            LoadTXD( m_vTexDictList[m_nTXDId] );
            button_toggled[2] = false;
        }
    } else {
        button_toggled[2] = true;
    }

    if ( GetKeyState( VK_DOWN ) & 0x8000 ) {
        if ( button_toggled[3] ) {
            m_nTXDId = ( m_nTXDId - 1 ) % m_vTexDictList.size();
            LoadTXD( m_vTexDictList[m_nTXDId] );
            button_toggled[3] = false;
        }
    } else {
        button_toggled[3] = true;
    }
}

void SimpleSample::LoadTXD( const rh::engine::String &path )
{
    if ( m_currentTexDict ) {
        m_vTextureList.clear();
        rh::rw::engine::RwTexDictionaryDestroy( m_currentTexDict );
        m_currentTexDict = nullptr;
    }

    m_currentTexDict = rh::rw::engine::GTAReadTexDict( path );
    /*rw_rh_engine::RwTexDictionaryForAllTextures( m_currentTexDict,
                                 [this](RwTexture*
tex,void*)->RwTexture*
                                 {
                                     m_vTextureList.push_back( tex
); return tex;
                                 },
                                 nullptr );*/

    {
        RwTexture *result;
        RwLLLink *cur, *end;
        cur = rh::rw::engine::rwLinkList::GetFirstLLLink( &m_currentTexDict->texturesInDict );
        end = rh::rw::engine::rwLinkList::GetTerminator( &m_currentTexDict->texturesInDict );

        while ( cur != end ) {
            result = rh::rw::engine::rwLLLink::GetData<RwTexture>( cur,
                                                                 offsetof( RwTexture,
                                                                           lInDictionary ) );

            m_vTextureList.push_back( result );

            cur = rh::rw::engine::rwLLLink::GetNext( cur );
        }
    }
    m_nTexId = 0;
}
