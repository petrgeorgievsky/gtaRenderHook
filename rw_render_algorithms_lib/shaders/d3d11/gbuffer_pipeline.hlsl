Texture2D       t0  : register( t0 );
SamplerState    s0  : register( s0 );

cbuffer SceneConstants : register( b0 )
{
    float4x4 mView;
    float4x4 mProjection;
    float4x4 mViewProjection;
    float4x4 mInvViewProjection;
    float4 vViewDir;
    float4 vViewPos;
    float4 deltas;
    float4 padd_[2];
};

cbuffer ModelConstants : register( b1 )
{
    float4x4 mWorld;
    float4x4 mWorldInv;
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

struct PS_OUT
{
    float4 vPosition : SV_Target0;
    float4 vNormals : SV_Target1;
    float4 vColor : SV_Target2;
    float4 vVertexRad : SV_Target3;
};

PS_IN BaseVS( VS_MAIN_IN i )
{
    PS_IN res;

    float4 res_pos = mul( mWorld, float4( i.vPosition.xyz, 1 ) );
    res.vWorldPos = res_pos;
    res.vPosition = mul( mViewProjection, res_pos );
    res.vColor = i.vColor;
    res.vUV = i.vUV;
    res.vNormals = mul( float4( i.vNormals.xyz, 0 ), mWorldInv ).xyz;
    return res;
}

PS_OUT TexPS( PS_IN i )
{
    PS_OUT out_ps;
    float4 texColor = t0.Sample( s0, i.vUV );
    texColor.a *= i.vColor.a;
    clip( texColor.a < 0.3 ? -1 : 1 );

    out_ps.vPosition =  float4( i.vWorldPos, 1);
    out_ps.vNormals = float4( i.vNormals, 1 );
    out_ps.vColor = texColor;
    out_ps.vVertexRad = i.vColor;
    
    return out_ps;
}

PS_OUT NoTexPS( PS_IN i )
{
    PS_OUT out_ps;
    float4 texColor = t0.Sample( s0, i.vUV );
    texColor.a *= i.vColor.a;
    clip( texColor.a < 0.3 ? -1 : 1 );

    out_ps.vPosition =  float4( i.vWorldPos, 1);
    out_ps.vNormals = float4( i.vNormals, 1 );
    out_ps.vColor = float4( 1, 1, 1, 1 );
    out_ps.vVertexRad = i.vColor;
    
    return out_ps;
}