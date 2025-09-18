#version 430 core

in vec2 TexCoords;

//out vec4 FragColor;
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gNormalMap;
uniform sampler2D gORMMap;
uniform sampler2D gTexCoords;
uniform sampler2D ssaoBuffer;

uniform bool useSSAO;
uniform bool useHDR;
uniform bool useORM;

uniform float ambientStrenght;
uniform float exposure;

uniform samplerCube irradianceMap;
uniform samplerCube preFilterMap;
uniform sampler2D brdfLUTTexture;

uniform float far_plane;

uniform vec3 lightDirection;
uniform vec3 lightPosition[32];
uniform float lightRadius[32];
uniform vec3 lightColor[32];
uniform vec3 viewPos;
uniform vec4 baseColor;

uniform float lightLinear;
uniform float lightQuadratic;

uniform float subSurface;
uniform float roughnessFactor;
uniform float metallicFactor;
uniform float _Specular;
uniform float _SpecularTint;
uniform float _Sheen;
uniform float _SheenTint;
uniform float _Anisotropic;
uniform float _ClearCoatGloss;
uniform float _ClearCoat;

#define PI 3.14159265358979323846

const float GAMMA = 2.2;
const float INV_GAMMA = 1.0 / GAMMA;

vec3 TanX;
vec3 TanY;
void getNormalFromMap(vec3 FragPos, vec2 TexCoords, vec3 Normal) {

    vec3 Q1  = dFdx(FragPos);
    vec3 Q2  = dFdy(FragPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1 * st2.t - Q2 * st1.t);
    //vec3 T  = Tangent;
    vec3 B  = -normalize(cross(N, T));
    mat3 tbn = mat3(T, B, N);

    TanX = T;
    TanY = B;
}

float sqr(float x) { 
    return x * x; 
}

float DotClamped(vec3 a, vec3 b){
    float res = max(dot(a, b), 0.0);
    return res;
}

float lerp(float a, float b, float t){
    return mix(a, b, t);
}

vec3 lerp3(vec3 a, vec3 b, float t){
    return mix(a, b, t);
}

float luminance(vec3 color) {
    return dot(color, vec3(0.299f, 0.587f, 0.114f));
}

float SchlickFresnel(float x) {
    x = clamp(1.0 - x, 0.0, 1.0);
    float x2 = x * x;

    return x2 * x2 * x; // While this is equivalent to pow(1 - x, 5) it is two less mult instructions
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// Isotropic Generalized Trowbridge Reitz with gamma == 1
float GTR1(float ndoth, float a) {
    float a2 = a * a;
    float t = 1.0f + (a2 - 1.0f) * ndoth * ndoth;
    return (a2 - 1.0f) / (PI * log(a2) * t);
}

// Isotropic Geometric Attenuation Function for GGX. This is technically different from what Disney uses, but it's basically the same.
float SmithGGX(float alphaSquared, float ndotl, float ndotv) {
    float a = ndotv * sqrt(alphaSquared + ndotl * (ndotl - alphaSquared * ndotl));
    float b = ndotl * sqrt(alphaSquared + ndotv * (ndotv - alphaSquared * ndotv));

    return 0.5f / (a + b);
}

// Anisotropic Generalized Trowbridge Reitz with gamma == 2. This is equal to the popular GGX distribution.
float AnisotropicGTR2(float ndoth, float hdotx, float hdoty, float ax, float ay) {
    return 1.0 / (PI * ax * ay * sqr(sqr(hdotx / ax) + sqr(hdoty / ay) + sqr(ndoth)));
}

// Anisotropic Geometric Attenuation Function for GGX.
float AnisotropicSmithGGX(float ndots, float sdotx, float sdoty, float ax, float ay) {
    return 1.0 / (ndots + sqrt(sqr(sdotx * ax) + sqr(sdoty * ay) + sqr(ndots)));
}

float NormalDistributionFunction(float ndoth, vec3 H, float alpha){

    float a2 = alpha * alpha;

    float aspectRatio = sqrt(1.0f - _Anisotropic * 0.9f);
    float alphaX = max(0.001f, a2 / aspectRatio);
    float alphaY = max(0.001f, a2 * aspectRatio);
    float Ds = AnisotropicGTR2(ndoth, dot(H, TanX), dot(H, TanY), alphaX, alphaY);

    return Ds;
}

 float GeometricAttenuation(float roughness, float ndotl, float ndotv, vec3 L, vec3 V){

    float GalphaSquared = sqr(0.5f + roughness * 0.5f);
    float aspectRatio = sqrt(1.0f - _Anisotropic * 0.9f);

    float GalphaX = max(0.001f, GalphaSquared / aspectRatio);
    float GalphaY = max(0.001f, GalphaSquared * aspectRatio);

    float G = AnisotropicSmithGGX(ndotl, dot(L, TanX), dot(L, TanY), GalphaX, GalphaY);
    G *= AnisotropicSmithGGX(ndotv, dot(V, TanX), dot (V, TanY), GalphaX, GalphaY); 

    // specular brdf denominator (4 * ndotl * ndotv) is baked into output here (I assume at least)  
    return G;
}

vec3 DisneyBRDF(vec3 V, vec3 N, vec3 surfaceColor, vec3 L, float Roughness, float Metallic){

    vec3 H = normalize(L + V); // Microfacet normal of perfect reflection

    float ndotl = DotClamped(N, L);
    float ndotv = DotClamped(N, V);
    float ndoth = DotClamped(N, H);
    float ldoth = DotClamped(L, H);

    float FL = SchlickFresnel(ndotl);
    float FV = SchlickFresnel(ndotv);
    float FH = SchlickFresnel(ldoth);

    // diffuse
    float Fss90 = ldoth * ldoth * Roughness;
    float Fd90 = 0.5 + 2.0 * Fss90;

    float Fd = lerp(1.0, Fd90, FL) * lerp(1.0, Fd90, FV);

     // Subsurface Diffuse (Hanrahan-Krueger brdf approximation)
    float Fss = lerp(1.0, Fss90, FL) * lerp(1.0, Fss90, FV);
    float ss = 1.25 * (Fss * ((1.0 / (ndotl + ndotv)) - 0.5) + 0.5);

    float Cdlum = luminance(surfaceColor);

    vec3 Ctint = Cdlum > 0.0 ? surfaceColor / Cdlum : 1.0;
    vec3 Cspec = vec3(_Specular * 0.08 * lerp3(vec3(1.0), Ctint, _SpecularTint));
    vec3 Cspec0 = lerp3(Cspec, surfaceColor, Metallic);
    vec3 Csheen = lerp3(vec3(1.0), Ctint, _SheenTint);

     // Sheen
    vec3 FSheen = FH * _Sheen * Csheen;

    //specular
    float D = NormalDistributionFunction(ndoth, H, Roughness);
    vec3 F = lerp3(Cspec0, vec3(1.0f), FH);
    float G = GeometricAttenuation(Roughness, ndotl, ndotv, L, V);

    // Clearcoat (Hard Coded Index Of Refraction -> 1.5f -> F0 -> 0.04)
    float Dr = GTR1(ndoth, lerp(0.1f, 0.001f, _ClearCoatGloss)); // Normalized Isotropic GTR Gamma == 1
    float Fr = lerp(0.04, 1.0f, FH);
    float Gr = SmithGGX(ndotl, ndotv, 0.25f);

    vec3 FDiffuse = (1.0 / PI) * (lerp(Fd, ss, subSurface) * surfaceColor + FSheen) * (1.0 - Metallic) * (1.0 - F);
    vec3 FSpecular = (D * G * F);
    vec3 FClearCoat = vec3(0.25f * _ClearCoat * Gr * Fr * Dr);

    vec3 result = (FDiffuse + FSpecular + FClearCoat);
    //result = vec3(FSpecular);

    return result;
}

vec3 LINEARtoSRGB(vec3 color) {
    return pow(color, vec3(INV_GAMMA));
}

vec4 SRGBtoLINEAR(vec4 srgbIn) {
    return vec4(pow(srgbIn.xyz, vec3(GAMMA)), srgbIn.w);
}

vec3 toneMapACES(vec3 color) {
    const float A = 2.51;
    const float B = 0.03;
    const float C = 2.43;
    const float D = 0.59;
    const float E = 0.14;
    return LINEARtoSRGB(clamp((color * (A * color + B)) / (color * (C * color + D) + E), 0.0, 1.0));
}

void main (){

    vec3 surfaceColor = baseColor.rgb;
    vec3 emissive = vec3(0.0);
    float Roughness = roughnessFactor;
    float Metallic = metallicFactor;
    float ao = 1.0f;

    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedo, TexCoords).rgb;
    vec3 normalMap = texture(gNormalMap, TexCoords).rgb;
    vec3 ORM = texture(gORMMap, TexCoords).rgb;
    vec2 tex = texture(gTexCoords, TexCoords).rg;
    float ssao = texture(ssaoBuffer, TexCoords).r;

    getNormalFromMap(FragPos, tex, Normal);

    if(useORM){
        Roughness = ORM.g;
        Metallic = ORM.b;
        ao = ORM.r;
    }
    if(useSSAO){
        ao = ao * ssao;
    }
    surfaceColor = Albedo;
    vec3 L = normalize(lightDirection);
    vec3 V = normalize(viewPos - FragPos);

    vec3 Lo = vec3(0.0);
    float ndotl = DotClamped(normalMap, L);
    // light direction
    //Lo += DisneyBRDF(V, normalMap, surfaceColor, L, Roughness, Metallic);

    for(int i = 0; i < 32; i++) {
        float distance = length(lightPosition[i] - FragPos);
        if(distance > lightRadius[i]){
            continue;
        }
        L = normalize(lightPosition[i] - FragPos);
        ndotl = DotClamped(normalMap, L);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance     = lightColor[i] * attenuation;
        Lo += (DisneyBRDF(V, normalMap, surfaceColor, L, Roughness, Metallic) * radiance * ndotl);
        //Lo += radiance;
    }
    
    //ambient IBL
    vec3 R = reflect(-V, normalMap); 
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, surfaceColor, Metallic);

    vec3 F = fresnelSchlickRoughness(max(dot(normalMap, V), 0.0), F0, Roughness);
    
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - Metallic;

    vec3 irradiance = texture(irradianceMap, normalMap).rgb;
    vec3 IBLDiffuse = irradiance * surfaceColor * ambientStrenght;

    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(preFilterMap, R,  Roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(brdfLUTTexture, vec2(max(dot(normalMap, V), 0.0), Roughness)).rg;
    vec3 IBLspecular = prefilteredColor * (F * brdf.x + brdf.y);
    vec3 metallicness = lerp3(vec3(0.0), IBLspecular, Metallic);

    vec3 ambient = (kD * IBLDiffuse + IBLspecular);
    //ambient *= ambientStrenght;

    // result += ambient * ao;
    vec3 result = (Lo + ambient + emissive) * ao;
    //vec3 result = blinnPhong();

    if(useHDR){
        vec3 mapped = toneMapACES(result);
        //result = result / (result + vec3(1.0));

        // gamma correct
        result = mapped * exposure;
    }

    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(result, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);

    FragColor = vec4(result, 1.0);
}