#version 430 core

layout (location = 0) in vec3 aPos;

out vec2 TexCoord;

void main() {

    TexCoord = vec2(aPos.x, aPos.z);
    gl_Position = vec4(aPos, 1.0);
}