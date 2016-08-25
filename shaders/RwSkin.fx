//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register( t0 );
SamplerState samLinear : register( s0 );
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
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
// Constant buffer for bone matrices
cbuffer SkinningInfoBuffer : register( b3 )
{
	float4x3 g_mConstBoneWorld[64];
};
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
float4 VS( in float4 Pos : POSITION,in float2 tc:TEXCOORD,in float3 normals:NORMAL,in float4 Color : COLOR, in float4 Weights : WEIGHTS,in uint4  Bones : BONES, out float2 texCoordOut:TEXCOORD, out float4 Col:COLOR,out float3 norm:NORMAL ) : SV_POSITION
{
	uint magic0=3;
	int magic1=0;
	float4 vPos=float4(Pos.xyz,1.0f);
	float4 outPos=float4(0.0f,0.0f,0.0f,0.0f);//transform to screen space
	
	float4x3 BoneToLocal = g_mConstBoneWorld[Bones[0]] * Weights[0] + g_mConstBoneWorld[Bones[1]] * Weights[1] + g_mConstBoneWorld[Bones[2]] * Weights[2] + g_mConstBoneWorld[Bones[3]] * Weights[3];

	outPos.xyz = mul(vPos,BoneToLocal).xyz;
	outPos.w = 1.0f;
	outPos = mul(World,outPos);
    outPos = mul(View ,outPos);
    outPos = mul(Projection,outPos );
	norm   = (mul(World,float4(mul(normals,(float3x3)BoneToLocal),0.0f))).xyz;
	
	texCoordOut=tc;
	Col=Bones;
    return outPos;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( float4 Pos : SV_POSITION,in float2 tc:TEXCOORD,in float4 Col:COLOR,in float3 norm:NORMAL ) : SV_Target
{
	float4 outColor=txDiffuse.Sample( samLinear, tc );
	outColor.xyz*=(max(dot(norm,float3(0,0,1.0f)),0.0f));
	//outColor.a*=Col.w;
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