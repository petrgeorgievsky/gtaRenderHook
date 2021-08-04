
// Recalculates reservoir weight
Reservoir updateReservoirWeight(Reservoir r, SurfacePoint s)
{
    float pdf = EvaluatePDF(r.selectedLightId, s);

    if(pdf > 0)
        r.selectedLightWeight = (1.0f / max(pdf, 0.0001f) ) *
                                (r.totalWeight / max( r.visitedSampleCount, 1 ) );
    else
        r.selectedLightWeight = 0.0f;
    return r;
}

// Combines 2 reservoirs and recalculates weight
Reservoir combineReservoirs(Reservoir a, Reservoir b, SurfacePoint surface, inout uint seed){
    float aPdf = max( EvaluatePDF(a.selectedLightId, surface), 0.0001f );
    float bPdf = max( EvaluatePDF(b.selectedLightId, surface), 0.0001f );

    // Limit visited sample count
    b.visitedSampleCount = min(20 * a.visitedSampleCount, b.visitedSampleCount);

    Reservoir c=newReservoir();

    c = updateReservoir(c, a.selectedLightId,
                            aPdf * a.selectedLightWeight * a.visitedSampleCount,
                             seed);
    c = updateReservoir(c, b.selectedLightId,
                            bPdf * b.selectedLightWeight * b.visitedSampleCount,
                             seed);

    c.visitedSampleCount = a.visitedSampleCount + b.visitedSampleCount;

    return updateReservoirWeight(c, surface);
}