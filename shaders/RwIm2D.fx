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
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
float4 VS( in float4 Pos : POSITION,in float4 Color : COLOR,in float2 tc : TEXCOORD, out float2 texCoordOut:TEXCOORD, out float4 Col:COLOR ) : SV_POSITION
{
	texCoordOut=tc;
	Col=Color.zyxw;
	float4 outPos=float4((Pos.x)*2/fScreenWidth-1.0f,1.0f-(Pos.y)*2/fScreenHeight,Pos.z,1.0);// transform to screen space
    return outPos;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( float4 Pos : SV_POSITION,in float2 tc:TEXCOORD,in float4 Col:COLOR ) : SV_Target
{
	float4 outColor;
	if(bHasTexture!=0)
		outColor = float4( txDiffuse.Sample( samLinear, tc ) * Col );
	else
		outColor = float4( Col );
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