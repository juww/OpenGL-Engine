#version 430 core

layout (location = 0) in uint  vertexID;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float lenght;

//out vec3 Normal;
out vec2 TexCoords;

vec3 planeVertex(){

    vec3 pos;

    float n = lenght * 2.0;
    float offsetVertices = n + 2.0;
    float rowIndex = mod(float(vertexID), offsetVertices);
    float clampedIndex = clamp(rowIndex - 1.0, 0.0, n - 1.0);

    pos.x = floor(clampedIndex / 2.0);
    pos.z = mod(clampedIndex, 2.0);

    pos.z += (floor(float(vertexID) / offsetVertices));

    return pos;
}

void main (){

    vec3 pos = planeVertex();

    vec4 mvp = model * vec4(pos, 1.0);
    TexCoords = mvp.xz / lenght;
    gl_Position = projection * view * model * vec4(pos, 1.0);
    
    //Normal = aNormal;
    //TexCoords = aTexCoords;
	
}