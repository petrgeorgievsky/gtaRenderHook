//
// Created by peter on 07.08.2021.
//

#include "material_system_patches.h"
#include "../game/TxdStore.h"
#include <common_headers.h>
#include <injection_utils/InjectorHelpers.h>
#include <material_storage.h>
#include <rw_engine/rh_backend/material_backend.h>

using namespace rh::rw::engine;

RwTexture *RwTextureRead( const char *name, const char *mask_name )
{
    return InMemoryFuncCall<RwTexture *>( 0x7F3AC0, name, mask_name );
}

RpMaterial *RpMaterialStreamReadImpl( void *stream )
{
    return InMemoryFuncCall<RpMaterial *>( 0x74DD30, stream );
}

RpMaterial *RpMaterialStreamRead( void *stream )
{
    auto material = RpMaterialStreamReadImpl( stream );
    if ( material == nullptr )
        return nullptr;
    // Old RenderWare doesn't support "Always" callbacks, so we just hack it in
    // for GTA SA
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
    RedirectCall( 0x74E7F0, reinterpret_cast<void *>( RpMaterialStreamRead ) );
}
