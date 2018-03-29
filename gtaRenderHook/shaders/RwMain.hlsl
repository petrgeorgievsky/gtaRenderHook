//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register( t0 );
SamplerState samLinear : register( s0 );
cbuffer ConstantBuffer : register( b0 )
{
	row_major matrix World;
	row_major matrix View;
	row_major matrix Projection;
}
cbuffer Globals : register( b1 )
{
	uint	bHasTexture;
	float	fScreenWidth;
	float	fScreenHeight;
	uint	uiAlphaTestType;
	float	fAlphaTestRef;
	uint	bFogEnable;
	uint	uiFogType;
	uint	cFogColor;
}
cbuffer MaterialInfoBuffer : register( b2 )
{
	float4 DiffuseColor;
}
struct VS_INPUT
{
    float3 inPosition   	: POSITION;
    float2 inTexCoord     	: TEXCOORD;
    float3 vInNormal    	: NORMAL;
	float4 vInColor 		: COLOR;
};

struct PS_INPUT
{
	float4 vPosition	: SV_POSITION;
	float4 vColor		: COLOR;
	float3 vNormal		: NORMAL;
	float2 texCoord		: TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT i )
{
	PS_INPUT Out;
	float4 	outPos 	= float4( i.inPosition,1.0);// transform to screen space
			outPos 	= mul( outPos, World );
			outPos	= mul( outPos, View );
    Out.vPosition 	= mul( outPos, Projection );
	Out.vNormal   	= mul( i.vInNormal,(float3x3)World);
	Out.texCoord	= i.inTexCoord;
	Out.vColor		= i.vInColor;
	
    return Out;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT i ) : SV_Target
{
	float4 outColor =txDiffuse.Sample( samLinear, i.texCoord ) * DiffuseColor;
	outColor.xyz	*=(max(dot(i.vNormal,float3(0,0,-1.0f)),0.0f)*0.1f+i.vColor.xyz*0.9f);
	outColor.a		*=i.vColor.w;
	
	if(uiAlphaTestType==1){
		if(outColor.a<=fAlphaTestRef) discard;
	}else if(uiAlphaTestType==2){
		if(outColor.a<fAlphaTestRef) discard;
	}else if(uiAlphaTestType==3){
		if(outColor.a>=fAlphaTestRef) discard;
	}else if(uiAlphaTestType==4){
		if(outColor.a>fAlphaTestRef) discard;
	}else if(uiAlphaTestType==5){
		if(outColor.a==fAlphaTestRef) discard;
	}else if(uiAlphaTestType==6){
		if(outColor.a!=fAlphaTestRef) discard;
	}
	return outColor;
}