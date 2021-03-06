#version 460
#extension GL_NV_ray_tracing : require
struct ShadowHitPayload
{
    float hitDistance;
    float velocity;
    float x;
    float y;
};
layout(location = 0) rayPayloadInNV ShadowHitPayload pld;

void main()
{
  pld.hitDistance = 1.0f;
  pld.velocity = 0.0f;
}