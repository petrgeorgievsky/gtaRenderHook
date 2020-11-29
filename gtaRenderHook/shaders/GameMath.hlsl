#include "Globals.hlsl"
#ifndef GAMEMATH
#define GAMEMATH
#define DO_ALPHA_TEST( alpha )                                                 \
    if ( uiAlphaTestType == 1 )                                                \
    {                                                                          \
        if ( alpha <= fAlphaTestRef )                                          \
            discard;                                                           \
    }                                                                          \
    else if ( uiAlphaTestType == 2 )                                           \
    {                                                                          \
        if ( alpha < fAlphaTestRef )                                           \
            discard;                                                           \
    }                                                                          \
    else if ( uiAlphaTestType == 3 )                                           \
    {                                                                          \
        if ( alpha >= fAlphaTestRef )                                          \
            discard;                                                           \
    }                                                                          \
    else if ( uiAlphaTestType == 4 )                                           \
    {                                                                          \
        if ( alpha > fAlphaTestRef )                                           \
            discard;                                                           \
    }                                                                          \
    else if ( uiAlphaTestType == 5 )                                           \
    {                                                                          \
        if ( alpha == fAlphaTestRef )                                          \
            discard;                                                           \
    }                                                                          \
    else if ( uiAlphaTestType == 6 )                                           \
    {                                                                          \
        if ( alpha != fAlphaTestRef )                                          \
            discard;                                                           \
    }                                                                          \
    else if ( uiAlphaTestType == 0 )                                           \
    {                                                                          \
        alpha = 1;                                                             \
    }
//-----------------------------------------VOXEL
//STUFF(UNUSED)-----------------------------------------------------
// Converts from world space to voxel space. TODO: use matrices
float3 ConvertToVoxelSpace( float3 p )
{
    float3 op =
        ( ( p - mViewInv[3].xyz ) * voxelGridScale +
          float3( voxelGridSize, voxelGridSize, voxelGridSize ) / ( 2 ) );
    return op;
}
// Converts from voxel space to world space. TODO: use matrices
float3 ConvertFromVoxelSpace( float3 p )
{
    float3 op = ( ( p - float3( voxelGridSize, voxelGridSize, voxelGridSize ) /
                            ( 2 ) ) /
                  voxelGridScale ) +
                mViewInv[3].xyz;
    return op;
}
// Converts from world space to voxel space. TODO: use matrices
float3 ConvertToVoxelSpace( float3 p, float scale )
{
    float3 op =
        ( ( p - mViewInv[3].xyz ) * scale +
          float3( voxelGridSize, voxelGridSize, voxelGridSize ) / ( 2 ) );
    return op;
}
// Converts from voxel space to world space. TODO: use matrices
float3 ConvertFromVoxelSpace( float3 p, float scale )
{
    float3 op = ( ( p - float3( voxelGridSize, voxelGridSize, voxelGridSize ) /
                            ( 2 ) ) /
                  scale ) +
                mViewInv[3].xyz;
    return op;
}
//-----------------------------------------------------------------------------------------------------------------

// Converts uint to rgba.
float4 UINTtoRGBA( uint color )
{
    if ( color == 0xffffffff )
        return float4( 1, 1, 1, 1 );

    float a = ( ( color & 0xff000000 ) >> 24 );

    float r = ( ( color & 0xff0000 ) >> 16 );

    float g = ( ( color & 0xff00 ) >> 8 );

    float b = ( ( color & 0xff ) );
    return float4( r, g, b, a ) / 255.0f;
}

float3 CosineSampleHemisphere( float u, float v )
{
    float phi      = v * 2.0 * 3.14;
    float cosTheta = sqrt( 1.0 - u );
    float sinTheta = sqrt( 1.0 - cosTheta * cosTheta );
    return float3( cos( phi ) * sinTheta, sin( phi ) * sinTheta, cosTheta );
}

float4 DepthToViewPos( float depth, float2 texCoord )
{
    float  x = texCoord.x * 2 - 1;
    float  y = ( 1 - texCoord.y ) * 2 - 1;
    float2 screenSpaceRay =
        float2( x / mProjection[0].x, y / mProjection[1].y );
    float4 pos = float4( screenSpaceRay * depth, depth, 1.0 );
    return pos;
}

float3 DepthToWorldPos( float depth, float2 texCoord )
{
    float4 pos = DepthToViewPos( depth, texCoord );
    pos        = mul( pos, mViewInv );
    return pos.xyz;
}

float GetLuminance( float3 Color ) { return dot( g_vLuminance, Color ); }

float Bayer4x4( float2 index )
{
    const float4x4 Bayer = { float4( 1, 13, 4, 16 ), float4( 9, 5, 12, 8 ),
                             float4( 3, 15, 2, 14 ), float4( 11, 7, 10, 6 ) };
    return Bayer[index.x % 4][index.y % 4] * 0.0625;
}

// expects integer-stepped values (e.g. position)
float InterleavedGradientNoise( float2 index )
{
    return frac( frac( dot( index.xy, float2( 0.06711056, 0.00583715 ) ) ) *
                 52.9829189 );
}

#endif