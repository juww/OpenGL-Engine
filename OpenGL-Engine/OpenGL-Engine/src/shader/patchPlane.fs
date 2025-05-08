#version 430 core

out vec4 FragColor;

uniform sampler2D Textures;

in float Height;
in vec2 texCoord;

void main()
{
    vec3 color = texture(Textures, texCoord).rgb;
    FragColor = vec4(vec3(1.0, 0.0, 0.0), 1.0); // set all 4 vector values to 1.0
}