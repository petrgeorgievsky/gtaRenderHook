
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

struct VS_MAIN_IN
{
    float4 vPosition : POSITION;
};

struct PS_IN
{
    float4 vPosition : SV_POSITION;
};

PS_IN BaseVS( VS_MAIN_IN i )
{
    PS_IN res;

    float4 res_pos = mul( mWorld, float4( i.vPosition.xyz, 1 ) );
    res.vPosition = mul( mViewProjection, res_pos );
    return res;
}

float4 LinePS( PS_IN i ) : SV_Target
{
    return float4( 1.0f,0.0f,0.0f, 1.0f );
}