#version 430 core

layout (location = 0) in float heightValue;
layout (location = 1) in vec3  aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float lenght;

out vec3 Normal;
out vec3 TexCoords;
out vec3 FragPos;

vec3 planeVertex(){

    vec3 pos;

    float n = lenght * 2.0;
    float offsetVertices = n + 2.0;
    float rowIndex = mod(float(gl_VertexID), offsetVertices);
    float clampedIndex = clamp(rowIndex - 1.0, 0.0, n - 1.0);

    pos.x = floor(clampedIndex / 2.0) - (lenght / 2.0);
    pos.z = mod(clampedIndex, 2.0) - (lenght / 2.0);
    pos.y = heightValue;
    // pos.y = -5.0;

    pos.z += (floor(float(gl_VertexID) / offsetVertices));

    return pos;
}

void main (){

    vec3 pos = planeVertex();

    FragPos = vec3(model * vec4(pos, 1.0));
    TexCoords = (pos.xyz + (lenght / 2.0)) / lenght;
    Normal = aNormal;

    gl_Position = projection * view * model * vec4(pos, 1.0);
}