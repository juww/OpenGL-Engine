#version 430 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gNormalMap;
layout (location = 4) out vec4 gORMMap;
layout (location = 5) out vec4 gViewPosition;
layout (location = 6) out vec4 gViewNormal;
layout (location = 7) out vec4 gTexCoords;

in vec3 FragPos;
in vec3 ViewPos;
in vec2 TexCoords;
in vec3 Normal;
in vec3 ViewNormal;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D MROMap;

vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(FragPos);
    vec3 Q2  = dFdy(FragPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 tbn = mat3(T, B, N);

    return normalize(tbn * tangentNormal);
}

void main() {

    // store the fragment position vector in the first gbuffer texture
    gPosition = vec4(FragPos, 1.0f);
    gViewPosition = vec4(ViewPos, 1.0f);

    // also store the per-fragment normals into the gbuffer
    gNormal = vec4(normalize(Normal), 1.0f);
    gViewNormal = vec4(normalize(ViewNormal), 1.0f);

    // and the diffuse per-fragment color
    gAlbedo = texture(albedoMap, TexCoords);

    gNormalMap = vec4(getNormalFromMap(), 1.0f);
    gTexCoords = vec4(TexCoords, 0.0f, 1.0f);
    
    gORMMap = texture(MROMap, TexCoords);
}