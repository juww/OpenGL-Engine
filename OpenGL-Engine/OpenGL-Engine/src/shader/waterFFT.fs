#version 430 core

in vec3 FragPos;
in vec2 texCoord;

uniform vec3 lightDirection;
uniform vec3 viewPos;

uniform mat4 model;

uniform sampler2D displacementTexture;
uniform sampler2D slopeTexture;

out vec4 FragColor;

vec3 phong(vec3 color, vec3 Normal){

    vec3 lightPos = vec3(-1000.0,1000.0,-1000.0);
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * vec3(1.0);

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * color;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32);
    vec3 specular = specularStrength * spec * vec3(1.0);

    return (diffuse + ambient + specular);
}

void main(){

    vec3 color = vec3(6.0 / 255.0, 66.0  / 255.0, 115.0  / 255.0);
    color = normalize(color);

    vec3 displacement = texture(displacementTexture, texCoord).rgb;
    vec2 slope = texture(slopeTexture, texCoord).rg;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 norm = normalize(vec3(-slope.x, 1.0f, -slope.y));
    norm = normalize(normalMatrix * norm);
    vec3 result = phong(color, norm);

    FragColor = vec4(result, 1.0);
}