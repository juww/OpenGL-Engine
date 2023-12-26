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

uniform mat4 boneTransform[MAX_BONES];

out geom_data {
    mat4 model;
    vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} data_out;

vec4 applyBoneTransform(vec4 p) {

    vec4 result = vec4(0.0);

    for (int i = 0; i < 4; ++i) {
         if(aJoint[i] == -1) continue;
         vec4 localTransform = boneTransform[aJoint[i]] * p;
         result += (aWeight[i] * localTransform);
    }
    return result;
}

void main() {

    vec4 position = applyBoneTransform(vec4(aPos, 1.0));
    vec4 normal = normalize(applyBoneTransform(vec4(aNormal, 0.0)));

    //gl_Position = projection * view * model * vec4(aPos, 1.0);
    gl_Position = projection * view * model * position;

    data_out.model = model;

    //data_out.FragPos = vec3(model * vec4(aPos, 1.0));
    //data_out.Normal = aNormal;

    data_out.FragPos = vec3(model * position);
    data_out.Normal = vec3(normal);

    data_out.TexCoords = aTexCoords;
}