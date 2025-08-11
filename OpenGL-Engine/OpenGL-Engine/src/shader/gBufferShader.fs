#version 430 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gNormalMap;
layout (location = 4) out vec4 gORMMap;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D MROMap;

void main() {
    // store the fragment position vector in the first gbuffer texture
    gPosition = vec4(FragPos, 1.0f);
    // also store the per-fragment normals into the gbuffer
    gNormal = vec4(normalize(Normal), 1.0f);
    // and the diffuse per-fragment color
    gAlbedo = texture(albedoMap, TexCoords);

    gNormalMap = texture(normalMap, TexCoords);

    gORMMap = texture(MROMap, TexCoords);
}