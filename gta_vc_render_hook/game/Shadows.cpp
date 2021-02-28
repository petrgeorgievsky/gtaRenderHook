//
// Created by peter on 29.11.2020.
//

#include "Shadows.h"

#include <injection_utils/InjectorHelpers.h>

void Shadows::Patch()
{
    // Buggy cutscene shadows
    RedirectJump( 0x625D80,
                  reinterpret_cast<void *>( IsCutsceneShadowEnabled ) );

    RedirectJump( 0x56DC70, reinterpret_cast<void *>(
                                Shadows::StoreShadowForPedObject ) );
    RedirectJump( 0x56D410,
                  reinterpret_cast<void *>( Shadows::StoreShadowForPole ) );
    RedirectJump( 0x56DCD0,
                  reinterpret_cast<void *>( Shadows::StoreCarLightShadow ) );
    RedirectJump( 0x56E780,
                  reinterpret_cast<void *>( Shadows::StoreStaticShadow ) );
    RedirectJump( 0x56DFA0,
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
bool Shadows::IsCutsceneShadowEnabled() { return false; }
