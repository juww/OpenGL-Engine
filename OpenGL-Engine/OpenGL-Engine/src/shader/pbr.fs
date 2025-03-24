#version 430 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in mat3 TBN;
in vec3 Tangent;
in vec3 Bitangent;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D roughnessMap;
uniform sampler2D metallicMap;
uniform sampler2D occlusionMap;
uniform sampler2D depthMap;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUTTexture;

uniform bool useAlbedoMapping;
uniform bool useNormalMapping;
uniform bool useRoughnessMapping;
uniform bool useMetallicMapping;
uniform bool useOcclusionMapping;
uniform bool useDepthMapping;

//uniform vec3 lightPos;
uniform vec3 lightPosition[4];
uniform vec3 viewPos;
uniform vec4 baseColor;

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

uniform float heightScale;

#define PI 3.14159265358979323846

vec3 blinnPhong();

float sqr(float x) { 
    return x * x; 
}

float DotClamped(vec3 a, vec3 b){
    float res = max(dot(a, b), 0.0);
    return res;
}

float lerp(float a, float b, float t){
    //return a * (1.0 - t) + b * t;
    //return a + (t * (b - a));
    return mix(a, b, t);
}

vec3 lerp3(vec3 a, vec3 b, float t){
    //return a * (1.0 - t) + b * t;
    //return a + (t * (b - a));
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
    float Ds = AnisotropicGTR2(ndoth, dot(H, Tangent), dot(H, Bitangent), alphaX, alphaY);

    return Ds;
}

 float GeometricAttenuation(float roughness, float ndotl, float ndotv, vec3 L, vec3 V){

    float GalphaSquared = sqr(0.5f + roughness * 0.5f);
    float aspectRatio = sqrt(1.0f - _Anisotropic * 0.9f);

    float GalphaX = max(0.001f, GalphaSquared / aspectRatio);
    float GalphaY = max(0.001f, GalphaSquared * aspectRatio);

    float G = AnisotropicSmithGGX(ndotl, dot(L, Tangent), dot(L, Bitangent), GalphaX, GalphaY);
    G *= AnisotropicSmithGGX(ndotv, dot(V, Tangent), dot (V, Bitangent), GalphaX, GalphaY); 

    // specular brdf denominator (4 * ndotl * ndotv) is baked into output here (I assume at least)  
    return G;
}

vec3 DisneyBRDF(vec3 N, vec3 surfaceColor, vec3 lightPos, float Roughness, float Metallic){
    vec3 L = normalize(lightPos - FragPos);
    vec3 V = normalize(viewPos - FragPos);

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

    vec3 metallicness = lerp3(vec3(1.0), surfaceColor, Metallic);

    // Clearcoat (Hard Coded Index Of Refraction -> 1.5f -> F0 -> 0.04)
    float Dr = GTR1(ndoth, lerp(0.1f, 0.001f, _ClearCoatGloss)); // Normalized Isotropic GTR Gamma == 1
    float Fr = lerp(0.04, 1.0f, FH);
    float Gr = SmithGGX(ndotl, ndotv, 0.25f);

    //vec3 result = blinnPhong();
    vec3 FDiffuse = (1.0 / PI) * (lerp(Fd, ss, subSurface) * surfaceColor + FSheen) * (1.0 - Metallic) * (1.0 - F);
    vec3 FSpecular = (D * G * F) * metallicness;
    vec3 FClearCoat = vec3(0.25f * _ClearCoat * Gr * Fr * Dr);

    vec3 result = (FDiffuse + FSpecular + FClearCoat) * ndotl;

    return result;
}

void main (){

    vec3 N = Normal;
    vec3 surfaceColor = baseColor.rgb;
    float Roughness = roughnessFactor;
    float Metallic = metallicFactor;
    float ao = 1.0f;
    
    if(useAlbedoMapping){
        surfaceColor = texture(albedoMap, TexCoords).rgb;
        surfaceColor *= baseColor.rgb;
    }

    if(useNormalMapping){
        N = texture(normalMap, TexCoords).rgb;
        N = N * 2.0 - 1.0;
        N = normalize(TBN * N);
    }
    
    if(useRoughnessMapping){
        Roughness = texture(roughnessMap, TexCoords).r * roughnessFactor;
    }

    if(useMetallicMapping){
        Metallic = texture(metallicMap, TexCoords).r * metallicFactor;
    }

    if(useOcclusionMapping){
        ao = texture(occlusionMap, TexCoords).r;
        //surfaceColor *= ao;
    }
    vec3 Lo = vec3(0.0);

    for(int i = 0; i < 4; i++) {
        Lo += DisneyBRDF(N, surfaceColor, lightPosition[i], Roughness, Metallic);
    }
    //ambient IBL
    vec3 V = normalize(viewPos - FragPos);
    vec3 R = reflect(-V, N); 
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, surfaceColor, Metallic);

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, Roughness);
    
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - Metallic;

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 IBLDiffuse = irradiance * surfaceColor;

    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  Roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(brdfLUTTexture, vec2(max(dot(N, V), 0.0), Roughness)).rg;
    vec3 IBLspecular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * IBLDiffuse + IBLspecular);

    // result += ambient * ao;
    vec3 result = (Lo + ambient) * ao;
    //result = result/ (result + vec3(1.0));
    // gamma correct
    //result= pow(result, vec3(1.0/2.2));

    FragColor = vec4(result, 1.0);
}

vec3 blinnPhong(vec3 lightPos) {
    
    vec3 normal = Normal;
    
    if(useNormalMapping){
        normal = texture(normalMap, TexCoords).rgb;
        normal = normal * 2.0 - 1.0;
        normal = normalize(TBN * normal);

        //normal = getNormalFromMap();
    }

    // get color
    vec3 color = texture(albedoMap, TexCoords).rgb;
    //ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * color;
    // diffuse
    vec3 lightDir = normalize(lightPos - FragPos);
    //float diff = max(dot(normal, lightDir), 0.0);
    float diff = DotClamped(normal, lightDir);
    vec3 diffuse = diff * color;
    // specular
    float specularStrength = 0.2;
    vec3 viewDir = normalize(viewPos - FragPos);
    //vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(DotClamped(normal, halfwayDir), 32.0);
    vec3 specular = vec3(0.2) * spec;


    return ambient + diffuse + specular;
}
