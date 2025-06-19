#version 430 core

out vec4 FragColor;

uniform sampler2D heightMap;

uniform mat4 model;
uniform vec3 lightDirection;
uniform vec3 viewPos;

in float Height;
in vec2 texCoord;
in vec3 FragPos;

vec3 phong(vec3 color, vec3 Normal);

void main() {
    vec3 color = texture(heightMap, texCoord).rgb;
    
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 Tangent = vec3(1.0, color.g, 0.0);
    vec3 BiNormal = vec3(0.0, color.b, 1.0);
    vec3 norm = normalize(-cross(Tangent, BiNormal));
    norm = normalize(normalMatrix * norm);

    vec3 result = phong(vec3(1.0f, 0.0f, 0.0f), norm);
    FragColor = vec4(result, 1.0); // set all 4 vector values to 1.0
}

vec3 phong(vec3 color, vec3 Normal){

    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * vec3(1.0);

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(vec3(0.0f, 1.0f, 0.0f));
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * color;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32);
    vec3 specular = specularStrength * spec * vec3(1.0);

    return (diffuse + ambient + specular);
}