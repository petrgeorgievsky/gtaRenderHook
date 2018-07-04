#pragma once
/*
	General idea how we are going to implement clouds later:
	1) Generate tileable 3d noise textures(worley noise, recurrent Perlin noise)
	2) Generate random points at some heights(n-layers) and move them along game wind direction,
		these points could serve as attractors for clouds noise making it more realistic.
	3) Generate cloud depth volume from above data
	4) Raymarch through cloud box/plane using generated cloud depth volume
	5) Alternativly draw clouds as distant sprites with simple normal maps to simulate lighting
	Should cost about 1-4 ms idk how bad it would be once we implement it.
*/
class CCloudsRH
{
public:
	static void Render();
};

