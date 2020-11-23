//
// Created by peter on 13.06.2020.
//

#include "EngineConfigBlock.h"
#include "Definitions.h"
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
}
void EngineConfigBlock::Deserialize( Serializable *serializable )
{
    assert( serializable != nullptr );
    IsWindowed         = serializable->Get<bool>( "IsWindowed" );
    SharedMemorySizeMB = serializable->Get<uint32_t>( "SharedMemorySizeMB" );
    RenderingAPI_id    = serializable->Get<uint32_t>( "RenderingAPI" );
}
void EngineConfigBlock::Reset()
{
#ifdef _DEBUG
    IsWindowed = true;
#else
    IsWindowed = false;
#endif
    SharedMemorySizeMB = 32;
    RenderingAPI_id    = static_cast<uint32_t>( RenderingAPI::DX11 );
}
} // namespace rh::engine