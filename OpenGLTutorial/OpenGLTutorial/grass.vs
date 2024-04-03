#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aOffset;
layout (location = 2) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float halfOffset;

out vec3 FragPos;
out vec3 Normal;

void main(){

	vec3 pos = aPos + aOffset;
	pos.x -= halfOffset;
	pos.z -= halfOffset;
	
	FragPos = vec3(model * vec4(pos, 1.0));
	Normal = aNormal;

	gl_Position = projection * view * model * vec4(pos, 1.0);
}