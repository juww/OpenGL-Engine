#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aOffset;
layout (location = 2) in float aRadian;
layout (location = 3) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float halfOffset;

out vec3 FragPos;
out vec3 Normal;
out vec3 Offset;

mat4 rotY(){
	mat4 ret = mat4(1.0);
	ret[0][0] = cos(aRadian);
	ret[0][2] = sin(aRadian);
	ret[2][0] = -sin(aRadian);
	ret[2][2] = cos(aRadian);

	return ret;
}

void main(){

	vec4 pos = vec4(aPos, 1.0);

	mat4 rot = rotY();
	pos = rot * pos;
	
	pos = pos + vec4(aOffset, 0.0);
	pos.x -= halfOffset;
	pos.z -= halfOffset;
	

	FragPos = vec3(model * pos);
	Normal = aNormal;
	Offset = aOffset;

	gl_Position = projection * view * model * pos;
}