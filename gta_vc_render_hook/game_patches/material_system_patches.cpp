//
// Created by peter on 23.02.2021.
//
//
// Created by peter on 06.01.2021.
//

#include "material_system_patches.h"
#include "../game/TxdStore.h"
#include <common_headers.h>
#include <injection_utils/InjectorHelpers.h>
#include <material_storage.h>
#include <rw_engine/rh_backend/material_backend.h>

using namespace rh::rw::engine;

RwTexture *RwTextureRead( const char *name, const char *maskName )
{
    return InMemoryFuncCall<RwTexture *>( 0x64E110, name, maskName );
}

RpMaterial *RpMaterialStreamReadImpl( void *stream )
{
    return InMemoryFuncCall<RpMaterial *>( 0x655920, stream );
}

RpMaterial *RpMaterialStreamRead( void *stream )
{
    auto material = RpMaterialStreamReadImpl( stream );
    if ( material == nullptr )
        return nullptr;

    // Old RenderWare doesn't support "Always" callbacks, so we just hack it in
    // for GTA VC
    if ( BackendMaterialPlugin::CallAlwaysCb( material ) == FALSE )
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
    RedirectCall( 0x66DD06, reinterpret_cast<void *>( RpMaterialStreamRead ) );
}
