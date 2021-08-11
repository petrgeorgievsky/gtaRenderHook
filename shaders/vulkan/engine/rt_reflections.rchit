#version 460
#extension GL_NV_ray_tracing : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_16bit_storage : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable
#include "raycommon.glsl"

layout(location = 0) rayPayloadInNV ReflectionRaysPayload pay_load;
layout(location = 1) rayPayloadNV ShadowHitPayload shad_pld;

layout(binding = 0, set = 0) uniform accelerationStructureNV topLevelAS;

hitAttributeNV vec3 attribs;

// Prim rays pass descriptors
layout(binding = 3, set = 0) uniform sampler baseSampler;
layout(binding = 4, set = 0) uniform SkyCfg
{
    vec4 sunDir;
    vec4 sunColor;
    vec4 horizonColor;
    vec4 skyColor;
}sky_cfg;

layout(binding = 0, set = 1) uniform CameraProperties
{
    mat4 view;
    mat4 proj;
    mat4 viewInverse;
    mat4 projInverse;
    mat4 viewInvPrev;
    mat4 viewPrev;
} cam;

// Scene descriptors
// TODO: Move to include
layout(binding = 0, set = 2, scalar) buffer ScnDesc{ sceneDesc i[]; } scnDesc;
layout(binding = 1, set = 2, scalar) buffer Vertices { Vertex v[]; } vertices[];
layout(binding = 2, set = 2) buffer Indices { uint16_t i[]; } indices[];
layout(binding = 3, set = 2) uniform texture2D textures[];
layout(binding = 4, set = 2, scalar) buffer MatDesc{ MaterialDesc i[]; } matDesc;

void main()
{
    int obj_id = scnDesc.i[gl_InstanceID].objId;
    // Indices of the triangle
    ivec3 ind = ivec3(
    indices[obj_id].i[3 * gl_PrimitiveID + 0],
    indices[obj_id].i[3 * gl_PrimitiveID + 1],
    indices[obj_id].i[3 * gl_PrimitiveID + 2]
    );
    // Vertex of the triangle
    Vertex v0 = vertices[obj_id].v[ind.x];
    Vertex v1 = vertices[obj_id].v[ind.y];
    Vertex v2 = vertices[obj_id].v[ind.z];

    MaterialDesc material = matDesc.i[scnDesc.i[gl_InstanceID].txtOffset + v0.material];

    const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

    vec4 color = unpackUnorm4x8(v0.color) * barycentrics.x +
    unpackUnorm4x8(v1.color) * barycentrics.y +
    unpackUnorm4x8(v2.color) * barycentrics.z;

    vec2 tc = v0.uv.xy * barycentrics.x +
    v1.uv.xy * barycentrics.y +
    v2.uv.xy * barycentrics.z;

    // Computing the normal at hit position
    vec3 normal = v0.normals.xyz * barycentrics.x +
    v1.normals.xyz * barycentrics.y +
    v2.normals.xyz * barycentrics.z;
    vec3 obj_pos = v0.pos.xyz * barycentrics.x +
    v1.pos.xyz * barycentrics.y +
    v2.pos.xyz * barycentrics.z;

    vec3 local_motion_ = (v0.local_motion.xyz) * barycentrics.x +
    (v1.local_motion.xyz) * barycentrics.y +
    (v2.local_motion.xyz) * barycentrics.z;
    vec3 prev_obj_pos = obj_pos - local_motion_;
    // Transforming the normal to world space
    normal = normalize(vec3(scnDesc.i[gl_InstanceID].transfoIT * vec4(normal, 0.0)));

    vec3 sun_dir = sky_cfg.sunDir.xyz;
    float ndotl = max(dot(sun_dir, normal), 0.0f);
    if (ndotl > 0)
    {
        shad_pld.hitDistance = 0.0f;
        shad_pld.velocity = 0.0f;
        shad_pld.x = 0.0f;
        shad_pld.y = 0.0f;

        float tMin   = 0.1;
        float tMax   = 1000.0;
        vec3  origin = vec3(vec4(obj_pos, 1.0) * scnDesc.i[gl_InstanceID].transfo);
        vec3  rayDir = sun_dir;
        uint  flags =
        gl_RayFlagsSkipClosestHitShaderNV;
        traceNV(topLevelAS, // acceleration structure
        flags, // rayFlags
        0xFF, // cullMask
        0, // sbtRecordOffset
        0, // sbtRecordStride
        1, // missIndex
        origin, // ray origin
        tMin, // ray min range
        rayDir, // ray direction
        tMax, // ray max range
        1// payload (location = 1)
        );
        ndotl *= (shad_pld.hitDistance > 0 ? 1.0f: 0.0f);
    }
    float intensity_coeff = min((v0.emission + v1.emission + v2.emission)/3, 2.0f);
    vec3 lighting = vec3(ndotl) + sky_cfg.skyColor.rgb * 0.3f;
    if (material.txd_id >= 0) {
        vec4 tex_color = texture(sampler2D(textures[material.txd_id], baseSampler), tc);
        pay_load.reflection_color = tex_color * unpackUnorm4x8(material.color);
    }
    else
        pay_load.reflection_color = vec4(unpackUnorm4x8(material.color));
    pay_load.reflection_color.rgb *= (lighting + vec3(intensity_coeff));
}
