#include "GameMath.hlsl"
//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#define PNTRIANGLES
Texture2D txDiffuse : register( t0 );
Texture2D txSpec 	: register( t1 );
SamplerState samLinear : register( s0 );


// Constant buffer for bone matrices
cbuffer SkinningInfoBuffer : register( b3 )
{
	float4x3 g_mConstBoneWorld[64];
};
struct VS_INPUT
{
    float4 inPosition   	: POSITION;
    float2 inTexCoord     	: TEXCOORD;
    float3 vInNormal    	: NORMAL;
	float4 vInColor 		: COLOR;
	float4 Weights			: WEIGHTS;
	uint4  Bones			: BONES;
#if FEATURE_LEVEL >= 0xb000
    uint   uVertexID      	: SV_VERTEXID;
#endif
};
struct VS_OUTPUT_HS_INPUT
{
    float3 vWorldPos 	: WORLDPOS;
    float3 vNormal   	: NORMAL;

    float2 texCoord  	: TEXCOORD0;
	float4 vColor		: COLOR;
};
struct HS_CONSTANT_DATA_OUTPUT
{
    float    Edges[3]         : SV_TessFactor;
    float    Inside           : SV_InsideTessFactor;
#ifdef PNTRIANGLES
    float3 vWorldB111    : CENTER;
#endif
};

struct HS_CONTROL_POINT_OUTPUT
{
#ifdef PNTRIANGLES
	float3 vWorldPos[3] : WORLDPOS;
#else
	float3 vWorldPos : WORLDPOS;
#endif
    float3 vNormal   : NORMAL;
    float2 texCoord  : TEXCOORD;
    float4 vColor	 : COLOR;
};

struct DS_OUTPUT
{
    float4 vColor			 : COLOR;
	float4 vWorldPos		 : WPOS;
	float3 vNormal			 : NORMAL;
	float2 texCoord          : TEXCOORD0;
    float4 vPosition         : SV_POSITION;
};

struct VS_OUTPUT
{
	float4 vColor		: COLOR;
    float3 vNormal   	: NORMAL;
	float3 vWorldPos	: WPOS;
    float2 texCoord  	: TEXCOORD0;
	float4 vPosition 	: SV_POSITION;
};

struct PS_INPUT
{
	float4 vColor			 : COLOR;
	float4 vWorldPos		 : WPOS;
	float3 vNormal			 : NORMAL;
	float2 texCoord          : TEXCOORD0;
};
struct PS_OUTPUT
{
	float4 vColor				: SV_Target0;
	float4 vNormalSpecular		: SV_Target1;
	float4 vWorldPos			: SV_Target2;
};
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
#if FEATURE_LEVEL >= 0xb000
#define VSout VS_OUTPUT_HS_INPUT
#else
#define VSout VS_OUTPUT
#endif
VSout VS( VS_INPUT i )
{
	VSout Out;
	
	Out.texCoord = i.inTexCoord;
	
	
	float4 		outPos			= float4(0.0f,0.0f,0.0f,0.0f);//transform to screen space
	float4x3 BoneToLocal = g_mConstBoneWorld[i.Bones[0]] * i.Weights[0] + g_mConstBoneWorld[i.Bones[1]] * i.Weights[1] + g_mConstBoneWorld[i.Bones[2]] * i.Weights[2] + g_mConstBoneWorld[i.Bones[3]] * i.Weights[3];

	outPos.xyz = mul(float4(i.inPosition.xyz,1.0f),BoneToLocal).xyz;
	outPos.w = 1.0f;

	float4 vPositionWS = mul(outPos, World);
	Out.vWorldPos = float3( vPositionWS.xyz );
#if FEATURE_LEVEL < 0xb000
	Out.vPosition = mul(mul( vPositionWS, View ), Projection);
#endif

	Out.vNormal =  (mul(float4(mul(i.vInNormal,(float3x3)BoneToLocal),0.0f), World)).xyz;
	Out.vColor = i.vInColor;
	
    return Out;
}

//--------------------------------------------------------------------------------------
// Hull Shader
//--------------------------------------------------------------------------------------
HS_CONSTANT_DATA_OUTPUT ConstantsHS( const OutputPatch<HS_CONTROL_POINT_OUTPUT, 3> p )
{
    HS_CONSTANT_DATA_OUTPUT output = (HS_CONSTANT_DATA_OUTPUT)0;
	
    float4 vEdgeTessellationFactors;
    float tess=2;
    // Tessellation level fixed by variable
    vEdgeTessellationFactors = float4(tess,tess,tess,tess);
    
    // Assign tessellation levels
    output.Edges[0] = vEdgeTessellationFactors.x;
    output.Edges[1] = vEdgeTessellationFactors.y;
    output.Edges[2] = vEdgeTessellationFactors.z;
    output.Inside   = vEdgeTessellationFactors.w;
#ifdef PNTRIANGLES
	float3 	f3B300 = p[0].vWorldPos[0],
			f3B210 = p[0].vWorldPos[1],
			f3B120 = p[0].vWorldPos[2],
			f3B030 = p[1].vWorldPos[0],
			f3B021 = p[1].vWorldPos[1],
			f3B012 = p[1].vWorldPos[2],
			f3B003 = p[2].vWorldPos[0],
			f3B102 = p[2].vWorldPos[1],
			f3B201 = p[2].vWorldPos[2];
	float3 f3E = (f3B210 + f3B120 + f3B021 + f3B012 + f3B102 + f3B201) / 6.0f;
	float3 f3V = (f3B003 + f3B030 + f3B300) / 3.0f;
	output.vWorldB111 = f3E + ((f3E - f3V) / 2.0f);
#endif
    return output;
}
float3 ComputeCP(float3 pA, float3 pB, float3 nA) {
	return (2 * pA + pB - (dot((pB - pA), nA) * nA)) / 3.0f;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ConstantsHS")]
HS_CONTROL_POINT_OUTPUT HS( InputPatch<VS_OUTPUT_HS_INPUT, 3> inputPatch, 
                            uint uCPID : SV_OutputControlPointID )
{
    HS_CONTROL_POINT_OUTPUT    output = (HS_CONTROL_POINT_OUTPUT)0;
    const uint NextCPID = uCPID < 2 ? uCPID + 1 : 0;
	const uint AddtlData = 3 + 2 * uCPID;
	const uint NextAddtlData = AddtlData + 1;

    // Copy inputs to outputs
#ifdef PNTRIANGLES
	output.vWorldPos[0] = inputPatch[uCPID].vWorldPos.xyz;
#else
    output.vWorldPos = inputPatch[uCPID].vWorldPos.xyz;
#endif
    output.vNormal =   inputPatch[uCPID].vNormal;
    output.texCoord =  inputPatch[uCPID].texCoord;
	output.vColor	= inputPatch[uCPID].vColor;
#ifdef PNTRIANGLES
	output.vWorldPos[1] = ComputeCP(inputPatch[uCPID].vWorldPos.xyz, inputPatch[NextCPID].vWorldPos.xyz, inputPatch[uCPID].vNormal);
	output.vWorldPos[2] = ComputeCP(inputPatch[NextCPID].vWorldPos.xyz, inputPatch[uCPID].vWorldPos.xyz, inputPatch[NextCPID].vNormal);
#endif
    return output;
}
//--------------------------------------------------------------------------------------
// Domain Shader
//--------------------------------------------------------------------------------------
[domain("tri")]
DS_OUTPUT DS( HS_CONSTANT_DATA_OUTPUT input, float3 BarycentricCoordinates : SV_DomainLocation, 
             const OutputPatch<HS_CONTROL_POINT_OUTPUT, 3> TrianglePatch )
{
    DS_OUTPUT output = (DS_OUTPUT)0;
	float fU = BarycentricCoordinates.x;
	float fV = BarycentricCoordinates.y;
	float fW = BarycentricCoordinates.z;

#ifdef PNTRIANGLES
	float fUU = fU * fU;
	float fVV = fV * fV;
	float fWW = fW * fW;
	float fUU3 = fUU * 3.0f;
	float fVV3 = fVV * 3.0f;
	float fWW3 = fWW * 3.0f;

	// Interpolate world space position with barycentric coordinates
    float3 vWorldPos = 	  TrianglePatch[0].vWorldPos[0] * fUU * fU
						+ TrianglePatch[1].vWorldPos[0] * fVV * fV
						+ TrianglePatch[2].vWorldPos[0] * fWW * fW
						+ TrianglePatch[0].vWorldPos[1] * fUU3 * fV 
						+ TrianglePatch[0].vWorldPos[2] * fVV3 * fU
						+ TrianglePatch[1].vWorldPos[1] * fVV3 * fW
						+ TrianglePatch[1].vWorldPos[2] * fWW3 * fV
						+ TrianglePatch[2].vWorldPos[1] * fWW3 * fU
						+ TrianglePatch[2].vWorldPos[2] * fUU3 * fW
						+ input.vWorldB111 * 6.0f * fW * fU * fV;
    
    // Interpolate world space normal and renormalize it
    float3 vNormal = fU * TrianglePatch[0].vNormal + 
                     fV * TrianglePatch[1].vNormal + 
                     fW * TrianglePatch[2].vNormal;
#else
    // Interpolate world space position with barycentric coordinates
    float3 vWorldPos = fU * TrianglePatch[0].vWorldPos + 
                       fV * TrianglePatch[1].vWorldPos + 
                       fW * TrianglePatch[2].vWorldPos;
    
    // Interpolate world space normal and renormalize it
    float3 vNormal = fU * TrianglePatch[0].vNormal + 
                     fV * TrianglePatch[1].vNormal + 
                     fW * TrianglePatch[2].vNormal;
					 
#endif
    output.vNormal = normalize( vNormal );

	
    // Interpolate other inputs with barycentric coordinates
    output.texCoord = fU * TrianglePatch[0].texCoord + 
                      fV * TrianglePatch[1].texCoord + 
                      fW * TrianglePatch[2].texCoord;
    output.vColor 	= fU * TrianglePatch[0].vColor + 
                      fV * TrianglePatch[1].vColor + 
                      fW * TrianglePatch[2].vColor;
	
	//vWorldPos+=output.vNormal*0.01f;
	output.vWorldPos.xyz = vWorldPos.xyz;
    // Transform world position with viewprojection matrix
	float4 outPos = mul( float4( vWorldPos.xyz, 1.0 ), View);
    outPos = mul( outPos, Projection);
	output.vWorldPos.w = outPos.z;
    output.vPosition = outPos;
        
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT i ) : SV_Target
{
	float4 outColor=txDiffuse.Sample( samLinear, i.texCoord );
	outColor.xyz*=(max(dot(i.vNormal,float3(0,0,1.0f)),0.0f)+0.3f);
	return outColor;
}
float4 ShadowPS(PS_INPUT i) : SV_Target
{
	float4 outColor = float4(i.vWorldPos.www,1);
	return outColor;
}
void VoxelPS(PS_INPUT i)
{
	
}
void VoxelGS(PS_INPUT i)
{

}
void VoxelVS(PS_INPUT i)
{

}
void VoxelEmmissivePS()
{

}
PS_OUTPUT DeferredPS( PS_INPUT i )
{
	PS_OUTPUT Out;
	Out.vColor 			= txDiffuse.Sample( samLinear, i.texCoord );
	Out.vNormalSpecular = float4(-i.vNormal,0.9f);
	Out.vWorldPos		= float4(i.vWorldPos.xyz,0.1f);
	return Out;
}