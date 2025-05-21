#version 430 core
out vec4 FragColor;

uniform sampler2DArray textures;
uniform float texIndex;
uniform float visibleStrength;

in vec2 TexCoords;

void main() {

    vec3 color = texture(textures, vec3(TexCoords, texIndex)).rgb * visibleStrength;
    FragColor = vec4(color, 1.0);
} 