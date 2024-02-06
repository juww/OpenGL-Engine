#version 430 core

layout (location = 0) in vec3  aPos;
layout (location = 1) in vec3  aNormal;
layout (location = 2) in vec2  aTexCoords;
layout (location = 3) in vec4  aTangent;
layout (location = 4) in vec4  aColor;
layout (location = 5) in ivec4 aJoint;
layout (location = 6) in vec4  aWeight;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

//out vec3 FragPos;
//out vec3 Normal;
//out vec3 Color;
//out vec2 TexCoords;

void main (){

    gl_Position = projection * view * model * vec4(aPos, 1.0);

    //FragPos = vec3(model * vec4(aPos, 1.0));
    //Normal = aNormal;
    //TexCoords = aTexCoords;
	
}