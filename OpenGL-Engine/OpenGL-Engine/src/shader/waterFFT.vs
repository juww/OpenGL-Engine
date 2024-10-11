#version 430 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float _amplitude;
uniform float _frequency;
uniform float _time;
uniform float _speed;
uniform float _seed;
uniform float _iter;
uniform int _waveCount;

out vec3 FragPos;
out vec3 Normal;

void main () {
    
    vec3 pos = aPos;
    vec4 res = calculateWave(pos);

    pos.y += res.w;

    Normal = vec3(res.xyz);
    mat4 normalMatrix = transpose(inverse(model));

    Normal = vec3(normalMatrix * vec4(-Normal, 1.0));
    FragPos = vec3(model * vec4(pos, 1.0));
    gl_Position = projection * view * model * vec4(pos, 1.0);
}