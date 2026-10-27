
// Generates a seed for a random number generator from 2 inputs plus a backoff
uint initRand(uint val0, uint val1, uint backoff)
{
	uint v0 = val0, v1 = val1, s0 = 0;

	for (uint n = 0; n < backoff; n++)
	{
		s0 += 0x9e3779b9;
		v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
		v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
	}
	return v0;
}

// Takes our seed, updates it, and returns a pseudorandom float in [0..1]
float nextRand(inout uint s)
{
	s = (1664525u * s + 1013904223u);
	return float(s & 0x00FFFFFF) / float(0x01000000);
}

// Samples a point on a triangle. Returns barycentric weights (v0, v1, v2).
vec3 SampleTriangleBarycentrics( inout uint randSeed )
{
    float r1    = nextRand( randSeed );
    float r2    = nextRand( randSeed );
    if (r1 + r2 > 1) {
      r1 = 1 - r1;
      r2 = 1 - r2;
    }
    vec2  attribs = vec2( r1, r2 );
    return vec3( 1.0 - attribs.x - attribs.y, attribs.x, attribs.y );
}

// Chooses a light index in [-1, lightCountFull - 2], where
// lightCountFull = LightsCount + TriLightsCount + 1.
int SampleLightId( inout uint randSeed, int lightCountFull )
{
    return min( int( nextRand( randSeed ) * lightCountFull ) - 1,
                lightCountFull - 1 );
}

Reservoir newReservoir() {
    Reservoir reservoir;

	reservoir.totalWeight = 0;
	reservoir.selectedLightWeight = 0.0f;
	reservoir.visitedSampleCount = 0;
	reservoir.selectedLightId = 0;
	reservoir.selectedLightSeed = 0;
	reservoir.pad0 = 0;
	reservoir.pad1 = 0;
	reservoir.pad2 = 0;

	return reservoir;
}

// 'sampleSeed' - rand seed for sampling point on a light of 'lightId', not correlated to randSeed
// 'randSeed' - rand seed to select if new weight will go to this reservoir
Reservoir updateReservoir(Reservoir reservoir, int lightId, uint sampleSeed, float weight, inout uint randSeed) {

	// Algorithm 2 of ReSTIR paper
	reservoir.totalWeight = reservoir.totalWeight + weight;
	reservoir.visitedSampleCount = reservoir.visitedSampleCount + 1;
	// randomly sample new light
	if (nextRand(randSeed) < weight / reservoir.totalWeight)
	{
		reservoir.selectedLightId = lightId;
		reservoir.selectedLightSeed = sampleSeed;
	}

	return reservoir;
}