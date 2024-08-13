#version 430 core

in vec3 Normal;
in vec2 TexCoords;
in vec3 FragPos;

uniform vec3 lightDirection;
uniform vec3 viewPos;

uniform sampler2D Textures;

out vec4 FragColor;

void main(){
    
    vec3 color = vec3(1.0, 0.0, 0.0);
    color = texture(Textures, vec2(TexCoords)).rgb;
    //color = normalize(color);
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * vec3(1.0);

    vec3 norm = normalize(Normal);
    //vec3 lightDir = normalize(lightPos - FragPos);
    vec3 lightDir = normalize(-lightDirection);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * color;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32);
    vec3 specular = specularStrength * spec * vec3(1.0);

    FragColor = vec4(diffuse + ambient + specular, 1.0);
    //FragColor = vec4(color, 1.0);
}