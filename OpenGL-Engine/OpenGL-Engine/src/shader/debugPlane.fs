#version 430 core
out vec4 FragColor;

uniform sampler2D textures;

in vec2 TexCoords;

void main() {

    vec3 color = texture(textures, TexCoords).rgb * 12345.0f;
    FragColor = vec4(color, 1.0);
} 