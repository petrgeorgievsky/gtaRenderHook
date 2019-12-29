

Texture2D<float4> gbPosition : register(t0);
Texture2D<float4> gbNormals : register(t1);
Texture2D<float4> shY : register(t2);
Texture2D<float2> shCoCg : register(t3);

RWTexture2D<float4> tResultSHY : register(u0);
RWTexture2D<float2> tResultSHCoCg : register(u1);

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

static const float wavelet_factor = 0.5f;
static const float wavelet_kernel[2][2] = {
	{ 1.0, wavelet_factor  },
	{ wavelet_factor, wavelet_factor * wavelet_factor }
};

void
filter_image(out SH filtered_lf, int2 ipos_lowres)
{
    float GRAD_DWN = fRenderingScale;
	int2 ipos_hires = ipos_lowres / GRAD_DWN;

	// Load the color of the target low-res pixel
	SH color_center_lf = load_SH(ipos_lowres);

	// Load the parameters of the anchor pixel
	float3 geo_normal_center = gbNormals[ipos_hires].xyz;
	float3 depth_center = gbPosition[ipos_hires].xyz;//texelFetch(TEX_PT_VIEW_DEPTH_A, ipos_hires, 0).x;
	//float fwidth_depth = texelFetch(TEX_PT_MOTION, ipos_hires, 0).w;

	const int step_size = int(1u << nPreFilterIteration);

	SH sum_color_lf = color_center_lf;

	float sum_w_lf = 1.0;

	// Compute the weighted average of color and moments from a sparse 3x3 pattern around the target pixel
	const int r = 1;
	for(int yy = -r; yy <= r; yy++) {
		for(int xx = -r; xx <= r; xx++) {
			int2 p_lowres = ipos_lowres + int2(xx, yy) * step_size;
			int2 p_hires = p_lowres / GRAD_DWN;

			if(xx == 0 && yy == 0)
				continue;

			float w = 1.0f;

			// Use geometric normals here so that we can blur over larger areas.
			// The lighting detail will be partially preserved by spherical harmonics.
			float3 geo_normal = gbNormals[p_hires].xyz;

			float3 depth = gbPosition[p_hires].xyz;// texelFetch(TEX_PT_VIEW_DEPTH_A, p_hires, 0).x;

			float dist_z = abs(distance(depth_center, depth));
			w *= exp(-dist_z / float(step_size / GRAD_DWN));
			w *= wavelet_kernel[abs(xx)][abs(yy)];


			float w_lf = w;
			
            float GNdotGN = max(0.0, dot(geo_normal_center, geo_normal));
			w_lf *= pow(GNdotGN, 2.0f);
			SH c_lf = load_SH(p_lowres);

            if(nPreFilterIteration >= 3)
				w_lf *= clamp(1.5 - c_lf.shY.w / color_center_lf.shY.w * 0.25, 0, 1);
            
			accumulate_SH(sum_color_lf, c_lf, w_lf);
			sum_w_lf += w_lf;
		}
	}

	filtered_lf.shY = sum_color_lf.shY / sum_w_lf;
	filtered_lf.CoCg = sum_color_lf.CoCg / sum_w_lf;
}

[numthreads(THREAD_GROUP_SIZE, THREAD_GROUP_SIZE, 1)]
void LowFreqGIPreFilter(uint3 DTid : SV_DispatchThreadID, uint groupid : SV_GroupIndex)
{
    SH filtered_lf = init_SH();
    filter_image(filtered_lf, DTid);

    tResultSHY[DTid.xy] = filtered_lf.shY;
    tResultSHCoCg[DTid.xy] = filtered_lf.CoCg;
}