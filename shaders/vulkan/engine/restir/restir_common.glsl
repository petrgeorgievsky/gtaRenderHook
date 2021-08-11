
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

Reservoir newReservoir() {
    Reservoir reservoir;

	reservoir.totalWeight = 0;
	reservoir.selectedLightWeight = 0.0f;
	reservoir.visitedSampleCount = 0;
	reservoir.selectedLightId = 0;

	return reservoir;
}

Reservoir updateReservoir(Reservoir reservoir, int lightId, float weight, inout uint randSeed) {

	// Algorithm 2 of ReSTIR paper
	//if(isnan(reservoir.totalWeight))
	//    reservoir.totalWeight = 0;
	//if(weight <= 0) return reservoir;

	reservoir.totalWeight = reservoir.totalWeight + weight;
	reservoir.visitedSampleCount = reservoir.visitedSampleCount + 1;

	// randomly sample new light
	if (nextRand(randSeed) < weight / reservoir.totalWeight)
	{
		reservoir.selectedLightId = lightId;
	}

	return reservoir;
}