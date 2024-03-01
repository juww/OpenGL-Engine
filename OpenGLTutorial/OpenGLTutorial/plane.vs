#version 430 core

layout (location = 0) in float heightValue;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float lenght;
uniform float heightMultiplier;

//out vec3 Normal;
out vec2 TexCoords;
out vec4 FragPos;

vec3 planeVertex(){

    vec3 pos;

    float n = lenght * 2.0;
    float offsetVertices = n + 2.0;
    float rowIndex = mod(float(gl_VertexID), offsetVertices);
    float clampedIndex = clamp(rowIndex - 1.0, 0.0, n - 1.0);

    pos.x = floor(clampedIndex / 2.0) - (lenght / 2.0);
    pos.z = mod(clampedIndex, 2.0) - (lenght / 2.0);
    pos.y = (heightValue * heightMultiplier) - (heightMultiplier);
    // pos.y = -5.0;

    pos.z += (floor(float(gl_VertexID) / offsetVertices));

    return pos;
}

void main (){

    vec3 pos = planeVertex();

    FragPos = vec4(pos, 1.0) * model;
    TexCoords = (pos.xz + (lenght / 2.0)) / lenght;
    gl_Position = projection * view * model * vec4(pos, 1.0);
    
    //Normal = aNormal;
    //TexCoords = aTexCoords;
}