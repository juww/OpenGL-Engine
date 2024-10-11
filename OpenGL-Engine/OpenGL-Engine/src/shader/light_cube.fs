#version 430 core

out vec4 FragColor;

uniform sampler2D Textures;

in vec2 texcoord;

void main()
{
    vec3 color = texture(Textures, texcoord).rgb;
    FragColor = vec4(vec3(color), 1.0); // set all 4 vector values to 1.0
}