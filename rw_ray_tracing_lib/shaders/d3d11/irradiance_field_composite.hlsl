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

Texture2D<float4> gbPosition : register(t0);
Texture2D<float4> gbNormals : register(t1);
Texture2D<float4> irradianceGrid : register(t2);

RWTexture2D<float4> tResult : register(u0);


SamplerState s0 : register(s0);

#define THREAD_GROUP_SIZE 4
#define GRID_SIZE 64

float3 GetIrradianceSample(float3 dir, uint2 pos)
{
    float2 theta_phi = float2( acos(clamp(dir.z,-1,1)),atan(dir.y/dir.x) );
    float2 sample_pos = pos + float2(theta_phi.x/(3.14*2),theta_phi.y/(3.14));
    return irradianceGrid.SampleLevel(s0, float2(sample_pos.x/(GRID_SIZE*4),sample_pos.y/(GRID_SIZE*GRID_SIZE*4)), 0);
}

[numthreads(THREAD_GROUP_SIZE, THREAD_GROUP_SIZE, 1)]
void Composite(uint3 DTid : SV_DispatchThreadID, uint groupid : SV_GroupIndex)
{
    int3 volume_center = floor(vViewPos.xyz) - int3(GRID_SIZE,GRID_SIZE,GRID_SIZE)*0.5f;
    int3 sample_id = (floor(gbPosition[DTid.xy].xyz) - volume_center) ;
    if(sample_id.x < 0 || sample_id.x >= GRID_SIZE || 
        sample_id.y < 0 || sample_id.y >= GRID_SIZE ||
        sample_id.z < 0 || sample_id.z >= GRID_SIZE)
    {
        tResult[DTid.xy] = float4(gbNormals[DTid.xy].xyz,1);
        return;
    }
    float3 sample_offsets_list[8] =
    {
        float3(1,0,0),
        float3(0,1,0),
        float3(0,0,1),

        float3(1,1,0),
        float3(0,1,1),
        float3(0,1,1),

        float3(1,1,1),
        float3(0,0,0)
    };
    float3 pos = gbPosition[DTid.xy].xyz;
    float3 normals = gbNormals[DTid.xy].xyz;
    float3 irradiance = 0.0f.xxx;
    float i_w =0.0f;
    for( uint s_id = 0; s_id<8;s_id++)
    {
        float3 sample_dir = float3(sample_id) + sample_offsets_list[s_id] + volume_center - pos;
        float s_len = length(sample_dir);
        sample_dir *= 1.0/s_len;
        float3 sample_vis = (dot(sample_dir,normals)+1)*0.5f;
        i_w+=sample_vis;
        irradiance += sample_vis * GetIrradianceSample((sample_dir), uint2(sample_id.x, sample_id.y + sample_id.z*GRID_SIZE*4));
    }

    tResult[DTid.xy] = float4(irradiance/i_w, 1);
}