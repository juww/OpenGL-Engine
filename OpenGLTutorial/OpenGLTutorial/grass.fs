#version 430 core

in vec3 FragPos;
in vec3 Offset;

uniform float deltaTime;
uniform float length;
uniform sampler2D noiseMap;
uniform float frequency;
uniform float amplitude;

out vec4 FragColor;

void main(){

    vec2 texCoord = Offset.xz / length;
    float noise = pow(texture(noiseMap, texCoord).r, 2.2);
    noise = (Offset.x + Offset.z) + (noise * frequency) * amplitude;
    float w = sin(noise + deltaTime);

    FragColor = vec4(w, w, w, 1.0);
}