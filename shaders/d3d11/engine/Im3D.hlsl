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
BINDING( 1, 0 ) cbuffer ObjectBuffer : register( b0 ) { float4x4 worldMat; }

BINDING( 2, 0 ) SamplerState s0 : register( ps, s0 );
BINDING( 2, 1 ) Texture2D t0 : register( ps, t0 );

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------

struct VS_IM3D_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : TEXCOORD0;
    float4 cColor : COLOR;
    float2 vTexCoord : TEXCOORD1;
};

struct PS_IM3D_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexCoord : TEXCOORD;
    float4 cColor : COLOR;
};

//--------------------------------------------------------------------------------------
// Vertex Shaders
//--------------------------------------------------------------------------------------

PS_IM3D_IN BaseVS( VS_IM3D_IN i )
{
    PS_IM3D_IN o;

    o.vTexCoord = i.vTexCoord;
    o.cColor    = i.cColor.bgra;
    o.vPosition = mul(
        proj, mul( float4( i.vPosition.x, i.vPosition.y, i.vPosition.z, 1.0f ),
                   view ) ); // transform to screen space

    return o;
}

//--------------------------------------------------------------------------------------
// Pixel Shaders
//--------------------------------------------------------------------------------------
float4 NoTexPS( PS_IM3D_IN i ) : SV_Target
{
    float4 OutColor;
    OutColor.rgb = i.cColor;
    // OutColor.r = fPadding.x;
    return OutColor;
}

float4 TexPS( PS_IM3D_IN i ) : SV_Target
{
    float4 OutColor;
    OutColor = t0.Sample( s0, i.vTexCoord ) * i.cColor;
    // OutColor.a = 1;
    return OutColor;
}