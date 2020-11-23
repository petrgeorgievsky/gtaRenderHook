//--------------------------------------------------------------------------------------
// Bindings
//--------------------------------------------------------------------------------------

#ifdef D3D11_API
#define BINDING( SET, BIND_ID )
#else
#define BINDING( SET, BIND_ID ) [[vk::binding( BIND_ID, SET )]]
#endif
BINDING( 0, 0 )
cbuffer CameraBuffer : register( b0 )
{
    float4x4 view;
    float4x4 proj;
    float4x4 viewInverse;
    float4x4 projInverse;
    float4x4 viewProj;
    float4x4 viewProjPrev;
}

BINDING( 1, 0 ) SamplerState s0 : register( ps, s0 );
BINDING( 1, 1 ) Texture2D t0 : register( ps, t0 );
// BINDING( 1, 0 ) SamplerState s0 : register( ps, s0 );
// BINDING( 1, 1 ) Texture2D t0 : register( ps, t0 );
BINDING( 1, 0 )
cbuffer ObjectBuffer : register( b1 ) { float4x4 mWorld; }

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------

struct VS_BASE3D_IN
{
    float4 vPosition : POSITION;
    float4 cColor : COLOR;
    float2 vTexCoord : TEXCOORD;
    float3 vNormals : NORMAL;
};

struct PS_BASE3D_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexCoord : TEXCOORD;
    float4 cColor : COLOR;
};

//--------------------------------------------------------------------------------------
// Vertex Shaders
//--------------------------------------------------------------------------------------

PS_BASE3D_IN BaseVS( VS_BASE3D_IN i )
{
    PS_BASE3D_IN o;

    float4 world_pos = mul( mWorld, float4( i.vPosition.xyz, 1 ) );

    o.vTexCoord = i.vTexCoord;
    o.cColor    = i.cColor.bgra;
    o.vPosition = mul( mViewProj, world_pos ); // transform to clip space

    return o;
}

//--------------------------------------------------------------------------------------
// Pixel Shaders
//--------------------------------------------------------------------------------------
float4 NoTexPS( PS_BASE3D_IN i ) : SV_Target
{
    float4 OutColor;
    OutColor.rgb = i.cColor;
    // OutColor.r = fPadding.x;
    return OutColor;
}

float4 TexPS( PS_BASE3D_IN i ) : SV_Target
{
    float4 OutColor;
    OutColor = /*t0.Sample( s0, i.vTexCoord ) */ i.cColor;
    // OutColor.a = 1;
    return OutColor;
}