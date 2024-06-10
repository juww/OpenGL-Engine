#version 430 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D depthTexture;

uniform float near;
uniform float far;
uniform float density;
uniform vec3 fogColor;

const float offset = 1.0 / 300.0;


vec3 inversion(){
    return vec3(1.0 - texture(screenTexture, TexCoords));
}

vec3 grayscale(vec3 col){
    float avg = 0.2126 * col.r + 0.7152 * col.g + 0.0722 * col.b;
    return vec3(avg,avg,avg);
}

vec3 Kernel(){
    vec2 offsets[9] = vec2[](
        vec2(-offset, offset), // top-left
        vec2( 0.0f, offset), // top-center
        vec2( offset, offset), // top-right
        vec2(-offset, 0.0f), // center-left
        vec2( 0.0f, 0.0f), // center-center
        vec2( offset, 0.0f), // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f, -offset), // bottom-center
        vec2( offset, -offset) // bottom-right
    );
    //sharpen
    float sharpenKernel[9] = float[](
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
    );
    // blur
    float blurKernel[9] = float[](
        1.0 / 16, 2.0 / 16, 1.0 / 16,
        2.0 / 16, 4.0 / 16, 2.0 / 16,
        1.0 / 16, 2.0 / 16, 1.0 / 16
    );
    float edgeDetectKernel[9] = float[](
        1,  1, 1,
        1, -8, 1,
        1,  1, 1
    );
    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++) {
        sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++){
        col += sampleTex[i] * edgeDetectKernel[i];
    }

    return col;
}

float calculateFogFactor(float depth, float st, float ed){
    
    float fog = 0.0;
    //linear
    //fog = (ed - depth) / (ed - st);
    //return fog;

    //exponential 
    float sq = depth * density;
    //fog = 1 / pow(2, sq);
    //return fog;

    // exponential squared;
    fog = 1 / pow(2, pow(sq, 2));
    fog = exp(-(sq * sq));
    fog = clamp(fog, 0.0, 1.0);

    return fog;
}

vec3 lerp(vec3 a, vec3 b, float t){
    return a + (t * (b - a));
}

void main()
{
    vec3 col = texture(screenTexture, TexCoords).rgb;
    vec3 baseColor = col;

    float depth = texture(depthTexture, TexCoords).r;
    depth = (1 - (far/near)) * depth + (far/near);
    depth = 1.0 / depth;
    depth = depth * far;

    float fog = calculateFogFactor(depth, near, far);
    vec3 outcolor = lerp(fogColor, baseColor, fog);

    FragColor = vec4(depth ,depth, depth, 1.0);
    FragColor = vec4(outcolor, 1.0);


    //inversion;
    //FragColor = vec4(inversion(), 1.0);
    //grayscale;
    //FragColor = vec4(grayscale(col), 1.0);
    //sharpenKernel
    //FragColor = vec4(Kernel(),1.0);
} 