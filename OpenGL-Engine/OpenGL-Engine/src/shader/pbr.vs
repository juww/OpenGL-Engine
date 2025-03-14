#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightPos;
uniform vec3 viewPos;

//out geom_data {
//    mat4 model;
//    vec3 FragPos;
//	vec3 Normal;
//	vec2 TexCoords;
//} data_out;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out mat3 TBN;
out vec3 Tangent;
out vec3 Bitangent;

void main() {

    vec4 position = vec4(aPos, 1.0);

    mat3 normalMatrix = transpose(inverse(mat3(model)));

    vec3 T = normalize(vec3(normalMatrix * aTangent));
    vec3 B = normalize(vec3(normalMatrix * aBitangent));
    vec3 N = normalize(vec3(normalMatrix * aNormal));
    TBN = mat3(T, B, N);


    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = normalize(normalMatrix * aNormal);
    TexCoords = aTexCoords;
    Tangent = T;
    Bitangent = B;

    //data_out.model = model;
    //data_out.FragPos = vec3(model * vec4(aPos, 1.0));
    //data_out.Normal = aNormal;
    //data_out.TexCoords = aTexCoords;

    gl_Position = projection * view * model * position;
}