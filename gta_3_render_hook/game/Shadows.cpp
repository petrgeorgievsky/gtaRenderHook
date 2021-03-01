//
// Created by peter on 29.11.2020.
//

#include "Shadows.h"
#include "../call_redirection_util.h"
#include "Entity.h"
#include "ModelInfo.h"
#include "PointLights.h"
#include <injection_utils/InjectorHelpers.h>

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

Vector operator*( const Matrix &mat, const Vector &vec )
{
    return Vector( mat.m_matrix.right.x * vec.x + mat.m_matrix.up.x * vec.y +
                       mat.m_matrix.at.x * vec.z + mat.m_matrix.pos.x,
                   mat.m_matrix.right.y * vec.x + mat.m_matrix.up.y * vec.y +
                       mat.m_matrix.at.y * vec.z + mat.m_matrix.pos.y,
                   mat.m_matrix.right.z * vec.x + mat.m_matrix.up.z * vec.y +
                       mat.m_matrix.at.z * vec.z + mat.m_matrix.pos.z );
}

void Shadows::StoreShadowForCar( Entity *e )
{
    struct fColor
    {
        float r, g, b;
    };

    auto *automobile = static_cast<Automobile *>( e );
    auto *mi = (VehicleModelInfo *)ModelInfo::GetModelInfo( e->mModelIndex );

    if ( automobile->EngineOn == 0 || automobile->LightsOn == 0 )
        return;

    // Generate "proper" symmetrical head/tail lights for cars instead of
    // generating blob shadows

    constexpr float  headlights_radius = 40.0f;
    constexpr fColor headlights_color  = { 1.0f, 1.0f, 1.0f };

    Vector hl_center = mi->mPositions[CarGizmoPosIds::HeadLights];
    Vector tl_center = mi->mPositions[CarGizmoPosIds::TailLights];

    // Compute right head/tail lights world pos
    Vector hl_center_wpos = e->mMatrix * hl_center;
    Vector tl_center_wpos = e->mMatrix * tl_center;

    // Compute offsets for head/tail lights
    Vector right_hl_offset = e->GetRight() * 2.0f * hl_center.x;
    Vector right_tl_offset = e->GetRight() * 2.0f * tl_center.x;

    Vector right_hl_wpos = hl_center_wpos;
    Vector left_hl_wpos  = hl_center_wpos - right_hl_offset;

    Vector right_tl_wpos = tl_center_wpos;
    Vector left_tl_wpos  = tl_center_wpos - right_tl_offset;

    auto generate_car_light = []( Automobile *a, eLights light_id, Vector pos,
                                  Vector dir, fColor color, float radius,
                                  char type ) {
        if ( a->Damage.GetLightStatus( light_id ) == 0 )
            PointLights::AddLight( type, pos.x, pos.y, pos.z, dir.x, dir.y,
                                   dir.z, radius, color.r, color.g, color.b, 0,
                                   false );
    };

    generate_car_light( automobile, VEHLIGHT_FRONT_RIGHT, right_hl_wpos,
                        e->GetForward(), headlights_color, headlights_radius,
                        1 );
    generate_car_light( automobile, VEHLIGHT_FRONT_LEFT, left_hl_wpos,
                        e->GetForward(), headlights_color, headlights_radius,
                        1 );

    generate_car_light( automobile, VEHLIGHT_REAR_RIGHT, right_tl_wpos,
                        { 0.0f, 0.0f, 0.0f }, { 0.6f, 0.0f, 0.0f },
                        automobile->mBrakePedal > 0.0f ? 10.0f : 7.0f, 0 );
    generate_car_light( automobile, VEHLIGHT_REAR_LEFT, left_tl_wpos,
                        { 0.0f, 0.0f, 0.0f }, { 0.6f, 0.0f, 0.0f },
                        automobile->mBrakePedal > 0.0f ? 10.0f : 7.0f, 0 );
}

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
