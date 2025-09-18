#version 430 core

//out vec4 FragColor;
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

uniform sampler2D Textures;
uniform vec3 lightColor;

in vec2 texcoord;

void main() {
    //vec3 color = texture(Textures, texcoord).rgb;

    BrightColor = vec4(lightColor, 1.0);
    FragColor = vec4(lightColor, 1.0);
}