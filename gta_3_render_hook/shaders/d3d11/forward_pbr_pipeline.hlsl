Texture2D       t0  : register( t0 );
SamplerState    s0  : register( s0 );

cbuffer SceneConstants : register( b0 )
{
    float4x4 mView;
    float4x4 mProjection;
    float4x4 mViewProjection;
    float4 vViewDir;
    float4 vViewPos;
};

cbuffer ModelConstants : register( b1 )
{
    float4x4 mWorld;
};

cbuffer MaterialConstants : register( b2 )
{
    float4 MaterialColor;
    float Roughness;
    float Metallness;
    float IOR;
    float Emmisivness;
};

struct VS_MAIN_IN
{
    float4 vPosition : POSITION;
    float4 vColor : COLOR;
    float2 vUV : TEXCOORD;
    float3 vNormals : NORMAL;
};

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float4 vColor : COLOR;
    float3 vNormals : NORMAL;
    float3 vWorldPos : TEXCOORD0;
    float2 vUV : TEXCOORD1;
};

PS_IN BaseVS( VS_MAIN_IN i )
{
    PS_IN res;

    float4 res_pos = mul( mWorld, float4( i.vPosition.xyz, 1 ) );
    res.vWorldPos = res_pos;
    res.vPosition = mul( mViewProjection, res_pos );
    res.vColor = i.vColor;
    res.vUV = i.vUV;
    res.vNormals = mul( float4( i.vNormals.xyz, 1 ), mWorld ).xyz;
    return res;
}

float4 NoTexPS( PS_IN i ) : SV_Target
{
    return float4( i.vColor.rgb, 1.0f );
}

inline float LambertDiffuse( in float3 vNormal, in float3 vLightDir )
{
    float fCosA = dot( vNormal, vLightDir ); // cosine of angle between normal and light vector
    return max( fCosA, 0.0f );
}

const float PI = 3.14f;
const float EPS = 1e-10f;

inline float BlinnPhongNDF( in float3 halfDir, in float3 normal, in float roughnessSq ) 
{
    float rsq = roughnessSq * roughnessSq + EPS;
    float power = 2.0f / rsq - 2.0f;
    return pow( max( dot( halfDir, normal ), 0.0f ), power ) /*/ ( rsq * PI )*/;
}

inline float BeckmannNDF( in float3 halfDir, in float3 normal, in float roughnessSq )
{
    float rsq = roughnessSq * roughnessSq + EPS;
    float power = 2.0f / rsq - 2.0f;
    float alpha = max( dot( halfDir, normal ), 0.0f );
    float alphaSq = alpha * alpha;
    float denom = alphaSq * alphaSq * rsq * PI;

    return exp( ( alphaSq - 1.0f ) / ( alphaSq * rsq ) ) /* denom*/;
}

float chiGGX( float v )
{
    return v > 0 ? 1 : 0;
}

float GGX_NDF( in float3 halfDir, in float3 normal, in float roughnessSq )
{
    float alpha =  dot( halfDir, normal );
    float alphaSq = alpha * alpha;

    float TanNdotHSqr = ( 1 - alphaSq ) / alphaSq;
    float denom = ( alphaSq * ( roughnessSq + TanNdotHSqr ) );
    return ( 1.0 / 3.1415926535 ) * roughnessSq / ( denom * denom );

    //float den = alphaSq * ( rsq * rsq - 1 ) + 1;
   // return ( rsq * rsq ) / ( PI * den * den );
}

// normal distribution function,
// represents approximation of microfacet normal distribution over point on surface
float NDF( in float3 halfDir, in float3 normal, in float roughnessSq )
{
    return GGX_NDF( halfDir, normal, roughnessSq );
}

float CookTorranceGSF( float NdotL, float NdotV,
                       float VdotH, float NdotH )
{
    float Gs = min( 1.0, min( 2 * NdotH * NdotV / VdotH,
                              2 * NdotH * NdotL / VdotH ) );
    return  ( Gs );
}

float MixFunction( float i, float j, float x ) {
    return  j * x + i * ( 1.0 - x );
}

float SchlickFresnel( float i ) {
    float x = clamp( 1.0 - i, 0.0, 1.0 );
    float x2 = x * x;
    return x2 * x2* x;
}

//normal incidence reflection calculation
float F0( float NdotL, float NdotV, float LdotH, float roughness ) {
    float FresnelLight = SchlickFresnel( NdotL );
    float FresnelView = SchlickFresnel( NdotV );
    float FresnelDiffuse90 = 0.5 + 2.0 * LdotH * LdotH * roughness;
    return  MixFunction( 1, FresnelDiffuse90, FresnelLight )* MixFunction( 1, FresnelDiffuse90, FresnelView );
}

float4 TexPS( PS_IN i ) : SV_Target
{
    float4 texColor = t0.Sample( s0, i.vUV ) * MaterialColor;
    texColor.a *= i.vColor.a;
    clip( texColor.a < 0.3 ? -1 : 1 );
    float3 lightDir = normalize( float3( 0.5, 0.3, -1 ) );
    float3 viewDir = normalize( i.vWorldPos - vViewPos );
    float3 normalDir = normalize( i.vNormals );
    float3 lighting = LambertDiffuse( i.vNormals, lightDir ).xxx;
    float3 halfDir = normalize( lightDir + viewDir );

    float  NoV = max( dot( normalDir, viewDir ), 0.0f );
    float  NoL = max( dot( normalDir, lightDir ), 0.0f );
    float  VoH = max( dot( halfDir, viewDir ), 0.0f );
    float  NoH = max( dot( halfDir, normalDir ), 0.0f );
    float  LoH = max( dot( halfDir, lightDir ), 0.0f );

    float roughness = saturate( texColor.x * 1.5f + 0.1f );

    float ndf = NDF( halfDir, normalDir, roughness * roughness );
    float geom = CookTorranceGSF( NoL, NoV, VoH, NoH );
    float fresnel = SchlickFresnel( LoH );//F0( NoL, NoV, LoH, Roughness );
    float denominator = max( 4 * NoL * NoV, 0.001f );
    float specularBRDF = ( ndf * geom * fresnel ) / denominator;
    float3 specularColor = lerp( float3( 1, 1, 1 ), texColor, Metallness );
    float3 result = ( texColor * ( 1 - Metallness ) * (NoL + 0.2f) + specularColor * specularBRDF ) ;
    return float4( result, min( max( texColor.a, specularBRDF ), 1 ) );//float4(  * float4( lighting + 0.2f.xxx , 1 ) );
}

float4 SelectedPS( PS_IN i ) : SV_Target
{
    return float4( 1.0, 0, 0, 1.0 );
}