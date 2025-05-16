#version 430 core

in vec3 FragPos;
in vec2 texCoord;

uniform mat4 model;

uniform vec3 lightDirection;
uniform vec3 viewPos;
uniform vec4 baseColor;

uniform float _Roughness;
uniform float _Metallic;
uniform vec3 _SunIrradiance;
uniform vec3 _ScatterColor;
uniform vec3 _BubbleColor;
uniform float _HeightModifier;
uniform float _BubbleDensity;
uniform float _WavePeakScatterStrength;
uniform float _ScatterStrength;
uniform float _ScatterShadowStrength;
uniform float _EnvironmentLightStrength;

uniform sampler2D displacementTexture;
uniform sampler2D slopeTexture;
uniform samplerCube _EnvironmentMap;

out vec4 FragColor;

vec3 phong(vec3 color, vec3 Normal);

#define PI 3.14159265358979323846

float rcp(float a){
    return 1.0f / a;
}

float DotClamped(vec3 a, vec3 b){
    return clamp(dot(a, b), 0.0f, 1.0f);
}

float SmithMaskingBeckmann(vec3 H, vec3 S, float roughness) {
    float hdots = max(0.001f, DotClamped(H, S));
    float a = hdots / (roughness * sqrt(1.0f - hdots * hdots));
    float a2 = a * a;

    return a < 1.6f ? (1.0f - 1.259f * a + 0.396f * a2) / (3.535f * a + 2.181f * a2) : 0.0f;
}

float Beckmann(float ndoth, float roughness) {
    float exp_arg = (ndoth * ndoth - 1.0f) / (roughness * roughness * ndoth * ndoth);

    return exp(exp_arg) / (PI * roughness * roughness * ndoth * ndoth * ndoth * ndoth);
}

vec3 BRDF(vec3 surfaceColor, vec3 norm, float d, vec4 displacementFoam){
    vec3 L = -normalize(lightDirection);
    vec3 V = normalize(viewPos - FragPos);
    vec3 H = normalize(L + V);
    vec3 N = norm;

    float roughness = _Roughness;
    float metallic = _Metallic;
    float depth = d;
    vec3 macroNormal = vec3(0.0f, 1.0f, 0.0f);

    float LdotH = DotClamped(L, H);
    float VdotH = DotClamped(V, H);
    float NdotL = DotClamped(N, L);
    float NdotH = max(0.0001f, dot(N, H));

    float a = roughness;

    float viewMask = SmithMaskingBeckmann(H, V, a);
    float lightMask = SmithMaskingBeckmann(H, L, a);

    float G = rcp(1.0f + viewMask + lightMask);

    float eta = 1.33f;
    float R = ((eta - 1.0f) * (eta - 1.0f)) / ((eta + 1.0f) * (eta + 1.0f));
    float thetaV = acos(V.y);

    float numerator = pow(1.0f - dot(N, V), 5.0f * exp(-2.69f * a));
    float F = R + (1.0f - R) * numerator / (1.0f + 22.7f * pow(a, 1.5f));
    F = clamp(F, 0.0f, 1.0f);
    
    vec3 specular = _SunIrradiance * F * G * Beckmann(NdotH, a);
    specular /= 4.0f * max(0.001f, DotClamped(macroNormal, L));
    specular *= DotClamped(N, L);

    vec3 I = normalize(FragPos - viewPos);
    vec3 envReflection = texture(_EnvironmentMap, reflect(I, N)).rgb;
    envReflection *= _EnvironmentLightStrength;

    float waveHeight = max(0.0f, displacementFoam.y) * _HeightModifier;
    vec3 scatterColor = _ScatterColor;
    vec3 bubbleColor = _BubbleColor;
    float bubbleDensity = _BubbleDensity;

    float k1 = _WavePeakScatterStrength * waveHeight * pow(DotClamped(L, -V), 4.0f) * pow(0.5f - 0.5f * dot(L, N), 3.0f);
    float k2 = _ScatterStrength * pow(DotClamped(V, N), 2.0f);
    float k3 = _ScatterShadowStrength * NdotL;
    float k4 = bubbleDensity;

    vec3 scatter = (k1 + k2) * scatterColor * _SunIrradiance * rcp(1 + lightMask);
    scatter += k3 * scatterColor * _SunIrradiance + k4 * bubbleColor * _SunIrradiance;

    vec3 result = (1.0f - F) * scatter + specular + F * envReflection;
    result = max(vec3(0.0f), result);
    //result = mix(result, _FoamColor, clamp(foam, 0.0f, 1.0f));

    result = vec3(result);

    return result;
}

void main(){

    vec3 surfaceColor = normalize(baseColor.rgb);

    vec3 displacement = texture(displacementTexture, texCoord).rgb;
    vec2 slope = texture(slopeTexture, texCoord).rg;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 N = normalize(vec3(-slope.x, 1.0f, -slope.y));
    N = normalize(normalMatrix * N);

    vec3 result = phong(surfaceColor, N);
    float depth = 10.0f;
    vec4 displacementFoam = vec4(1.0f);
    result = BRDF(surfaceColor, N, depth, displacementFoam);

    FragColor = vec4(result, 1.0);
}

vec3 phong(vec3 color, vec3 Normal){

    vec3 lightPos = vec3(-1000.0,1000.0,-1000.0);
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * vec3(1.0);

    vec3 norm = normalize(Normal);
    //vec3 lightDir = normalize(lightPos - FragPos);
    vec3 lightDir = normalize(lightDirection);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * color;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32);
    vec3 specular = specularStrength * spec * vec3(1.0);

    return (diffuse + ambient + specular);
}
