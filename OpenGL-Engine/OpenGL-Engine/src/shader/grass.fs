#version 430 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 Offset;
in float noiseColor;

out vec4 FragColor;

vec4 lerp(vec4 a, vec4 b, float t){
    return a + (t * (b - a));
}

void main(){
    
    vec4 a1 = vec4(0.6798911, 0.8980392, 0.654902, 1.0);
    vec4 a2 = vec4(0.3019608, 0.4509804, 0.36278453, 1.0);
    vec4 aoColor = vec4(0.094117634, 0.16034704, 0.2627451, 1.0);
    vec4 tipColor = vec4(1.0, 1.0, 1.0, 1.0);

    vec4 col = lerp(a2, a1, TexCoord.y);
    vec3 lightDir = normalize(-vec3(-0.2, -1.0, -0.3));
    float ndotl = max(dot(normalize(vec3(0.0, 1.0, 0.0)), lightDir), 0.0);

    vec4 ao = lerp(vec4(1.0), aoColor, TexCoord.y);
    vec4 tip = lerp(tipColor, vec4(0.0), TexCoord.y * TexCoord.y);
    vec4 grassColor = (col + tip) * ndotl * ao;

    FragColor = grassColor;
    
    // for debug to see the noise map
    // vec4 noise = vec4(noiseColor, noiseColor, noiseColor, 1.0);
    // FragColor = noise;
}