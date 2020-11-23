struct hitPayload
{
    vec3 hitValue;
    int  depth;
    vec3 attenuation;
    int  done;
    vec3 rayOrigin;
    vec3 rayDir;
    vec3 materialParams;
    vec4 normalDepth;
};

struct ShadowHitPayload
{
    float hitDistance;
    float velocity;
    float x;
    float y;
};

struct PrimRaysPayload
{
    vec4 albedo;
    vec4 normalDepth;
    vec4 materialParams;
    vec4 motionVectors;
};

struct ReflectionRaysPayload
{
    vec4 reflection_color;
//vec4 refraction_coeff;
};

struct Vertex
{
    vec4 pos;
    vec4 uv;
    vec4 normals;
    vec4 local_motion;
    vec4 weights;
    uint indices;
    uint color;
    uint material;
    uint padd;
};


struct sceneDesc
{
    int  objId;
    int  txtOffset;
    int  align_a;
    int  align_b;
    mat4 transfo;
    mat4 transfoIT;
    mat4 prevTransfo;
};

struct MaterialDesc
{
    int  txd_id;
    uint color;
    int spec_id;
    float specular;
//float diffuse;
};