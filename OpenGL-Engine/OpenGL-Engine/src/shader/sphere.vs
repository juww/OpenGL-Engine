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
out mat4 invNormalM;

void main (){
    
    TexCoords = aTexCoords;

    FaceCube = aFaces;
    mat4 normalMatrix = transpose(inverse(model));
    Normal = vec3(normalMatrix * vec4(aNormal, 1.0));
    Normal = normalize(Normal);

    invNormalM = normalMatrix;
    FragPos = vec3(model * vec4(aPos, 1.0));

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}