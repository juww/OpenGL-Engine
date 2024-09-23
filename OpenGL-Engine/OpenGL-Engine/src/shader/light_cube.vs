#version 430 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 texcoord;
out float asu;

void main()
{
    float halfN = 257.0 * 0.5;
    float x = (aPos.x + halfN) / 257.0;
    float z = (aPos.z + halfN) / 257.0;
    texcoord = vec2(x, z);
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}