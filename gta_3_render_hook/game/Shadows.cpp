//
// Created by peter on 29.11.2020.
//

#include "Shadows.h"
#include "../call_redirection_util.h"
#include <MemoryInjectionUtils/InjectorHelpers.h>

void Shadows::Patch()
{
    RedirectJump(
        GetAddressByGame( 0x513CB0, 0x513EC0, 0x513E50 ),
        reinterpret_cast<void *>( Shadows::StoreShadowForPedObject ) );
    RedirectJump( GetAddressByGame( 0x513E10, 0x514020, 0x513FB0 ),
                  reinterpret_cast<void *>( Shadows::StoreShadowForPole ) );
    RedirectJump( GetAddressByGame( 0x513A70, 0x513C80, 0x513C10 ),
                  reinterpret_cast<void *>( Shadows::StoreCarLightShadow ) );
    RedirectJump( GetAddressByGame( 0x5130A0, 0x5132B0, 0x513240 ),
                  reinterpret_cast<void *>( Shadows::StoreStaticShadow ) );
    RedirectJump( GetAddressByGame( 0x513830, 0x513A40, 0x5139D0 ),
                  reinterpret_cast<void *>( Shadows::StoreShadowForCar ) );
}

void Shadows::StoreShadowForCar( Entity * ) {}
void Shadows::StoreShadowForPedObject( Entity *, float, float, float, float,
                                       float, float )
{
}
void Shadows::StoreShadowForPole( Entity *, float, float, float, float, float,
                                  uint32_t )
{
}
void Shadows::StoreCarLightShadow( Entity *, int32_t, RwTexture *, Vector *,
                                   float, float, float, float, uint8_t, uint8_t,
                                   uint8_t, float )
{
}
void Shadows::StoreStaticShadow( uint32_t, uint8_t, RwTexture *, Vector *,
                                 float, float, float, float, int16_t, uint8_t,
                                 uint8_t, uint8_t, float, float, float, bool,
                                 float )
{
}
