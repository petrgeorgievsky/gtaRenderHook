#include "AtmosphericScatteringFunctions.hlsli"
#include "GBuffer.hlsl"
#include "GameMath.hlsl"
#include "Shadows.hlsl"
Texture2D txScreen : register( t0 );
Texture2D txGB1 : register( t1 );
Texture2D txVolumetric : register( t2 );
// red - inverted woreley, green - perlin,
Texture3D txNoise : register( t3 );
#ifndef SAMLIN
#define SAMLIN
SamplerState samLinear : register( s0 );
#endif
SamplerComparisonState samShadow : register( s1 );
#ifndef CLOUD_RM_STEPS
#define CLOUD_RM_STEPS 16
#endif
#ifndef CLOUD_TO_SUN_RM_STEPS
#define CLOUD_TO_SUN_RM_STEPS 4
#endif

struct PS_QUAD_IN
{
    float4 vPosition : SV_Position;
    float4 vTexCoord : TEXCOORD0;
};

float ReMap( float value, float old_low, float old_high, float new_low,
             float new_high )
{
    float ret_val = new_low + ( value - old_low ) * ( new_high - new_low ) /
                                  ( old_high - old_low );
    return ret_val;
}

float SampleCloudsDensityAt( float3 Pos )
{
    const float Altitude1 = CloudEndHeight;
    float       min_dist  = CloudStartHeight;
    float       max_dist  = Altitude1;

    // float height_density =
    //   lerp( 0.0f, 1.0f,
    //          min( saturate( ( Pos.z - min_dist )/ 5.0f ),
    //               saturate( 1.0f - ( Pos.z - max_dist )/ 5.0f ) ) );
    float height_density2 =
        saturate( ReMap( Pos.z, min_dist, max_dist, 0.0f, 1.0f ) );
    float  height_density = abs( ( height_density2 - 0.5f ) * 2.0 );
    float4 noise_tex = saturate( txNoise.Sample( samLinear, Pos / 100.0f ) );
    float4 small_noise_tex =
        saturate( txNoise.Sample( samLinear, Pos / 50.0f ) );
    float4 noise_tex_2d = saturate( txNoise.Sample(
        samLinear,
        float3( ( Pos.xy ) / 3000.0f + Time * 0.0001f * CloudSpeed * float2( 0.5f, 0.5f ),
                0.0f ) ) );
    float  main_shape =
        pow( noise_tex_2d.a, lerp( 45.0f, 0.5f, CloudCoverage ) );
    float shape = saturate( ReMap( height_density2, 0.0, 0.07f, 0.0f, 1.0f ) );
    float stop_height = saturate( 0.5f + 0.12 );
    shape *= saturate(
        ReMap( height_density2, stop_height * 0.2f, stop_height, 1.0f, 0.0f ) );
    float detail = ( small_noise_tex.r ) * 0.625f +
                   ( small_noise_tex.g ) * 0.25f + ( small_noise_tex.b ) * 0.125f;
    return detail * shape *
           main_shape; // height_density
                       // *small_noise_tex.r*small_noise_tex.b * main_shape;
}
float Beer( float depth )
{

    float Extinct = 0.01f;
    return exp( -Extinct * depth );
}

float BeerPowder( float depth )
{
    float Extinct = 0.01f;
    return exp( -Extinct * depth ) * ( 1 - exp( -Extinct * 2 * depth ) );
}

float3 line_plane_intersection( float3 line_dir, float3 line_start,
                                float3 point_on_plane, float3 plane_norm )
{
    float3 diff  = line_start - point_on_plane;
    float  prod1 = dot( diff, plane_norm );
    float  prod2 = dot( line_dir, plane_norm );
    float  prod3 = prod1 / prod2;
    return line_start - line_dir * prod3;
}

bool DetermineStartAndEndRMPoints( float3 CamPos, float3 WorldPos,
                                   float min_height, float max_height,
                                   out float3 start_pos, out float3 end_pos )
{
    bool above_clouds = CamPos.z > max_height;
    bool below_clouds = CamPos.z < min_height;
    bool in_clouds    = !above_clouds && !below_clouds;

    float3 ViewDir = normalize( WorldPos - CamPos );

    start_pos = 0.0f.xxx;
    end_pos   = 0.0f.xxx;
    if ( below_clouds && ViewDir.z < 0 || above_clouds && ViewDir.z > 0 )
        return false;
    const float3 low_plane_normal  = float3( 0, 0, -1 );
    const float3 high_plane_normal = float3( 0, 0, 1 );
    if ( below_clouds )
    {
        start_pos = line_plane_intersection(
            ViewDir, CamPos, float3( 0, 0, min_height ), low_plane_normal );
        end_pos = line_plane_intersection(
            ViewDir, CamPos, float3( 0, 0, max_height ), high_plane_normal );
    }
    else if ( above_clouds )
    {
        end_pos = line_plane_intersection(
            ViewDir, CamPos, float3( 0, 0, min_height ), low_plane_normal );
        start_pos = line_plane_intersection(
            ViewDir, CamPos, float3( 0, 0, max_height ), high_plane_normal );
    }
    else
    {
        start_pos = CamPos;
        end_pos   = CamPos + ViewDir * 8;
        if ( ViewDir.z < 0 )
            end_pos = line_plane_intersection(
                ViewDir, CamPos, float3( 0, 0, min_height ), low_plane_normal );
        else
            end_pos = line_plane_intersection( ViewDir, CamPos,
                                               float3( 0, 0, max_height ),
                                               high_plane_normal );
    }
    return true;
}

float MarchLight( float3 pos, float3 light )
{
    float3 start;
    float3 end;
    int    iter_count = CLOUD_TO_SUN_RM_STEPS;
    float  dist_      = 8.0f;

    pos = pos + light * dist_ / float( iter_count );

    float depth = 0;
    for ( int s = 0; s < iter_count; s++ )
    {
        depth += SampleCloudsDensityAt( pos );
        pos += light * dist_ / float( iter_count );
    }

    return depth / float( iter_count );
}

float HG( float cos_angle, float g )
{
    float g2  = g * g;
    float val = ( ( 1.0f - g2 ) / pow( 1.0f + g2 - 2.0f * g * cos_angle, 1.5f ) ) /
                4 * 3.1415f;

    return val;
}

float Attenuation( float density_to_sun, float cos_angle )
{
    float cloud_attuention_clampval = 1.0f;
    float prim                      = Beer( density_to_sun );
    float scnd                      = Beer( cloud_attuention_clampval ) * 0.7f;

    // reduce clamping while facing the sun
    float checkval = ReMap( cos_angle, 0.0f, 1.0f, scnd, scnd * 0.5f );
    return max( checkval, prim );
}

float OutScatterAmbient( float density, float percent_height )
{
    float cloud_outscatter_ambient = 1.0f;
    float depth       = cloud_outscatter_ambient * pow( density, 0.75f );
    float vertical    = pow( 0.85f, 0.8f );
    float out_scatter = depth * vertical;
    out_scatter       = 1.0f - saturate( out_scatter );
    return out_scatter;
}
float InOutScatter( float cos_angle )
{
    float cloud_silver_intensity = 1.0f;
    float cloud_inscatter        = 1.0f;
    float cloud_outscatter       = 1.0f;
    float cloud_silver_exponent  = 1.0f;
    float cloud_in_vs_outscatter = 0.5f;

    float first_hg  = HG( cos_angle, cloud_inscatter );
    float second_hg = cloud_silver_intensity *
                      pow( saturate( cos_angle ), cloud_silver_exponent );
    float in_scatter_hg  = max( first_hg, second_hg );
    float out_scatter_hg = HG( cos_angle, -cloud_outscatter );

    return lerp( in_scatter_hg, out_scatter_hg, cloud_in_vs_outscatter );
}

float3 ComputeCloudsAtDir( PS_QUAD_IN i, float3 LightDir, float3 ViewPos,
                           float3 WorldPos, float Length, out float visibility )
{
    const int SunRaySampleCount = CLOUD_RM_STEPS;
    float3    ViewDir           = normalize( WorldPos - ViewPos );
    float3    SunPos            = ViewPos - LightDir * 1000.0f;
    float     Jitter            = Bayer4x4( i.vPosition.xy );
    float3    start_pos;
    float3    end_pos;
    visibility = 0.0f;
    if ( !DetermineStartAndEndRMPoints( ViewPos, WorldPos, CloudStartHeight,
                                        CloudEndHeight, start_pos, end_pos ) )
        return float3( 0, 0, 0 );
    if ( dot( start_pos - WorldPos, ViewDir ) > 0 )
        return float3( 0, 0, 0 );
    if ( dot( WorldPos - end_pos, ViewDir ) < 0 )
        end_pos = WorldPos;
    Length            = length( end_pos - start_pos );
    float  StepLength = Length / (float)SunRaySampleCount;
    float3 Step       = ViewDir * StepLength * Jitter;
    float3 CurrentPos = start_pos + Step;

    float3 ResultColor = 0.0f.xxx;
    float  depth       = 0;
    for ( int index = 0; index < SunRaySampleCount; index++ )
    {
        float density_falloff = 1 - saturate( length( ViewPos - CurrentPos ) /
                                              ( fFarClip + CloudEndHeight ) );
        float density = SampleCloudsDensityAt( CurrentPos ) * density_falloff;
        if ( density > 0 )
        {
            float cos_angle        = dot( ViewDir, LightDir );
            float density_to_sun   = MarchLight( CurrentPos, -LightDir );
            float attenuation_prob = Attenuation( density_to_sun, cos_angle );
            float ambient_out_scatter = OutScatterAmbient( density, 0 );
            float sun_highlight       = InOutScatter( cos_angle );
            float attenuation =
                ( attenuation_prob * ambient_out_scatter * sun_highlight );

            attenuation = max(
                density * 0.5f *
                    ( 1 - pow( saturate( StepLength * ( index + 1 ) / 5000.0f ),
                               2 ) ),
                attenuation );
            ResultColor += attenuation * vSunColor.rgb * vSunLightDir.a;
            depth += density;
        }
        CurrentPos += Step;
    }
    // if( depth > 0.0f)
    ResultColor /= (float)SunRaySampleCount;
    ResultColor += ( Beer( depth ) ) * CloudsColor.rgb *
                   ( depth > 0 ? 1.0f : 0.0f ) * 0.33f * CloudsColor.a;
    ResultColor += ( OutScatterAmbient( depth, 0 ) ) * vSkyLightCol.rgb *
                   0.33f * ( depth > 0 ? 1.0f : 0.0f );
    // outscatter
    // ResultColor *= (1- saturate((depth) )) * vSkyLightCol.rgb;
    visibility = Beer( depth ) * ( depth > 0 ? 1.0f : 0.0f );
    return ResultColor;
}
void GetNormalsAndInfDepth( Texture2D TexBuffer, SamplerState Sampler,
                            float2 TexCoords, out float ViewDepth,
                            out float3 Normals )
{
    float4 NormalSpec = TexBuffer.SampleLevel( Sampler, TexCoords, 0 );

    ViewDepth = DecodeFloatRG( NormalSpec.zw );
    ViewDepth = ViewDepth <= 0 ? 100000.0f : ViewDepth;
    Normals   = DecodeNormals( NormalSpec.xy );
    // transform normals back to world-space
    Normals = mul( Normals, (float3x3)mViewInv );
}

float4 RenderCloudsPS( PS_QUAD_IN i ) : SV_TARGET
{
    float4 OutLighting;

    const float3 ViewPos = mViewInv[3].xyz;

    // Retrieve all needed buffer samples first
    float  ViewZ;
    float3 Normals;
    GetNormalsAndInfDepth( txGB1, samLinear, i.vTexCoord.xy, ViewZ, Normals );

    float3 WorldPos = DepthToWorldPos( ViewZ, i.vTexCoord.xy ).xyz;

    // Directions calculation maybe we should introduce macroses to encapsulate
    // them
    float3 ViewDir  = normalize( WorldPos - ViewPos );
    float3 LightDir = normalize( vSunLightDir.xyz );
    float  vis      = 0;
    OutLighting.rgb = ComputeCloudsAtDir(
        i, LightDir, ViewPos, WorldPos,
        min( length( WorldPos - ViewPos ), RaymarchingDistance ),
        vis ); // SunRays * SunlightIntensity;
    OutLighting.a = vis;

    return OutLighting;
}

float4 CloudsCombinePS( PS_QUAD_IN i ) : SV_TARGET
{
    float4 OutLighting;

    float4 ScreenColor = txScreen.Sample( samLinear, i.vTexCoord.xy );
    float  intesity =
        saturate( txVolumetric.Sample( samLinear, i.vTexCoord.xy ).a );
    OutLighting.rgb =
        lerp( ScreenColor.rgb,
              txVolumetric.Sample( samLinear, i.vTexCoord.xy ).rgb, intesity );
    OutLighting.a = 1;

    return OutLighting;
}