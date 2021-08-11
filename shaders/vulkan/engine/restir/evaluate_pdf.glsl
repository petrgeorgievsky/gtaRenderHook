
float EvaluateLambertBrdf(SurfacePoint surface, vec3 lightDir)
{
    return min(max(dot(surface.normal, normalize(lightDir.xyz)), 0), 1);
}

/// Calculates geometry term approximation for analytic light
float PointLightGeometryTerm(SurfacePoint surface, AnalyticLight pl)
{
    vec3 lightDir = (pl.posAndRadius.xyz - surface.worldPos.xyz);
    float distToLight = length(lightDir);
    if(distToLight > 0.001f)
        lightDir /= distToLight;

    // So far it uses somewhat incorrect geometry term TODO: Replace it with sphere light one
    float g = 1.0f - pow(min(max(distToLight / pl.posAndRadius.w, 0.0f), 1.0f), 2);//= 2*(1-(distance_to_light/sqrt(distance_to_light*distance_to_light + lr_sq)))/(lr_sq);
    g *= g;

    // TODO: research in finding somewhat correct geometry trem for spot lights
    float spot_cutoff = pl.dirAndAttenuation.w;
    if( spot_cutoff > 0 )
    {
        float spot_angle = dot(-lightDir.xyz, pl.dirAndAttenuation.xyz);
        float spot_attenuation = 0.0f;
        if (spot_angle >= spot_cutoff)
            spot_attenuation = clamp((spot_angle - spot_cutoff)/0.1f, 0.0f, 1.0f);

        g*=spot_attenuation;
    }

    return g;
}

/// Evaluates approximation of light PDF - p_hat in ReSTIR paper
float EvaluatePDF(int light_id, SurfacePoint surface, inout uint randSeed)
{
	float p_hat = 0.0f;
    // Main directional light
    if(light_id < 0)
    {
        vec3 lightDir = SunDir.xyz;
        float lightIntensity = 1.0f;
        if (lightDir.z < 0) {
            lightDir.z = -lightDir.z;
            lightIntensity = 0.15f;
        }
        float brdf = EvaluateLambertBrdf(surface, lightDir);

        p_hat = (lightIntensity * brdf);
    }
    // Analytic lights
    else if(light_id < LightsCount)
    {
        AnalyticLight pl = UnpackLight(SceneLights[light_id]);

        float lightIntensity = length(pl.color.rgb);
        vec3 lightDir = (pl.posAndRadius.xyz - surface.worldPos.xyz);

        float brdf = EvaluateLambertBrdf(surface, lightDir); // lambertian term
        float g = PointLightGeometryTerm(surface, pl);

        // p_hat of the light is Le * f * G / pdf, pdf is 1 / lights count
        p_hat = (lightIntensity * brdf * g);
    }
    // Triangle lights
    else
    {
        int triangleId = (light_id-int(LightsCount)) % (TriLightsCount);

        TriangleLight triLight = UnpackTriLight(SceneTriLights[triangleId]);

        vec2 attribs = vec2(nextRand(randSeed),nextRand(randSeed));

        const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

        vec3 obj_pos = triLight.v0.xyz * barycentrics.x +
                       triLight.v1.xyz * barycentrics.y +
                       triLight.v2.xyz * barycentrics.z;
        vec4 world_pos = ((vec4(obj_pos, 1.0) * scnDesc.i[triLight.instanceId].transfo));
        vec3 lNormal = cross(triLight.v1.xyz - triLight.v0.xyz,triLight.v2.xyz - triLight.v0.xyz);
        lNormal = normalize(lNormal);
        //float tri_area = length();

        vec3 lightDir = (world_pos.xyz - surface.worldPos.xyz);
        float distToLight = length(lightDir);
        if(distToLight > 0.001f)
            lightDir /= distToLight;

        float brdf = EvaluateLambertBrdf(surface, lightDir); // lambertian term
        float g = (1.0f/(distToLight * distToLight)) * max(dot(lNormal, -lightDir),0.0f);

        // p_hat of the light is Le * f * G / pdf, pdf is 1 / lights count
        p_hat = (triLight.intensity * brdf * g);
    }
	return p_hat;
}