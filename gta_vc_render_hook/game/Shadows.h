//
// Created by peter on 29.11.2020.
//
#pragma once
#include <common_headers.h>
class Entity;
class Vector;

/// Original game static shadows functions, we remove them in favor of new
/// lighting system TODO: Allow to use draw blob shadows, via config
class Shadows
{
  public:
    static void StoreShadowForPedObject( Entity *, float, float, float, float,
                                         float, float );
    static void StoreShadowForPole( Entity *, float, float, float, float, float,
                                    uint32_t );
    static void StoreCarLightShadow( Entity *, int32_t, RwTexture *, Vector *,
                                     float, float, float, float, uint8_t,
                                     uint8_t, uint8_t, float );

    static void StoreStaticShadow( uint32_t, uint8_t, RwTexture *, Vector *,
                                   float, float, float, float, int16_t, uint8_t,
                                   uint8_t, uint8_t, float, float, float, bool,
                                   float );
    static void StoreShadowForCar( Entity * );
    static bool IsCutsceneShadowEnabled();

    static void Patch();
};