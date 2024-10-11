#version 430 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

void main(){

    vec3 color = vec3(6.0 / 255.0, 66.0  / 255.0, 115.0  / 255.0);
    color = normalize(color);

    FragColor = vec4(color, 1.0);
}