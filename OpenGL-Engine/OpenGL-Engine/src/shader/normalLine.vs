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

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
} vs_out;

vec4 calculateWave(vec3 v) {
    float h = 0.0f;
    float sumdx = 0.0f;
    float sumdz = 0.0f;
    float a = 1.0f;
    float sumAmp = 0.0f;
    float f = 1.0f;
    float s = _speed;
    float seed = 0.0f;
    vec2 sumDeriv = vec2(0.0f);
    for(int i = 0; i < _waveCount; i++) {
        vec2 d = vec2(cos(seed),sin(seed));
        d= normalize(d);
        float xz = dot(d,v.xz);

        float dirx = dot(vec2(d.x, 0), v.xz);
        float dirz = dot(vec2(0, d.y), v.xz);

        float w = a * sin((xz * f) + (_time * s));
        float eulerWave = exp(w - 1);

        float dx = dirx * f * a * cos((xz * f) + (_time * s)) * eulerWave;
        float dz = dirz * f * a * cos((xz * f) + (_time * s)) * eulerWave;

        vec2 derivative = d * f * a * cos((xz * f) + (_time * s)) * eulerWave;


        s *= 1.07;

        h += eulerWave;
        sumDeriv += derivative;
        sumdx += dx;
        sumdz += dz;

        sumAmp += a;
        a *= _amplitude;
        f *= _frequency;
        seed += 4.3;
    }
    
    vec3 Tangent = vec3(1,sumDeriv.x,0);
    vec3 BiNormal = vec3(0,sumDeriv.y,1);
    vec3 Norm = cross(Tangent, BiNormal);
    
    Norm = normalize(-Norm);
    vec4 ret = vec4(Norm.xyz, h / sumAmp);
    return ret;
}

void main () {
    
    vec3 pos = aPos;
    vec4 res = calculateWave(pos);

    pos.y += res.w;
    vec3 Normal = vec3(res.xyz);
    mat4 normalMatrix = transpose(inverse(view * model));
    vec4 norm = normalMatrix * vec4(Normal, 1.0);
    vs_out.Normal = vec3(normalize(norm));
    vs_out.FragPos = vec3(model * vec4(pos, 1.0));
    gl_Position = view * model * vec4(pos, 1.0);
}