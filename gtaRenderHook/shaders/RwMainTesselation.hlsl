//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#define PNTRIANGLES
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
	float3  g_vEye;
}
cbuffer MaterialInfoBuffer : register( b2 )
{
	float4 DiffuseColor;
}
struct VS_INPUT
{
    float4 inPosition   	: POSITION;
    float2 inTexCoord     	: TEXCOORD;
    float3 vInNormal    	: NORMAL;
	float4 vInColor 		: COLOR;
	
    uint   uVertexID      	: SV_VERTEXID;
};
struct VS_OUTPUT_HS_INPUT
{
    float3 vWorldPos 	: WORLDPOS;
    float3 vNormal   	: NORMAL;

    float2 texCoord  	: TEXCOORD0;
	float4 vColor		: COLOR;
	float  fVertexDistanceFactor : VERTEXDISTANCEFACTOR;
};
struct HS_CONSTANT_DATA_OUTPUT
{
    float    Edges[3]         : SV_TessFactor;
    float    Inside           : SV_InsideTessFactor;
#ifdef PNTRIANGLES
	float3 vB210    : POSITION3;
    float3 vB120    : POSITION4;
    float3 vB021    : POSITION5;
    float3 vB012    : POSITION6;
    float3 vB102    : POSITION7;
    float3 vB201    : POSITION8;
    float3 vB111    : CENTER;
	
	// Normal quadratic generated control points
    float3 vN110    : NORMAL3;      
    float3 vN011    : NORMAL4;
    float3 vN101    : NORMAL5;
#endif
};

struct HS_CONTROL_POINT_OUTPUT
{
    float3 vWorldPos : WORLDPOS;
    float3 vNormal   : NORMAL;
    float2 texCoord  : TEXCOORD;
    float4 vColor	 : COLOR;
};


struct DS_OUTPUT
{
    float2 texCoord          : TEXCOORD0;
	float4 vColor			 : COLOR;
	float3 vNormal			 : NORMAL;
	float3 vWorldPos		 : WORLDPOS;
    float4 vPosition         : SV_POSITION;
};

struct PS_INPUT
{
   float2 texCoord           : TEXCOORD0;
   float4 vColor			 : COLOR;
   float3 vNormal			 : NORMAL;
   float3 vWorldPos			 : WORLDPOS;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT_HS_INPUT VS( VS_INPUT i )
{
	VS_OUTPUT_HS_INPUT Out;
	Out.texCoord = i.inTexCoord;
	Out.vNormal = (mul(World,float4(i.vInNormal,0.0f))).xyz;
	float4 vPositionWS = mul(World, float4(i.inPosition.xyz,1.0f) );
	
	Out.vWorldPos = float3( vPositionWS.xyz );
	Out.vColor = i.vInColor;
	
	// Min and max distance should be chosen according to scene quality requirements
    const float fMinDistance = 2.0f;
    const float fMaxDistance = 25.0f;  

    // Calculate distance between vertex and camera, and a vertex distance factor issued from it
    float fDistance = distance( vPositionWS.xyz, g_vEye );
    Out.fVertexDistanceFactor = 1.0 - clamp( ( ( fDistance - fMinDistance ) / ( fMaxDistance - fMinDistance ) ), 
                                             0.0, 1.0-(1.0/16.0f));
											 
    return Out;
}
float3 AutoNormalGen(float2 texCoord) {
	uint width;
	uint height;
	txDiffuse.GetDimensions(width, height);
   
   float2 off = 1.0 / float2(width,height);
   float4 lightness = float4(0.2,0.59,0.11,0);
   // Take all neighbor samples
   float4 s00 = txDiffuse.SampleLevel( samLinear, texCoord + float2(-off.x, -off.y), 0 );//tex2D(sample, texCoord + float2(-off, -off));
   float4 s01 = txDiffuse.SampleLevel( samLinear, texCoord + float2(0,   -off.y), 0 );//tex2D(sample, texCoord + float2( 0,   -off));
   float4 s02 = txDiffuse.SampleLevel( samLinear, texCoord + float2(off.x, -off.y), 0 );//tex2D(sample, texCoord + float2( off, -off));
   float4 s10 = txDiffuse.SampleLevel( samLinear, texCoord + float2(-off.x,  0), 0 );//tex2D(sample, texCoord + float2(-off,  0));
   float4 s12 = txDiffuse.SampleLevel( samLinear, texCoord + float2(off.x,  0), 0 );//tex2D(sample, texCoord + float2( off,  0));
   float4 s20 = txDiffuse.SampleLevel( samLinear, texCoord + float2(-off.x,  off.y), 0 );//tex2D(sample, texCoord + float2(-off,  off));
   float4 s21 = txDiffuse.SampleLevel( samLinear, texCoord + float2(0,    off.y), 0 );//tex2D(sample, texCoord + float2( 0,    off));
   float4 s22 = txDiffuse.SampleLevel( samLinear, texCoord + float2(off.x,  off.y), 0 );//tex2D(sample, texCoord + float2( off,  off));

   float4 sobelX = s00 + 2 * s10 + s20 - s02 - 2 * s12 - s22;
   float4 sobelY = s00 + 2 * s01 + s02 - s20 - 2 * s21 - s22;
   
   float sx = dot(sobelX, lightness);
   float sy = dot(sobelY, lightness);
   
   float3 normal = normalize(float3(sx, sy, 1));
   return normal * 0.5 + 0.5;
}
//--------------------------------------------------------------------------------------
// Hull Shader
//--------------------------------------------------------------------------------------
HS_CONSTANT_DATA_OUTPUT ConstantsHS( InputPatch<VS_OUTPUT_HS_INPUT, 3> p, uint PatchID : SV_PrimitiveID )
{
    HS_CONSTANT_DATA_OUTPUT output = (HS_CONSTANT_DATA_OUTPUT)0;
    float4 vEdgeTessellationFactors;
    float tess=3;
    // Tessellation level fixed by variable
    vEdgeTessellationFactors = float4(tess,tess,tess,tess);
	
    float3 fScaleFactor;
    fScaleFactor.x = 0.5 * ( p[1].fVertexDistanceFactor + p[2].fVertexDistanceFactor );
    fScaleFactor.y = 0.5 * ( p[2].fVertexDistanceFactor + p[0].fVertexDistanceFactor );
    fScaleFactor.z = 0.5 * ( p[0].fVertexDistanceFactor + p[1].fVertexDistanceFactor );

    // Scale edge factors 
    vEdgeTessellationFactors *= fScaleFactor.xyzx;
	
    // Assign tessellation levels
    output.Edges[0] = vEdgeTessellationFactors.x;
    output.Edges[1] = vEdgeTessellationFactors.y;
    output.Edges[2] = vEdgeTessellationFactors.z;
    output.Inside   = vEdgeTessellationFactors.w;
#ifdef PNTRIANGLES
	// Assign Positions
    float3 vB003 = p[0].vWorldPos;
    float3 vB030 = p[1].vWorldPos;
    float3 vB300 = p[2].vWorldPos;
    // And Normals
    float3 vN002 = p[0].vNormal;
    float3 vN020 = p[1].vNormal;
    float3 vN200 = p[2].vNormal;
        
    // Compute the cubic geometry control points
    // Edge control points
    output.vB210 = ( ( 2.0f * vB003 ) + vB030 - ( dot( ( vB030 - vB003 ), vN002 ) * vN002 ) ) / 3.0f;
    output.vB120 = ( ( 2.0f * vB030 ) + vB003 - ( dot( ( vB003 - vB030 ), vN020 ) * vN020 ) ) / 3.0f;
    output.vB021 = ( ( 2.0f * vB030 ) + vB300 - ( dot( ( vB300 - vB030 ), vN020 ) * vN020 ) ) / 3.0f;
    output.vB012 = ( ( 2.0f * vB300 ) + vB030 - ( dot( ( vB030 - vB300 ), vN200 ) * vN200 ) ) / 3.0f;
    output.vB102 = ( ( 2.0f * vB300 ) + vB003 - ( dot( ( vB003 - vB300 ), vN200 ) * vN200 ) ) / 3.0f;
    output.vB201 = ( ( 2.0f * vB003 ) + vB300 - ( dot( ( vB300 - vB003 ), vN002 ) * vN002 ) ) / 3.0f;
    // Center control point
    float3 vE = ( output.vB210 + output.vB120 + output.vB021 + output.vB012 + output.vB102 + output.vB201 ) / 6.0f;
    float3 vV = ( vB003 + vB030 + vB300 ) / 3.0f;
    output.vB111 = vE + ( ( vE - vV ) / 2.0f );
    
    // Compute the quadratic normal control points, and rotate into world space
    float fV12 = 2.0f * dot( vB030 - vB003, vN002 + vN020 ) / dot( vB030 - vB003, vB030 - vB003 );
    output.vN110 = normalize( vN002 + vN020 - fV12 * ( vB030 - vB003 ) );
    float fV23 = 2.0f * dot( vB300 - vB030, vN020 + vN200 ) / dot( vB300 - vB030, vB300 - vB030 );
    output.vN011 = normalize( vN020 + vN200 - fV23 * ( vB300 - vB030 ) );
    float fV31 = 2.0f * dot( vB003 - vB300, vN200 + vN002 ) / dot( vB003 - vB300, vB003 - vB300 );
    output.vN101 = normalize( vN200 + vN002 - fV31 * ( vB003 - vB300 ) );
#endif
    return output;
}
[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ConstantsHS")]
HS_CONTROL_POINT_OUTPUT HS( InputPatch<VS_OUTPUT_HS_INPUT, 3> inputPatch, 
                            uint uCPID : SV_OutputControlPointID )
{
    HS_CONTROL_POINT_OUTPUT    output = (HS_CONTROL_POINT_OUTPUT)0;
    
    // Copy inputs to outputs
    output.vWorldPos = inputPatch[uCPID].vWorldPos.xyz;
    output.vNormal =   inputPatch[uCPID].vNormal;
    output.texCoord =  inputPatch[uCPID].texCoord;
	output.vColor	= inputPatch[uCPID].vColor;

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
#ifdef PNTRIANGLES
	// The barycentric coordinates
    float fU = BarycentricCoordinates.x;
    float fV = BarycentricCoordinates.y;
    float fW = BarycentricCoordinates.z;

    // Precompute squares and squares * 3 
    float fUU = fU * fU;
    float fVV = fV * fV;
    float fWW = fW * fW;
    float fUU3 = fUU * 3.0f;
    float fVV3 = fVV * 3.0f;
    float fWW3 = fWW * 3.0f;
    
    // Compute position from cubic control points and barycentric coords
    float3 vWorldPos  = TrianglePatch[0].vWorldPos * fWW * fW +
                        TrianglePatch[1].vWorldPos * fUU * fU +
                        TrianglePatch[2].vWorldPos * fVV * fV +
                        input.vB210 * fWW3 * fU +
                        input.vB120 * fW * fUU3 +
                        input.vB201 * fWW3 * fV +
                        input.vB021 * fUU3 * fV +
                        input.vB102 * fW * fVV3 +
                        input.vB012 * fU * fVV3 +
                        input.vB111 * 6.0f * fW * fU * fV;
    
    // Compute normal from quadratic control points and barycentric coords
    float3 vNormal  =   TrianglePatch[0].vNormal * fWW +
                        TrianglePatch[1].vNormal * fUU +
                        TrianglePatch[2].vNormal * fVV +
                        input.vN110 * fW * fU +
                        input.vN011 * fU * fV +
                        input.vN101 * fW * fV;
#else
    // Interpolate world space position with barycentric coordinates
    float3 vWorldPos = BarycentricCoordinates.x * TrianglePatch[0].vWorldPos + 
                       BarycentricCoordinates.y * TrianglePatch[1].vWorldPos + 
                       BarycentricCoordinates.z * TrianglePatch[2].vWorldPos;
    
    // Interpolate world space normal and renormalize it
    float3 vNormal = BarycentricCoordinates.x * TrianglePatch[0].vNormal + 
                     BarycentricCoordinates.y * TrianglePatch[1].vNormal + 
                     BarycentricCoordinates.z * TrianglePatch[2].vNormal;
					 
#endif
    output.vNormal = normalize( vNormal );
	
    // Interpolate other inputs with barycentric coordinates
    output.texCoord = BarycentricCoordinates.z * TrianglePatch[0].texCoord + 
                      BarycentricCoordinates.x * TrianglePatch[1].texCoord + 
                      BarycentricCoordinates.y * TrianglePatch[2].texCoord;
    output.vColor 	= BarycentricCoordinates.z * TrianglePatch[0].vColor + 
                      BarycentricCoordinates.x * TrianglePatch[1].vColor + 
                      BarycentricCoordinates.y * TrianglePatch[2].vColor;
	float fHeightMapMIPLevel = clamp( ( distance( vWorldPos, g_vEye ) - 100.0f ) / 100.0f, 3.0f, 6.0f);
	float4 vNormalHeight = ((txDiffuse.SampleLevel( samLinear, output.texCoord, 0 )-0.5f) * max(10, 0)) + 0.5f;
    //output.vNormal=;
    // Displace vertex along normal
    vWorldPos += (normalize(output.vNormal*(1.0f-vNormalHeight.xyz)))*0.05f;
	
    // Transform world position with viewprojection matrix
	float4 outPos = mul( View,float4( vWorldPos.xyz, 1.0 ) );
    outPos = mul( Projection,outPos );
    output.vPosition = outPos;
    output.vWorldPos = vWorldPos;
    return output;
}
struct PS_OUTPUT
{
	float4 vColor				: SV_Target0;
	float4 vNormalSpecular		: SV_Target1;
	float4 vWorldPos			: SV_Target2;
};
//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
PS_OUTPUT PS( PS_INPUT i )
{
	PS_OUTPUT Out;
	float4 outColor		=txDiffuse.Sample( samLinear, i.texCoord )*DiffuseColor;
	//outColor.xyz*=(max(dot(i.vNormal,float3(0,0,-1.0f)),0.0f)*0.2f+i.vColor.xyz*0.7f);
	outColor.a			*=i.vColor.w;
	Out.vColor			= outColor;
	Out.vNormalSpecular	= float4(i.vNormal,0.0);
	Out.vWorldPos		= float4(i.vWorldPos,0.0);
	//outColor.xyz=i.vNormal;
	/*if(uiAlphaTestType==1){
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
	}*/
	return Out;
}