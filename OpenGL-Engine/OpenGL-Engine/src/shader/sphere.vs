#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in float aFaces;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float _time;

out vec3 Normal;
out vec2 TexCoords;
out vec3 FragPos;
out float FaceCube;

void main (){
    
    TexCoords = aTexCoords;

    Normal = aNormal;
    FaceCube = aFaces;
    mat4 normalMatrix = transpose(inverse(view * model));
    Normal = vec3(normalMatrix * vec4(Normal, 1.0));
    Normal = normalize(Normal);

    FragPos = vec3(model * vec4(aPos, 1.0));

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}