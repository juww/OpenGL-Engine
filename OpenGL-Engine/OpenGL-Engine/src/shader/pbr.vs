#version 430 core

layout (location = 0) in vec3  aPos;
layout (location = 1) in vec3  aNormal;
layout (location = 2) in vec2  aTexCoords;
layout (location = 3) in vec3  aTangent;
layout (location = 4) in vec3  aBitangent;
layout (location = 5) in ivec4 aJoint;
layout (location = 6) in vec4  aWeight;

const int MAX_BONES = 128;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

uniform int hasBone;
uniform mat4 boneTransform[MAX_BONES];

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
out vec4 FragPosLightSpace;

vec4 applyBoneTransform(vec4 p) {

    vec4 ret = vec4(0.0f);

    for (int i = 0; i < 4; ++i) {
         vec4 temp = (boneTransform[aJoint[i]] * p);
         ret += (temp * aWeight[i]);
    }

    return ret;
}

void main() {

    vec4 position = vec4(aPos, 1.0);
    vec4 norm = vec4(aNormal, 0.0);

    mat3 normalMatrix = transpose(inverse(mat3(model)));

    vec3 T = normalize(vec3(normalMatrix * aTangent));
    vec3 B = normalize(vec3(normalMatrix * aBitangent));
    vec3 N = normalize(vec3(normalMatrix * aNormal));
    TBN = mat3(T, B, N);

    if(hasBone == 1){
        position = applyBoneTransform(vec4(aPos, 1.0));
        norm = normalize(applyBoneTransform(vec4(aNormal, 0.0)));
    }

    FragPos = vec3(model * position);
    Normal = normalize(normalMatrix * vec3(norm));
    TexCoords = aTexCoords;
    Tangent = T;
    Bitangent = B;

    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);

    //data_out.model = model;
    //data_out.FragPos = vec3(model * vec4(aPos, 1.0));
    //data_out.Normal = aNormal;
    //data_out.TexCoords = aTexCoords;

    gl_Position = projection * view * model * position;
}