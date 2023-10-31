#version 330 core
out vec4 FragColor;

struct Light {
    vec3 position;
    vec3 color;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in mat3 TBN;
  
uniform vec3 viewPos;

uniform Light light;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D roughnessMap;
uniform sampler2D ambientOcclusionMap;
uniform float uMetallic;

const float PI = 3.14159265359;
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
    for(int i = 0; i < 1; ++i) {
        
        vec3 L = normalize(light.position - FragPos);
        vec3 H = normalize(V + L);
        float distance = length(light.position - FragPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = light.color * attenuation;

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

    vec3 TangentLightPos = TBN * light.position;
    vec3 TangentViewPos  = TBN * viewPos;
    vec3 TangentFragPos  = TBN * FragPos;

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