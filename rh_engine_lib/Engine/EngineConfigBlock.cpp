//
// Created by peter on 13.06.2020.
//

#include "EngineConfigBlock.h"
#include "../ConfigUtils/ConfigurationManager.h"
#include "../ConfigUtils/Serializable.h"
#include "../DebugUtils/DebugLogger.h"
#include "Definitions.h"
#include <cassert>

namespace rh::engine
{

EngineConfigBlock EngineConfigBlock::It{};
EngineConfigBlock::EngineConfigBlock() noexcept
{
    Reset();
    ConfigurationManager::Instance().AddConfigBlock(
        static_cast<ConfigBlock *>( this ) );
}
void EngineConfigBlock::Serialize( Serializable *serializable )
{
    assert( serializable != nullptr );

    serializable->Set<bool>( "IsWindowed", IsWindowed );
    serializable->Set<uint32_t>( "SharedMemorySizeMB", SharedMemorySizeMB );
    serializable->Set<uint32_t>( "RenderingAPI", RenderingAPI_id );
    serializable->Set<uint32_t>( "RendererWidth", RendererWidth );
    serializable->Set<uint32_t>( "RendererHeight", RendererHeight );
}
void EngineConfigBlock::Deserialize( Serializable *serializable )
{
    assert( serializable != nullptr );
    // TODO: Add better handling of such errors
    // try
    //{
    IsWindowed         = serializable->Get<bool>( "IsWindowed" );
    SharedMemorySizeMB = serializable->Get<uint32_t>( "SharedMemorySizeMB" );
    RenderingAPI_id    = serializable->Get<uint32_t>( "RenderingAPI" );
    RendererWidth      = serializable->Get<uint32_t>( "RendererWidth" );
    RendererHeight     = serializable->Get<uint32_t>( "RendererHeight" );
    //}
    /*catch ( const std::exception &ex )
    {
        rh::debug::DebugLogger::ErrorFmt(
            "Failed to deserialize engine config block, resetting: %s",
            ex.what() );
        Reset();
    }*/
}
void EngineConfigBlock::Reset()
{
    IsWindowed         = gDebugEnabled;
    RendererWidth      = 1920;
    RendererHeight     = 1080;
    SharedMemorySizeMB = 32;
    RenderingAPI_id    = static_cast<uint32_t>( RenderingAPI::DX11 );
}
} // namespace rh::engine