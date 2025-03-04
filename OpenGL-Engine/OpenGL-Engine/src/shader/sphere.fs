#version 430 core

in vec3 Normal;
in vec2 TexCoords;
in vec3 FragPos;
in float FaceCube;
in mat4 invNormalM;

uniform vec3 lightDirection;
uniform vec3 viewPos;

uniform sampler2D Textures;
uniform sampler2DArray CubeTextures;
uniform samplerCube skybox;

out vec4 FragColor;

vec3 CalculateReflection(){
    
    vec3 incident = normalize(FragPos - viewPos);
    vec3 reflection = reflect(incident, Normal);
    reflection = vec3(invNormalM * vec4(normalize(reflection), 0.0));
    vec3 res = texture(skybox, reflection).rgb;

    return res;
}

vec3 CalculateRefraction(){

    float ratio = 1.00 / 1.52;
    vec3 incident = normalize(viewPos - FragPos);
    vec3 Refraction = refract(incident, Normal, ratio);
    Refraction = normalize(Refraction);
    vec3 res = texture(skybox, Refraction).rgb;

    return res;
}

void main(){
    
    vec3 color = vec3(1.0, 0.0, 0.0);
    color = texture(Textures, vec2(TexCoords)).rgb;
    //color = texture(CubeTextures, vec3(TexCoords, FaceCube)).rgb;
    //color = CalculateRefraction();

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
    FragColor = vec4(color, 1.0);

}