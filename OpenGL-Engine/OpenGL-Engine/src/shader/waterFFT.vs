#version 430 core

layout (location = 0) in vec3 aPos;

uniform float width;
uniform float heigth;

out vec2 TexCoord;

void main() {

    TexCoord = vec2(aPos.x/width, aPos.z/heigth);
    gl_Position = vec4(aPos, 1.0);
}