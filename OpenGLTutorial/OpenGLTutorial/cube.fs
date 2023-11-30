#version 430 core
out vec4 FragColor;

#define N_LIGHTS 4

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in mat3 TBN;
  
uniform vec3 viewPos;

uniform int light_n;
uniform int lightID[N_LIGHTS];
uniform vec3 lightDirection[N_LIGHTS];
uniform vec3 lightPosition[N_LIGHTS];
uniform vec3 lightColor[N_LIGHTS];

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D roughnessMap;
uniform sampler2D ambientOcclusionMap;
uniform float uMetallic;

const float PI = 3.14159265359;

// pointlight 
float calculateAttenuation(int indx){
    
    if(lightID[indx] != 2) return 1.0;

    float attenuation = 1.0;
    float d = length(lightPosition[indx] - FragPos);   // distance position and vertex
//  float Kc = max(light[indx].constant, 1.0);          // constant value, default = 1.0
//  float Kl = max(light[indx].linear, 0.0);            // linear value
//  float Kq = max(light[indx].light.quadratic, 0.0);   // quadratic value
//  attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    attenuation = 1.0 / (d * d);
    return attenuation;
}

// spotlight or flashlight
float calculateIntensity(int indx){
    
    if(lightID[indx] != 3) return 1.0;

    vec3 lightDir = normalize(lightPosition[indx] - FragPos);
    // float theta = dot(lightDir, normalize(-lightDirection[indx])); 
    //float epsilon = light[indx].cutOff - light[indx].outerCutOff;
    //float intensity = clamp((theta - light[indx].outerCutOff) / epsilon, 0.0, 1.0);
    float intensity = 1.0;
    return intensity;
}

vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(FragPos);
    vec3 Q2  = dFdy(FragPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 tbn = mat3(T, B, N);

    return normalize(tbn * tangentNormal);
}
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
vec3 PBR(vec3 albedo, float metallic, float roughness, float ao){

    vec3 N = getNormalFromMap();
    vec3 V = normalize(viewPos - FragPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < light_n; ++i) {
        
        vec3 L = normalize(lightPosition[i] - FragPos);
        if(lightID[i] == 1){
            L = normalize(-lightDirection[i]);
        }

        vec3 H = normalize(V + L);
        float attenuation = calculateAttenuation(i);
        float intensity = calculateIntensity(i);
        vec3 radiance = lightColor[i] * attenuation * intensity;

        // Cook-Torrance BRDF
        float D = DistributionGGX(N, H, roughness);   
        float G = GeometrySmith(N, V, L, roughness);      
        vec3 F  = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator    = D * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 fSpecular = numerator / denominator;

        vec3 fDiffuse = albedo / PI;
        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
        vec3 Fr = (kD * fDiffuse) + fSpecular;

        float NdotL = max(dot(N, L), 0.0);
        Lo += Fr * radiance * NdotL;
    }

    return Lo;
}

void main() {

  // vec3 TangentLightPos = TBN * light.position;
  // vec3 TangentViewPos  = TBN * viewPos;
  // vec3 TangentFragPos  = TBN * FragPos;

    vec3 albedo     = pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));
    //float metallic  = texture(metallicMap, TexCoords).r;
    float roughness = texture(roughnessMap, TexCoords).r;
    float ao        = 1.0;
    // float ao        = texture(aoMap, TexCoords).r;

    vec3 Lo = PBR(albedo, uMetallic, roughness, ao);
    
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo;
    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
} 