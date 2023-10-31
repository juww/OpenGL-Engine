#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out geom_data {
    mat4 model;
    vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} data_out;

void main() {

    gl_Position = projection * view * model * vec4(aPos, 1.0);

    data_out.model = model;
    data_out.FragPos = vec3(model * vec4(aPos, 1.0));
    data_out.Normal = aNormal;
    data_out.TexCoords = aTexCoords;
}