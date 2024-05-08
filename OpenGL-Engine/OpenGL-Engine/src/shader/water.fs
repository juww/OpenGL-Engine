#version 430 core

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightDirection;
uniform vec3 viewPos;

out vec4 FragColor;

void main(){

    vec3 lightPos = vec3(1000.0,1000.0,1000.0);

    vec3 color = vec3(0.0, 89.0, 179.0);
    color = normalize(color);
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * vec3(1.0);

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    //vec3 lightDir = normalize(-lightDirection);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * color;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 64);
    vec3 specular = specularStrength * spec * vec3(1.0);

    FragColor = vec4(diffuse + ambient + specular, 1.0);
}