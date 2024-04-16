#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

layout (location = 6) in vec3 aOffset;
layout (location = 7) in float aRadian;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float halfOffset;

uniform float deltaTime;
uniform float length;
uniform sampler2D noiseMap;
uniform float frequency;
uniform float amplitude;
uniform float _scale;
uniform float _droop;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec3 Offset;
out float noiseColor;

uint rng_state;

//Hash invented by Thomas Wang
void wang_hash(uint seed) {
    rng_state = (seed ^ 61) ^ (seed >> 16);
    rng_state *= 9;
    rng_state = rng_state ^ (rng_state >> 4);
    rng_state *= 0x27d4eb2d;
    rng_state = rng_state ^ (rng_state >> 15);
}

//Xorshift algorithm from George Marsaglia's paper
uint rand_xorshift() {
    rng_state ^= (rng_state << 13);
    rng_state ^= (rng_state >> 17);
    rng_state ^= (rng_state << 5);

    return rng_state;
}

float rand() {
    return float(rand_xorshift()) * (1.0 / 4294967296.0);
}

void initRand(uint seed) {
    wang_hash(seed);
}

float randValue(uint seed) {
    initRand(seed);
    return rand();
}

float rand2(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

float lerp(float a, float b, float t){
	return a + t * (b - a);
}

vec4 rotY(vec4 pos, float rad){
	mat4 rot = mat4(1.0);
	float sina = sin(rad);
	float cosa = cos(rad);

	rot[0][0] = cosa; rot[0][2] = -sina;
	rot[2][0] = sina; rot[2][2] = cosa;

	pos = rot * pos;
	return pos;
}

void main(){

	vec4 pos = vec4(aPos, 1.0);
	
	float noise = texture(noiseMap, vec2(aOffset.xz / length)).r;
	noise = (aOffset.x * 0.05 + aOffset.z * 0.1) + 2.3 * noise;
    float w = -(sin((noise + deltaTime) * frequency) + 1.0f) * amplitude;
	noiseColor = -w;
	
	// note:
	// do i need this random hashid (?)
	uint seed = uint(abs(Offset.x * 10000 + Offset.y * 100 + Offset.z * 0.05 + 2));
	float idHash = randValue(seed);
	idHash = randValue(uint(idHash * 100000));

    vec4 animationDirection = vec4(1.0f, 0.0f, 1.0f, 0.0f);
	animationDirection = -normalize(rotY(animationDirection, aRadian));

	vec4 localPosition = pos;
    localPosition = rotY(localPosition, aRadian);
    localPosition.y += _scale * pos.y * pos.y * pos.y;
	vec4 dir = _droop * lerp(0.5f, 1.0f, idHash) * (pos.y * pos.y * _scale) * animationDirection;
    localPosition += dir;

	float swayVariance = lerp(0.8, 1.0, idHash);
    float movement = pos.y * pos.y * w;
    movement *= swayVariance;
                
    localPosition.xz += movement;
	
	pos = localPosition + vec4(aOffset, 0.0);
	pos.x -= halfOffset;
	pos.z -= halfOffset;

	FragPos = vec3(model * pos);
	TexCoord = aTexCoord;
	Normal = aNormal;
	Offset = aOffset;

	gl_Position = projection * view * model * pos;
}