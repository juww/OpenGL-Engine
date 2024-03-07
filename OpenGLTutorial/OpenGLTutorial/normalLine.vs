#version 430 core
layout (location = 2) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 3) in vec2 aTextCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out geom_data {
	vec3 Normal;
	vec2 texCoord;
	mat4 projection;
} data_out;

void main() {

    gl_Position = projection * view * model * vec4(aPos, 1.0);

    data_out.Normal = aNormal;
    data_out.texCoord = (aPos.xz + (9.0 / 2.0)) / 9.0;
    data_out.projection = projection;
}