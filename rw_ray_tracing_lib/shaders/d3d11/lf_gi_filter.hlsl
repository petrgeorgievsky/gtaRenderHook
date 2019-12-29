

Texture2D<float4> gbPosition : register(t0);
Texture2D<float4> gbNormals : register(t1);
Texture2D<float4> shY : register(t2);
Texture2D<float2> shCoCg : register(t3);
Texture2D<float4> gbColor : register(t4);
Texture2D<float4> gbRadiance : register(t5);

RWTexture2D<float4> tResult : register(u0);

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

cbuffer GIParams : register( b1 )
{
    float fRenderingScale;
    int nPreFilterIteration;
    float fNormalsScale;
    float fDepthScale;
};

SamplerState s0 : register(s0);

#define THREAD_GROUP_SIZE 8

struct SH
{
    float4 shY;
    float2 CoCg;
};

SH irradiance_to_SH(float3 color, float3 dir)
{
    SH result;

    float   Co      = color.r - color.b;
    float   t       = color.b + Co * 0.5;
    float   Cg      = color.g - t;
    float   Y       = max(t + Cg * 0.5, 0.0);

    result.CoCg = float2(Co, Cg);

    float   L00     = 0.282095;
    float   L1_1    = 0.488603 * dir.y;
    float   L10     = 0.488603 * dir.z;
    float   L11     = 0.488603 * dir.x;

    result.shY = float4 (L11, L1_1, L10, L00) * Y;

    return result;
}
SH init_SH()
{
    SH result;
    result.shY = float4(0,0,0,0);
    result.CoCg = float2(0,0);
    return result;
}

void accumulate_SH(inout SH accum, SH b, float scale)
{
    accum.shY += b.shY * scale;
    accum.CoCg += b.CoCg * scale;
}

SH load_SH(int2 p)
{
    SH result;
    result.shY = shY[p];
    result.CoCg = shCoCg[p];
    return result;
}

SH interpolate_lf(int2 ipos)
{
    float GRAD_DWN = fRenderingScale;
	// Target pixel parameters
	float3 depth_center = gbPosition[ipos].xyz;
	float fwidth_depth = 0.5f;//texelFetch(TEX_PT_MOTION, ipos, 0).w;
	float3 geo_normal_center = gbNormals[ipos].xyz;//decode_normal(texelFetch(TEX_PT_GEO_NORMAL, ipos, 0).x);


	float2 pos_lowres = (float2(ipos)) * GRAD_DWN;
	float2 pos_ld = floor(pos_lowres);
	float2 subpix = frac(pos_lowres - pos_ld);

	SH sum_lf = init_SH();
	float sum_w = 0;

	// 4 bilinear taps
	const int2 off[4] = { { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 } };
	float w[4] = {
		(1.0 - subpix.x) * (1.0 - subpix.y),
		(subpix.x      ) * (1.0 - subpix.y),
		(1.0 - subpix.x) * (subpix.y      ),
		(subpix.x      ) * (subpix.y      )
	};
	for(int i = 0; i < 4; i++) 
	{
		int2 p_lowres = int2(pos_ld) + off[i];
		int2 p_hires = p_lowres / GRAD_DWN;

		// Low-res pixel parameters
		float3 p_depth = gbPosition[p_hires].xyz;//texelFetch(TEX_PT_VIEW_DEPTH_A, p_hires, 0).x;
		float3 p_geo_normal = gbNormals[p_hires].xyz;//decode_normal(texelFetch(TEX_PT_GEO_NORMAL, p_hires, 0).x);

		// Start with bilinear weight
		float p_w = w[i];

		// Compute depth and normal similarity between the target pixel and the low-res anchor pixel
		float dist_depth = abs(distance(p_depth, depth_center));
		p_w *= exp(-dist_depth);
		p_w *= pow(max(0.0, dot(geo_normal_center, p_geo_normal)), 2);

		if(p_w > 0)
		{
			SH p_lf = load_SH(p_lowres);
			accumulate_SH(sum_lf, p_lf, p_w);
			sum_w += p_w;
		}
	}

	if(sum_w > 0)
	{
		// We found at least one relevant pixel among the 4 bilinear taps - good
		float inv_w = 1 / sum_w;
		sum_lf.shY *= inv_w;
		sum_lf.CoCg *= inv_w;
	}
	else
	{
		// We didn't find anything relevant, so use the full-res temporally filtered LF data instead
		sum_lf = init_SH();
	}

	return sum_lf;
}

float3 project_SH_irradiance(SH sh, float3 N)
{
    float d = dot(sh.shY.xyz, N);
    float Y = 2.0 * (1.023326 * d + 0.886226 * sh.shY.w);
    Y = max(Y, 0.0);

    sh.CoCg *= Y * 0.282095 / (sh.shY.w + 1e-6);

    float   T       = Y - sh.CoCg.y * 0.5;
    float   G       = sh.CoCg.y + T;
    float   B       = T - sh.CoCg.x * 0.5;
    float   R       = B + sh.CoCg.x;

    return max(float3(R, G, B), float3(0.0,0.0,0.0));
}

[numthreads(THREAD_GROUP_SIZE, THREAD_GROUP_SIZE, 1)]
void LowFreqGIFilter(uint3 DTid : SV_DispatchThreadID, uint groupid : SV_GroupIndex)
{
    float4 resultColor = float4( 0, 0, 0, 1 );
    float4 position = gbPosition[DTid.xy];
    float3 normals = normalize( gbNormals[DTid.xy].xyz );
   // if( DTid.y < DTid.x * 900.0/1600.0 )
        tResult[DTid.xy] = float4((project_SH_irradiance( interpolate_lf(DTid.xy), normals )), 1 );
   // else
   //     tResult[DTid.xy] = float4( gbRadiance[DTid.xy].rgb, 1 );
}