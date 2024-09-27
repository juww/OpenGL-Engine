#version 430 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float nsize;

out vec2 texcoord;
out float asu;

void main()
{
    float halfN = nsize * 0.5;
    float x = (aPos.x + halfN) / nsize;
    float z = (aPos.z + halfN) / nsize;
    texcoord = vec2(x, z);
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}