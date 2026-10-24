#include "GameMath.hlsl"
#include "GBuffer.hlsl"
//--------------------------------------------------------------------------------------
// Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register( t0 );
SamplerState samLinear : register( s0 );

struct VS_IM3D_IN
{
    float4 vPosition    : POSITION;
    float4 cColor       : COLOR;
    float2 vTexCoord    : TEXCOORD;
};

struct PS_IM3D_IN
{
    float4 vPosition    : SV_POSITION;
    float4 cColor       : COLOR;
    float2 vTexCoord    : TEXCOORD;
    float4 vWorldPos    : TEXCOORD1;    // w - view-space depth
};
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_IM3D_IN VS(VS_IM3D_IN i)
{
    PS_IM3D_IN o;
	o.vTexCoord = i.vTexCoord;
	o.cColor    = i.cColor.zyxw;
	float4 outPos = float4(i.vPosition.xyz,1.0);// transform to world space
    outPos = mul( outPos, mWorld );
    o.vWorldPos = outPos;
	outPos = mul( outPos, mView );
    o.vWorldPos.w = outPos.z;
    outPos = mul( outPos, mProjection );
    o.vPosition = outPos;
    return o;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_IM3D_IN i) : SV_Target
{
	float4 OutColor;
	if(bHasTexture!=0)
        OutColor = float4(txDiffuse.Sample(samLinear, i.vTexCoord) * i.cColor);
	else
        OutColor = float4(i.cColor);
	
    DO_ALPHA_TEST(OutColor.w)
    return OutColor;
}

//--------------------------------------------------------------------------------------
// G-buffer pixel shader: the G-buffer can't be blended, so transparent texels are cut out. Im3D vertices carry no
// normals the game fills in, so the face normal is taken from the triangle itself, turned towards the camera.
//--------------------------------------------------------------------------------------
PS_DEFERRED_OUT DeferredPS(PS_IM3D_IN i)
{
    PS_DEFERRED_OUT Out;
    float4 baseColor = i.cColor;
    if (bHasTexture != 0)
        baseColor *= txDiffuse.Sample(samLinear, i.vTexCoord);
    if (baseColor.a < 0.3)
        discard;
    float3 normal = normalize(cross(ddx(i.vWorldPos.xyz), ddy(i.vWorldPos.xyz)));
    if (dot(normal, mViewInv[3].xyz - i.vWorldPos.xyz) < 0)
        normal = -normal;
    FillGBuffer(Out, baseColor, normal, i.vWorldPos.w, float4(0.1f, 0.1f, 0, 0));
    return Out;
}
