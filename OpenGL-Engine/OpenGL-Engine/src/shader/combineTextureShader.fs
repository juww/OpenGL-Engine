#version 430 core

out vec3 FragColor;

in vec2 TexCoords;

uniform bool useMetallicMapping;
uniform bool useOcclusionMapping;

uniform sampler2D roughnessMap;
uniform sampler2D metallicMap;
uniform sampler2D occlusionMap;

void main() {
    
    float roughness = 0.0;
    float metallic = 0.0;
    float occlusion = 1.0;

    vec3 mro = texture(roughnessMap, TexCoords).rgb;

    //occlusion = mro.r;
    roughness = mro.g;
    metallic = mro.b;

    if(useMetallicMapping) {
        metallic = texture(metallicMap, TexCoords).r;
    }
    if(useOcclusionMapping) {
        occlusion = texture(occlusionMap, TexCoords).r;
    }

    
    FragColor = vec3(occlusion, roughness, metallic);
}