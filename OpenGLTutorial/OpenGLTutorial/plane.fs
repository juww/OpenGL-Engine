#version 430 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

const int N_COLOUR = 8;
const float EPS = 1e-4;

uniform sampler2D noiseMap;
uniform vec3 lightPos;
uniform vec3 lightDirection;
uniform vec3 viewPos;
uniform vec3 lightColor;

uniform int colorCount;
uniform vec3 baseColor[N_COLOUR];
uniform float baseStartHeight[N_COLOUR];
uniform float baseBlend[N_COLOUR];

uniform float minHeight;
uniform float maxHeight;

out vec4 FragColor;

float inverseLerp(float a, float b, float value){
    return clamp((value - a) / (b - a), 0.0, 1.0);
}

vec3 blinnPhong(){

    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse 
    vec3 norm = normalize(Normal);
    // vec3 lightDir = normalize(lightPos - FragPos);
    vec3 lightDir = normalize(-lightDirection);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;  
        
    vec3 result = (ambient + diffuse + specular);

    return result;
}

vec3 heightColor(float heightPercent){
    vec3 res = vec3(heightPercent, heightPercent, heightPercent);
    for(int i = 0; i < colorCount; i++){   
        //float drawStrength = clamp(sign(heightPercent - baseStartHeight[i]), 0.0, 1.0);
        float drawStrength = inverseLerp(-baseBlend[i]/2 - EPS, baseBlend[i]/2, heightPercent - baseStartHeight[i]);
        res = res * (1 - drawStrength) + baseColor[i] * drawStrength;
    }
    return res;
}

void main() {
    
    vec3 noise = texture(noiseMap, TexCoords).rgb;

    float heightPercent = inverseLerp(minHeight, maxHeight, FragPos.y);

    vec3 lighting = blinnPhong();
    vec3 terrainColor = heightColor(heightPercent);

     vec3 result = terrainColor * lighting;

    FragColor = vec4(result, 1.0);
}
