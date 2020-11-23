Texture2D       t0  : register( t0 );
SamplerState    s0  : register( s0 );

cbuffer SceneConstants : register( b0 )
{
    float4x4 mView;
    float4x4 mProjection;
    float4x4 mViewProjection;
};

cbuffer ModelConstants : register( b1 )
{
    float4x4 mWorld;
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
    float2 vUV : TEXCOORD;
    float3 vNormals : NORMAL;
};

PS_IN BaseVS( VS_MAIN_IN i )
{
    PS_IN res;

    float4 res_pos = mul( mWorld, float4( i.vPosition.xyz, 1 ) );

    res.vPosition = mul( mViewProjection, res_pos );
    res.vColor = i.vColor;
    res.vUV = i.vUV;
    res.vNormals = i.vNormals;
	return res;
}

float4 NoTexPS( PS_IN i ) : SV_Target
{
    return float4( i.vColor.rgb, 1.0f );
}

inline float LambertDiffuse( in float3 vNormal, in float3 vLightDir ) {
    float fCosA = dot( vNormal, vLightDir ); // cosine of angle between normal and light vector
    return max( fCosA, 0.0f );
}

float4 TexPS( PS_IN i ) : SV_Target
{
    //return float4( i.vNormals, 1 );
    float4 texColor = t0.Sample( s0, i.vUV );
    clip( texColor.a < 0.3 ? -1 : 1 );
    //if( texColor.a < 0.1 )
    //    discard;
    float3 lightDir = float3( 0.5, 0.3, -1 );
    float3 lighting = LambertDiffuse( i.vNormals, lightDir ).xxx;
    return float4( texColor*float4( lighting + 0.2f.xxx , 1) );
}

float4 TexSelectedPS( PS_IN i ) : SV_Target
{
    //return float4( i.vNormals, 1 );
    float4 texColor = t0.Sample( s0, i.vUV ) * float4( 1.0, 0, 0, 1.0 );
    clip( texColor.a < 0.8f ? -1 : 1 );
    float3 lightDir = float3( 0.5, 0.3, -1 );
    float3 lighting = LambertDiffuse( i.vNormals, lightDir ).xxx;
    return float4( texColor * float4( lighting + 0.2f.xxx , 1 ) );
}