cbuffer SceneConstants : register(b0)
{
    float screen_width;
    float screen_height;
    uint sphere_count;
    uint tri_count;
    //float4x4 mView;
    //float4x4 mProjection;
};
cbuffer SceneVariables : register(b1)
{
    float random_a;
    float random_b;
    uint random_a_ui;
    uint random_b_ui;
    float4 camPos;
    float4 lightPos;
};
/*!
    Deferred output structure.
*/
struct PS_DEFERRED_OUT
{
    float4 vColor       : SV_Target0; // color rgb, roughness a
    float4 vNormalDepth : SV_Target1; // normals - xy and depth - zw
};
/*!
    Deferred input structure.
*/
struct PS_DEFERRED_IN
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float4 vWPos : TEXCOORD0;
    /*float4 vColor : COLOR;
    
    float4 vTexCoord : TEXCOORD0;*/
};
struct VS_MAIN_IN
{
    float4 vPosition : POSITION;
    float4 vNormal : NORMAL;
};

PS_DEFERRED_IN deferred_vs(VS_MAIN_IN i)
{
    PS_DEFERRED_IN o;
    float4 outPos = float4(i.vPosition.xyz, 1.0); // transform to screen space
    //outPos = mul(outPos, mWorld);
    o.vWPos = float4(outPos.xyz, 1.0);
    outPos.xyz -= camPos.xyz; //mul(outPos, mView);
    float zf = 1000.0f;
    float zn = 0.1f;
    float4x4 proj = float4x4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.33f, 0.0f, 0.0f,
            0.0f, 0.0f, zf / (zf - zn), 1.0f,
            0.0f, 0.0f, -zn * zf / (zf - zn), 0.0f
            );
    o.vPosition = mul(outPos, proj); //mul(outPos, mProjection);
    o.vNormal = i.vNormal;
    return o;
}

PS_DEFERRED_OUT deferred_ps(PS_DEFERRED_IN i)
{
    PS_DEFERRED_OUT o;
    o.vColor = float4(i.vWPos.xyz, 1);
    o.vNormalDepth = float4(i.vNormal.xyz, 1);
    return o;
}