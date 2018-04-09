//#define PBR_LIGTHING
// TODO: Cleanup this mess, and do some research
#ifndef LIGHTING_FUNCTIONS
#define LIGHTING_FUNCTIONS
static const float PI = 3.14;

inline float pow5(float v) {
	return v*v*v*v*v;
}

// Microfacet normal distribution function(Beckmann) see http://graphicrants.blogspot.ru/2013/08/specular-brdf-reference.html for details
float MicrofacetNDFBeckmann(in float3 vNormal,in float3 vHalfWay,in float fRoughness){
	float fRoughnessSqr	= fRoughness*fRoughness;

	float fCosAlphaSqr 	= dot(vNormal,vHalfWay);// Alpha is angle between normal and half-way vector
	fCosAlphaSqr 		*= fCosAlphaSqr;		// need to check if can be optimized to use less instructions

	float fNDCoeff		= exp((fCosAlphaSqr-1.0f)/(fRoughnessSqr*fCosAlphaSqr));
	fNDCoeff			/= PI*fRoughnessSqr*fCosAlphaSqr*fCosAlphaSqr;

	return fNDCoeff;
	//float 
}
float MicrofacetNDF_GGX(in float3 vNormal, in float3 vHalfWay, in float fRoughness) {
	float fRoughnessSqr = fRoughness*fRoughness;

	float fCosAlphaSqr = saturate(dot(vNormal, vHalfWay));// Alpha is angle between normal and half-way vector
	//fCosAlphaSqr *= fCosAlphaSqr;		// need to check if can be optimized to use less instructions
	float fDiv = ((fRoughnessSqr - 1)*fCosAlphaSqr + 1);
	float fNDCoeff = fRoughnessSqr;
	fNDCoeff /= PI*(fDiv*fDiv+ 1e-7f);

    half d = (fCosAlphaSqr * fRoughnessSqr - fCosAlphaSqr) * fCosAlphaSqr + 1.0f; // 2 mad
    return fRoughnessSqr / (d * d + 1e-7f);
	//return fNDCoeff;
	//float 
}
// Microfacet normal geometric shadowing function(Schlick-Beckmann)
float MicrofacetGeometricShadow(in float3 vNormal,in float3 vViewDir,in float fRoughness){
	float fRoughnessInt	= fRoughness*fRoughness/2;// integral of f(r)=r, where r - roughness
	float fCosAlpha 	= saturate(dot(vNormal,vViewDir));// Alpha is angle between normal and viewer direction
	float fShadowCoeff	= fCosAlpha/(fCosAlpha*(1-fRoughnessInt)+fRoughnessInt);
	return fShadowCoeff;
	//float 
}
float GGX_PartialGeometry(float cosThetaN, float alpha)
{
    float cosTheta_sqr = saturate(cosThetaN * cosThetaN);
    float tan2 = (1 - cosTheta_sqr) / cosTheta_sqr;
    float GP = 2 / (1 + sqrt(1 + alpha * alpha * tan2));
    return GP;
}
float MicrofacetGeometricShadow_Sh(in float NdotL, in float NdotV, in float fRoughness) {
	float k = 0.797884560802865*fRoughness*fRoughness;
	float gL = NdotL * (1 - k) + k;
	float gV = NdotV * (1 - k) + k;
	return 1.0 / (gL * gV + 1e-5f);
	//float 
}
inline float SmithJointGGXVisibilityTerm(float NdotL, float NdotV, float roughness)
{
	// Original formulation:
	//	lambda_v	= (-1 + sqrt(a2 * (1 - NdotL2) / NdotL2 + 1)) * 0.5f;
	//	lambda_l	= (-1 + sqrt(a2 * (1 - NdotV2) / NdotV2 + 1)) * 0.5f;
	//	G			= 1 / (1 + lambda_v + lambda_l);

	// Reorder code to be more optimal
	float a = roughness;
	float a2 = a * a;

	float lambdaV = NdotL * sqrt((-NdotV * a2 + NdotV) * NdotV + a2);
	float lambdaL = NdotV * sqrt((-NdotL * a2 + NdotL) * NdotL + a2);

	// Simplify visibility term: (2.0f * NdotL * NdotV) /  ((4.0f * NdotL * NdotV) * (lambda_v + lambda_l + 1e-5f));
	return 0.5f / (lambdaV + lambdaL + 1e-5f);
}
// Totally microfacet, so true, much not false(need to think about it in spare time, 
// model some microfacet surface, and approx frensel coeff over small area)
float MicrofacetFresnel(in float3 LightDir, in float3 Normal, in float fRoughness) {
	float IOR = 1.5f;
    float f0 = (1 - IOR) / (1 + IOR);
	f0 *= f0;
    // Cosine between light and normal
    float CosPhi = max(dot(LightDir, Normal), 0);

    return f0 + (1 - f0) * pow5(1 - CosPhi);
}
float MicrofacetSpecular(in float3 vNormal,in float3 vLightDir,in float3 vViewDir,in float fRoughness){

	return 0.0f;
	//float 
}
// Basic lambertian diffuse term, Normal and Light Direction should be normalized
inline float LambertDiffuse(in float3 vNormal,in float3 vLightDir){
	float fCosA = dot(vNormal, vLightDir); // cosine of angle between normal and light vector
	return max(fCosA,0.0f);
}
// Basic Phong specular term
float PhongSpecular(in float3 vNormal,in float3 vLightDir,in float3 vViewDir,in float fRoughness){
	float specPower = pow(2.0f, saturate(1.0f - fRoughness) * 12.0f);
	
	float3 vReflectDir = reflect(vLightDir, vNormal); // reflection vector
    float fCosPhi = max(dot(vReflectDir, vViewDir), 0.0);// cosine of angle between reflection vector and viewer direction
	
	return pow(fCosPhi, specPower/4.0);
}
void CalculateDiffuseTerm(in float3 vNormal,in float3 vLightDir,out float fDiffuseTerm,in float fRoughness){
	float fLambert = LambertDiffuse(vNormal, vLightDir);
	fDiffuseTerm = fLambert;
}

void CalculateDiffuseTerm_ViewDependent(in float3 vNormal, in float3 vLightDir, in float3 vViewDir, out float fDiffuseTerm, in float fRoughness) {
	float fLambert = LambertDiffuse(vNormal, vLightDir);
	float fFL = pow5(1 - saturate(dot(vNormal, vLightDir)));
	float fFV = pow5(1 - saturate(dot(vNormal, vViewDir)));
	float3 vHalfWay = normalize(vLightDir + vViewDir);
	
	float LdotH = saturate(dot(vHalfWay, vLightDir));

	float fd90 = 0.5 + 2 * LdotH * LdotH * fRoughness;
	// Two schlick fresnel term
	float lightScatter = (1 + (fd90 - 1) * fFL);
	float viewScatter = (1 + (fd90 - 1) * fFV);
	fDiffuseTerm = (lightScatter * viewScatter )*fLambert;
}
float GGX_Distribution(float cosThetaNH, float alpha)
{
    float alpha2 = alpha * alpha;
    float NH_sqr = saturate(cosThetaNH * cosThetaNH);
    float den = NH_sqr * alpha2 + (1.0 - NH_sqr);
    return alpha2 / (PI * den * den);
}

void CalculateSpecularTerm(in float3 vNormal,in float3 vLightDir,in float3 vViewDir,in float fRoughness,out float fSpecularTerm){
	float3 vHalfWay = normalize(vLightDir + vViewDir);
	float fresnelTerm = MicrofacetFresnel(vLightDir, vHalfWay, fRoughness);
    float roug_sqr = fRoughness * fRoughness;
    float G = GGX_PartialGeometry(dot(vNormal, vViewDir), roug_sqr) * GGX_PartialGeometry(dot(vNormal, vLightDir), roug_sqr);
    float D = GGX_Distribution(dot(vNormal, vHalfWay), roug_sqr);
    float ndfTerm = MicrofacetNDF_GGX(vNormal, vHalfWay, fRoughness*fRoughness);
	float ndl = saturate(dot(vNormal, vLightDir));
    float ndv = saturate(dot(vNormal, vViewDir));
    float visibilityTerm = SmithJointGGXVisibilityTerm(ndl, ndv, fRoughness * fRoughness);
    /*if (ndv <= 0.0 || ndl <= 0.0)
    {
        fSpecularTerm = 0.0f;
        return;
    }*/
	//MicrofacetGeometricShadow(vNormal, vViewDir, fRoughness);
#if USE_PBR==1
    fSpecularTerm = ndfTerm * 0.25f;
#else
    fSpecularTerm = PhongSpecular(vNormal, vLightDir, -vViewDir, fRoughness)*4.0f;
#endif
    //
    //ndfTerm * fresnelTerm;
}
#endif