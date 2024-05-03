#version 430 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float _amplitude;
uniform float _frequency;
uniform float _time;
uniform float _speed;

out vec3 FragPos;

void main () {
    
    FragPos = vec3(model * vec4(aPos, 1.0));
    FragPos.y += sin(((aPos.x + aPos.z) * _frequency) + (_time * _speed)) * _amplitude;
    FragPos.y += sin(((aPos.x + aPos.z) * (_frequency * 1.5)) + (_time * _speed)) * (_amplitude * 0.7);
    FragPos.y += sin(((aPos.x) * (_frequency * 1.1)) + (_time * _speed * 0.5)) * (_amplitude * 0.5);
    FragPos.y += sin(((aPos.z) * (_frequency * 1.7)) + (_time * _speed * 0.6)) * (_amplitude);

    gl_Position = projection * view * vec4(FragPos, 1.0);
}