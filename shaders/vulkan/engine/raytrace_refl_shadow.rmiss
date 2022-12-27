#version 460
#extension GL_EXT_ray_tracing : require
struct ShadowHitPayload
{
    float hitDistance;
    float velocity;
    float x;
    float y;
};
layout(location = 1) rayPayloadInEXT ShadowHitPayload pld;

void main()
{
  pld.hitDistance = 1.0f;
  pld.velocity = 0.0f;
}