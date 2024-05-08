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

vec4 calculateWave(vec3 v) {
    float h = 0.0f;
    float a = 1.0f;
    float sumAmp = 0.0f;
    float f = 1.0f;
    float s = _speed;
    float seed = 0.0f;
    vec2 sumDeriv = vec2(0.0f);
    vec2 p = vec2(v.xz);
    for(int i = 0; i < _waveCount; i++) {
        vec2 d = vec2(cos(seed),sin(seed));
        d = normalize(d);
        float xz = dot(d, p);

        float w = a * sin((xz * f) + (_time * s));
        float eulerWave = exp(w - 1);

        vec2 derivative = d * f * a * cos((xz * f) + (_time * s)) * eulerWave;

        p+= -derivative.x * d * a * 0.5;


        h += eulerWave;
        sumDeriv += derivative;

        sumAmp += a;
        a *= _amplitude;
        f *= _frequency;
        s *= 1.07;
        seed += 4.3;
    }
    vec3 Tangent = vec3(1,sumDeriv.x/ sumAmp,0);
    vec3 BiNormal = vec3(0,sumDeriv.y/ sumAmp,1);
    vec3 Norm = cross(Tangent, BiNormal);
    
    Norm = normalize(-Norm);
    vec4 ret = vec4(Norm.xyz, h / sumAmp);
    return ret;
}

void main () {
    
    vec3 pos = aPos;
    vec4 res = calculateWave(pos);

    pos.y += res.w;

    Normal = vec3(res.xyz);
    mat4 normalMatrix = transpose(inverse(view * model));
    vec4 norm = normalMatrix * vec4(Normal, 1.0);

    Normal = vec3(normalize(norm));
    FragPos = vec3(model * vec4(pos, 1.0));
    gl_Position = projection * view * model * vec4(pos, 1.0);
}