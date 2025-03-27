#version 430 core

const int MAX_BONES = 128;

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
uniform int hasBone;

uniform mat4 boneTransform[MAX_BONES];

out geom_data {
    mat4 model;
    vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} data_out;

vec4 applyBoneTransform(vec4 p) {

    mat4 result = mat4(1.0);

    for (int i = 0; i < 4; ++i) {
         if(aJoint[i] < 0) continue;
         result += (boneTransform[aJoint[i]] * aWeight[i]);
    }
    vec4 res = result * p;
    return res;
}

void main() {

    vec4 position = vec4(aPos, 1.0);
    vec4 normal = vec4(aNormal, 0.0);
    if(hasBone == 1){
        position = applyBoneTransform(vec4(aPos, 1.0));
        normal = normalize(applyBoneTransform(vec4(aNormal, 0.0)));
    } 

    gl_Position = projection * view * model * position;

    data_out.model = model;
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 Normal = normalize(normalMatrix * vec3(normal));
    data_out.FragPos = vec3(model * position);
    data_out.Normal = vec3(Normal);

    data_out.TexCoords = aTexCoords;
}