//
// Created by peter on 06.01.2021.
//

#include "material_system_patches.h"
#include "../call_redirection_util.h"
#include "../game/TxdStore.h"
#include <MemoryInjectionUtils/InjectorHelpers.h>
#include <common_headers.h>
#include <material_storage.h>
#include <render_loop.h>

using namespace rh::rw::engine;

RwTexture *RwTextureRead( const char *name, const char *maskName )
{
    uint32_t address = GetAddressByGame( 0x5A7580, 0x5A7840, 0x5A8E00 );
    return InMemoryFuncCall<RwTexture *>( address, name, maskName );
}

RpMaterial *RpMaterialStreamReadImpl( void *stream )
{
    uint32_t address = GetAddressByGame( 0x5ADDA0, 0x5AE060, 0x5B0AA0 );
    return InMemoryFuncCall<RpMaterial *>( address, stream );
}

RpMaterial *RpMaterialStreamRead( void *stream )
{
    auto material = RpMaterialStreamReadImpl( stream );
    if ( material == nullptr )
        return nullptr;

    // Old RenderWare doesn't support "Always" callbacks, so we just hack it in
    // for GTA 3
    // TODO: Find a better solution?
    if ( BackendMaterialStreamAlwaysCallback(
             material, gBackendMaterialExtOffset,
             sizeof( BackendMaterialExt ) ) == FALSE )
        return nullptr;
    return material;
}

RwTexture *ReadTextureCallback( std::string_view dict, std::string_view name )
{
    auto txd_slot = TxdStore::FindTxdSlot( dict.data() );
    if ( txd_slot < 0 )
        return nullptr;
    TxdStore::PushCurrentTxd();

    TxdStore::SetCurrentTxd( txd_slot );
    auto texture = RwTextureRead( name.data(), nullptr );

    TxdStore::PopCurrentTxd();
    return texture;
}

void PatchMaterialSystem()
{
    // Register material system texture reading CB
    MaterialExtensionSystem::GetInstance().RegisterReadTextureCallback(
        ReadTextureCallback );
    RedirectCall( GetAddressByGame( 0x5C8E66, 0x5C9126, 0x5CFB1D ),
                  reinterpret_cast<void *>( RpMaterialStreamRead ) );
}
