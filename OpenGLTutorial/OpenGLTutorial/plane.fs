#version 430 core

in vec2 TexCoords;

uniform sampler2D noiseMap;

out vec4 FragColor;

void main() {
    
    vec3 noise = texture(noiseMap, TexCoords).rgb;

    FragColor = vec4(noise, 1.0);
}
