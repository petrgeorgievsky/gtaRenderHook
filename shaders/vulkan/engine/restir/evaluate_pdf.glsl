
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
float EvaluatePDF(int light_id, SurfacePoint surface)
{
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

        return (lightIntensity * brdf) * (LightsCount+1);
    }
    // Analytic lights
    else if(light_id < LightsCount)
    {
        AnalyticLight pl = SceneLights[light_id];

        float lightIntensity = length(pl.color.rgb);
        vec3 lightDir = (pl.posAndRadius.xyz - surface.worldPos.xyz);

        float brdf = EvaluateLambertBrdf(surface, lightDir); // lambertian term
        float g = PointLightGeometryTerm(surface, pl);

        // p_hat of the light is Le * f * G / pdf, pdf is 1 / lights count
        return (lightIntensity * brdf * g) * (LightsCount+1);
    }
    // Triangle lights
    else
        return 0.0f;
}