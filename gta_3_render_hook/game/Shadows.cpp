//
// Created by peter on 29.11.2020.
//

#include "Shadows.h"
#include <MemoryInjectionUtils/InjectorHelpers.h>

void Shadows::Patch()
{
    RedirectJump( 0x513EC0, reinterpret_cast<void *>(
                                Shadows::StoreShadowForPedObject ) );
    RedirectJump( 0x514020,
                  reinterpret_cast<void *>( Shadows::StoreShadowForPole ) );
    RedirectJump( 0x513C80,
                  reinterpret_cast<void *>( Shadows::StoreCarLightShadow ) );
    RedirectJump( 0x5132B0,
                  reinterpret_cast<void *>( Shadows::StoreStaticShadow ) );
    RedirectJump( 0x513A40,
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
