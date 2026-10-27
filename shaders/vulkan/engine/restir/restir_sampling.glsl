// Requires restir_common.glsl (SampleLightId, newReservoir, updateReservoir)
// and evaluate_pdf.glsl (EvaluatePDF).

// Samples one light and folds it into the reservoir.
// sampleIndex keeps the sample seed distinct per iteration; randSeed drives the
// light selection and must stay independent of the sample's own random draws.
Reservoir SampleReservoirCandidate( Reservoir reservoir, SurfacePoint surface,
                                    int lightCountFull, uint sampleIndex,
                                    inout uint randSeed )
{
    const int   sampleId     = SampleLightId( randSeed, lightCountFull );
    const float lightSampleP = 1.0f / float( lightCountFull );
    const uint  sampleSeed   = initRand( randSeed, sampleIndex, 8 );
    const float p_hat        = EvaluatePDF( sampleId, surface, sampleSeed );

    return updateReservoir( reservoir, sampleId, sampleSeed,
                            p_hat / lightSampleP, randSeed );
}

// p_hat already carries the radiance magnitude (|L| * f * G), so the shading
// estimate must apply only the chromaticity of the light, never its magnitude.
vec3 ReservoirRadianceTint( vec3 radiance )
{
    return radiance / max( length( radiance ), 0.0001f );
}

// Contribution of the selected sample: the reservoir estimate W * p_hat
// modulated by visibility and the light chromaticity.
vec3 ReservoirShadowEstimate( Reservoir r, SurfacePoint surface,
                              vec3 radiance_tint, float visibility )
{
    const float FireflyThreshold = 50.0f;
    const float p_hat =
        EvaluatePDF( r.selectedLightId, surface, r.selectedLightSeed );

    return visibility * radiance_tint *
           min( r.selectedLightWeight * p_hat, FireflyThreshold ) / 3.14f;
}

// Spatial reuse may only combine reservoirs that belong to the same surface.
// Otherwise light leaks across silhouettes and depth discontinuities.
bool ReservoirNeighbourIsCompatible( vec3 center_normal, float center_depth,
                                     vec3 neighbour_normal,
                                     float neighbour_depth )
{
    const float NormalThreshold        = 0.9f;
    const float RelativeDepthThreshold = 0.1f;
    // Matches the sky threshold used by the deferred composition pass.
    const float SkyDepth = 5000.0f;

    if ( center_depth >= SkyDepth || neighbour_depth >= SkyDepth )
        return false;

    if ( dot( normalize( center_normal ), normalize( neighbour_normal ) ) <
         NormalThreshold )
        return false;

    const float depth_scale = max( abs( center_depth ), 0.001f );
    return abs( center_depth - neighbour_depth ) <=
           RelativeDepthThreshold * depth_scale;
}
