#ifdef D3D11_API
#define BINDING( SET, BIND_ID )
#else
#define BINDING( SET, BIND_ID ) [[vk::binding( BIND_ID, SET )]]
#endif
//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

BINDING( 0, 0 )
cbuffer CameraBuffer : register( b0 ) { float4x4 mView;float4x4 mProj;float4x4 mViewInv;float4x4 mProjInv; }

BINDING( 1, 0 )
cbuffer ObjectBuffer : register( b1 ) { float4x4 mWorld;  float4x4 mWorldInv; }

struct VS_INPUT
{
    float4 vPosition   	: POSITION;
    float2 vTexCoord    : TEXCOORD;
    float3 vInNormal : NORMAL;
    float4 vInWeights : WEIGHTS;
    uint vInIndices : INDICES;
    float4 vInColor : COLOR;
    uint nMatId : MAT_IDX;
};

struct PS_DEFERRED_DN_IN
{
    float4 vPosition    : SV_POSITION;
    float4 vColor       : COLOR;
    float4 vNormalDepth : NORMAL;
    float4 vTexCoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
};

struct PS_DEFERRED_OUT
{
	float4 vColor				: SV_Target0; // color rgb, roughness a
	float4 vNormalDepth			: SV_Target1; // normals - xy and depth - zw
	//float4 vParameters			: SV_Target2; // x - spec intensity, y - glossiness,
                                              // z - custom parameter, w - material id
                                              // for now material id-s is following 0 - simple object, 1 - car object, 2 - ped/skin object, 3 - vertexlit object
                                              // custom param for simple object - metallness or detail tex id or something else?
                                              // custom param for car - metallness or second layer glossiness ?
                                              // custom param for ped/skin - translucency
    //float4 vLighting            : SV_Target3;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_DEFERRED_DN_IN BaseVS(VS_INPUT i)
{
    PS_DEFERRED_DN_IN o;
	float4 	outPos 		= float4( i.vPosition,1.0);// transform to screen space
			outPos 		= mul( outPos, mWorld );
    o.vWorldPos = outPos;

	outPos		= mul( outPos, mView );

    o.vPosition         = mul( outPos, mProj );

    o.vNormalDepth =
        float4( mul( (float3x3)mWorldInv, i.vInNormal ), outPos.z );
    o.vTangent   = float4( mul( (float3x3)mWorldInv, i.vInTangents ), 1.0 );
    o.vBiTangent = float4( mul( (float3x3)mWorldInv, i.vInBiTangents ), 1.0 );
	o.vTexCoord     = float4(i.vTexCoord, 0, 0);
	o.vColor		= i.vInColor;

    return o;
}
/*!
    Encoding 32 bit float to 16bit RG chanell.
*/
inline float2 EncodeFloatRG(float v)
{
	float2 kEncodeMul = float2(1.0, 255.0);
	float kEncodeBit = 1.0 / 255.0;
	float2 enc = kEncodeMul * v;
	enc = frac(enc);
	enc.x -= enc.y * kEncodeBit;
	return enc;
}

/*!
    Decoding 32bit float from 16bit RG chanell.
*/
inline float DecodeFloatRG(float2 enc)
{
	float2 kDecodeDot = float2(1.0, 1 / 255.0);
    float res = dot(enc, kDecodeDot);
    return res * fFarClip;
}
/*!
    Encodes normals.
*/
inline float2 EncodeNormals(float2 v)
{
	return v*0.5+0.5;
}
/*!
    Encodes normals via SM.
*/
inline float2 EncodeNormalsSphereMap(float3 v)
{
    float p = sqrt(v.z * 8 + 8);
    return v.xy / p + 0.5f;
}
/*!
    Decodes normals.
*/
inline float3 DecodeNormals(float2 v)
{
	float3 n;
	n.xy = v * 2 - 1;
	n.z = -sqrt(max(1.01 - dot(n.xy, n.xy), 0.0f));// added 0.01 to avoid z-fighting(or sqrt(0) to be more precise)
	return n;
}

PS_DEFERRED_OUT BasePS(PS_DEFERRED_IN i)
{
    PS_DEFERRED_OUT Out;

    float3 normal = i.vNormalDepth.xyz;
    float ViewZ = i.vNormalDepth.w;
    float2 EncNormals = EncodeNormals(normal.xy);
    Out.vColor = float4(1,1,1,1);
    Out.vNormalDepth = float4(EncNormals, EncodeFloatRG(ViewZ / 1000.0f));
    //FillGBufferVertexRadiance( Out, baseColor, normal, i.vNormalDepth.w, params,
    //                           i.vColor * lerp( 0.25f, 1.0f, 1 - vSunLightDir.a ) );
	return Out;
}