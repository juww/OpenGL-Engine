#include "waterFFT.h"

#define PI 3.14159265358979323846

WaterFFT::WaterFFT(int n) {
    N = n;
    m_Seed = 1234;
    m_LowCutoff = 0.0001f;
    m_HighCutoff = 9000.0f;
    m_Gravity = 9.81f;
    m_Depth = 20.0f;
}

WaterFFT::~WaterFFT() {

}


float WaterFFT::hash(unsigned int n) {
    // integer hash copied from Hugo Elias
    n = (n << 13U) ^ n;
    n = n * (n * n * 15731U + 0x789221U) + 0x1376312589U;
    return float(n & unsigned int(0x7fffffffU)) / float(0x7fffffff);
}


glm::vec2 WaterFFT::ComplexMult(glm::vec2 a, glm::vec2 b) {
    return glm::vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

glm::vec2 WaterFFT::EulerFormula(float x) {
    return glm::vec2(cos(x), sin(x));
}

glm::vec2 WaterFFT::UniformToGaussian(float u1, float u2) {
    float R = sqrt(-2.0f * log(u1));
    float theta = 2.0f * PI * u2;

    return glm::vec2(R * cos(theta), R * sin(theta));
}

float WaterFFT::Dispersion(float kMag) {
    return sqrt(m_Gravity * kMag * tanh(std::min(kMag * m_Depth, 20.0f)));
}

float WaterFFT::DispersionDerivative(float kMag) {
    float th = tanh(std::min(kMag * m_Depth, 20.0f));
    float ch = cosh(kMag * m_Depth);
    return m_Gravity * (m_Depth * kMag / ch / ch + th) / Dispersion(kMag) / 2.0f;
}

float WaterFFT::JonswapAlpha(float fetch, float windSpeed) {
    return 0.076f * glm::pow(m_Gravity * fetch / windSpeed / windSpeed, -0.22f);
}

float WaterFFT::JonswapPeakFrequency(float fetch, float windSpeed) {
    return 22 * glm::pow(windSpeed * fetch / m_Gravity / m_Gravity, -0.33f);
}

float WaterFFT::NormalizationFactor(float s) {
    float s2 = s * s;
    float s3 = s2 * s;
    float s4 = s3 * s;
    if (s < 5) return -0.000564f * s4 + 0.00776f * s3 - 0.044f * s2 + 0.192f * s + 0.163f;
    else return -4.80e-08f * s4 + 1.07e-05f * s3 - 9.53e-04f * s2 + 5.90e-02f * s + 3.93e-01f;
}

float WaterFFT::Cosine2s(float theta, float s) {
    return NormalizationFactor(s) * pow(abs(cos(0.5f * theta)), 2.0f * s);
}

float WaterFFT::SpreadPower(float omega, float peakOmega) {
    if (omega > peakOmega)
        return 9.77f * pow(abs(omega / peakOmega), -2.5f);
    else
        return 6.97f * pow(abs(omega / peakOmega), 5.0f);
}

float WaterFFT::DirectionSpectrum(float theta, float omega, spectrumSettings spectrum) {
    float s = SpreadPower(omega, spectrum.peakOmega) + 16 * tanh(std::min(omega / spectrum.peakOmega, 20.0f)) * spectrum.swell * spectrum.swell;
    return interpolate::lerp(2.0f / 3.1415f * cos(theta) * cos(theta), Cosine2s(theta - spectrum.angle, s), spectrum.spreadBlend);
}

float WaterFFT::ShortWavesFade(float kLength, spectrumSettings spectrum) {
    return exp(-spectrum.shortWavesFade * spectrum.shortWavesFade * kLength * kLength);
}

float WaterFFT::TMACorrection(float omega) {
    float omegaH = omega * sqrt(m_Depth / m_Gravity);
    if (omegaH <= 1.0f)
        return 0.5f * omegaH * omegaH;
    if (omegaH < 2.0f)
        return 1.0f - 0.5f * (2.0f - omegaH) * (2.0f - omegaH);

    return 1.0f;
}

float WaterFFT::JONSWAP(float omega, spectrumSettings spectrum) {
    float sigma = (omega <= spectrum.peakOmega) ? 0.07f : 0.09f;

    float r = exp(-(omega - spectrum.peakOmega) * (omega - spectrum.peakOmega) / 2.0f / sigma / sigma / spectrum.peakOmega / spectrum.peakOmega);

    float oneOverOmega = 1.0f / omega;
    float peakOmegaOverOmega = spectrum.peakOmega / omega;
    return spectrum.scale * TMACorrection(omega) * spectrum.alpha * m_Gravity * m_Gravity
        * oneOverOmega * oneOverOmega * oneOverOmega * oneOverOmega * oneOverOmega
        * exp(-1.25f * peakOmegaOverOmega * peakOmegaOverOmega * peakOmegaOverOmega * peakOmegaOverOmega)
        * pow(abs(spectrum.gamma), r);
}

void WaterFFT::initialSpectrum() {

    spectrumParameter param;
    param.scale = 0.1f;
    param.windDirection = PI * 0.25f;
    param.spreadBlend = 0.66f;
    param.swell = 1.0f;
    param.fetch = 100000.0f;
    param.windSpeed = 200.0f;
    param.peakEnhancement = 100.0f;
    param.shortWavesFade = 0.025f;


    spectrumSettings settings;
    for (int k = 0; k < 9; k++) {
        initSpectrumTextures[k].resize(N * N);
    }
    m_LengthScales[0] = 128.0f;

    setSpectrumSettings(settings, param);

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            unsigned int seed = j + N * i + N;
            seed += m_Seed;

            float halfN = N / 2.0f;

            float deltaK = 2.0f * PI / m_LengthScales[0];
            // deltaK = 2.0f / m_LengthScales[0];
            glm::vec2 K = glm::vec2(j - halfN, i - halfN) * deltaK;
            float kLength = glm::length(K);

            seed += hash(seed) * 10;
            glm::vec4 uniformRandSamples = glm::vec4(hash(seed), hash(seed * 2), hash(seed * 3), hash(seed * 4));
            glm::vec2 gauss1 = UniformToGaussian(uniformRandSamples.x, uniformRandSamples.y);
            glm::vec2 gauss2 = UniformToGaussian(uniformRandSamples.z, uniformRandSamples.w);
            if (kLength >= m_LowCutoff && kLength <= m_HighCutoff) {
                float kAngle = std::atan2(K.y, K.x);

                float omega = Dispersion(kLength);
                float dOmegadk = DispersionDerivative(kLength);

                float spectrum = JONSWAP(omega, settings);
                float spread = DirectionSpectrum(kAngle, omega, settings);
                float shortWave = ShortWavesFade(kLength, settings);

                float result = spectrum * spread * shortWave;

                glm::vec2 res = glm::vec2(gauss2.x, gauss1.y) * sqrt(2 * result * abs(dOmegadk) / kLength * deltaK * deltaK);

                initSpectrumTextures[0][i * N + j] = glm::vec4(res.x, res.y, 0.0f, 0.0f);
                initSpectrumTextures[1][i * N + j] = glm::vec4(spectrum, 0.0f, 0.0f, 0.0f);
                initSpectrumTextures[2][i * N + j] = glm::vec4(spread, 0.0f, 0.0f, 0.0f);
                initSpectrumTextures[3][i * N + j] = glm::vec4(kLength, 0.0f, 0.0f, 0.0f);
            } else {
                initSpectrumTextures[0][i * N + j] = glm::vec4(0.0f);
                initSpectrumTextures[1][i * N + j] = glm::vec4(0.0f);
                initSpectrumTextures[2][i * N + j] = glm::vec4(0.0f);
                initSpectrumTextures[3][i * N + j] = glm::vec4(0.0f);
            }
        }
    }
}

void WaterFFT::conjugateSpectrum() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            glm::vec2 h0 = {initSpectrumTextures[0][i * N + j].r, initSpectrumTextures[0][i * N + j].g};
            int invI = (N - i) % N;
            int invJ = (N - j) % N;
            glm::vec2 h0conj = { initSpectrumTextures[0][invI * N + invJ].r, initSpectrumTextures[0][invI * N + invJ].g };

            initSpectrumTextures[0][i * N + j] = glm::vec4(h0, h0conj.x, -h0conj.y);
            initSpectrumTextures[4][i * N + j] = glm::vec4(h0conj.x, -h0conj.y, 0.0f, 0.0f);
        }
    }
}

void WaterFFT::updateSpectrum(float repeatTime, float frameTime) {

    for (int k = 0; k < 2; k++) {
        spectrumTextures[k].resize(N * N);
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            glm::vec4 initSignal = initSpectrumTextures[0][i * N + j];
            glm::vec2 h0 = glm::vec2(initSignal.r, initSignal.g);
            glm::vec2 h0conj = glm::vec2(initSignal.b, initSignal.a);

            float halfN = N / 2.0f;
            float x = (float)j - halfN;
            float y = (float)j - halfN;
            glm::vec2 K = glm::vec2(x, y);
            K *= (2.0f * PI / m_LengthScales[0]);
            float kMag = glm::length(K);
            float kMagRcp = 1.0f / kMag;

            if (kMag < 0.0001f) {
                kMagRcp = 1.0f;
            }

            float w_0 = 2.0f * PI / repeatTime;
            float dispersion = floor(sqrt(m_Gravity * kMag) / w_0) * w_0 * frameTime;

            glm::vec2 exponent = EulerFormula(dispersion);

            glm::vec2 htilde = ComplexMult(h0, exponent) + ComplexMult(h0conj, glm::vec2(exponent.x, -exponent.y));
            glm::vec2 ih = glm::vec2(-htilde.y, htilde.x);

            glm::vec2 displacementX = ih * K.x * kMagRcp;
            glm::vec2 displacementY = htilde;
            glm::vec2 displacementZ = ih * K.y * kMagRcp;

            glm::vec2 displacementX_dx = -htilde * K.x * K.x * kMagRcp;
            glm::vec2 displacementY_dx = ih * K.x;
            glm::vec2 displacementZ_dx = -htilde * K.x * K.y * kMagRcp;

            glm::vec2 displacementY_dz = ih * K.y;
            glm::vec2 displacementZ_dz = -htilde * K.y * K.y * kMagRcp;

            glm::vec2 htildeDisplacementX = glm::vec2(displacementX.x - displacementZ.y, displacementX.y + displacementZ.x);
            glm::vec2 htildeDisplacementZ = glm::vec2(displacementY.x - displacementZ_dx.y, displacementY.y + displacementZ_dx.x);

            glm::vec2 htildeSlopeX = glm::vec2(displacementY_dx.x - displacementY_dz.y, displacementY_dx.y + displacementY_dz.x);
            glm::vec2 htildeSlopeZ = glm::vec2(displacementX_dx.x - displacementZ_dz.y, displacementX_dx.y + displacementZ_dz.x);

            spectrumTextures[0][i * N + j] = glm::vec4(htildeDisplacementX, htildeDisplacementZ);
            spectrumTextures[1][i * N + j] = glm::vec4(htildeSlopeX, htildeSlopeZ);

            initSpectrumTextures[5][i * N + j] = spectrumTextures[0][i * N + j];
        }
    }
}

void WaterFFT::ButterflyValues(unsigned int step, unsigned int index, glm::ivec2& indices, glm::vec2& twiddle) {
    const float twoPi = 6.28318530718;
    unsigned int b = N >> (step + 1);
    unsigned int w = b * (index / b);
    unsigned int i = (w + index) % N;

    twiddle.y = glm::sin(-twoPi / N * w);
    twiddle.x = glm::cos(-twoPi / N * w);

    //This is what makes it the inverse FFT
    twiddle.y = -twiddle.y;
    indices = glm::ivec2(i, i + b);
}

glm::vec4 WaterFFT::FFT(unsigned int Index, glm::vec4 input) {
    fftGroupBuffer[0][Index] = input;
    bool flag = false;
    int logn = std::log2(N);
    for (int step = 0; step < logn; ++step) {
        glm::ivec2 inputsIndices;
        glm::vec2 twiddle;
        ButterflyValues(step, Index, inputsIndices, twiddle);

        glm::vec4 v = fftGroupBuffer[flag][inputsIndices.y];
        fftGroupBuffer[!flag][Index] = fftGroupBuffer[flag][inputsIndices.x] + glm::vec4(ComplexMult(twiddle, glm::vec2(v.x, v.y)), ComplexMult(twiddle, glm::vec2(v.z, v.w)));

        flag = !flag;
    }

    return fftGroupBuffer[flag][Index];
}

void WaterFFT::horizontalFFT() {
    for (int k = 0; k < 2; k++) {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                spectrumTextures[k][i * N + j] = FFT(i * N + j, spectrumTextures[k][i * N + j]);
            }
        }
    }
}

void WaterFFT::verticalFFT() {
    for (int k = 0; k < 2; k++) {
        for (int j = 0; j < N; j++) {
            for (int i = 0; i < N; i++) {
                spectrumTextures[k][i * N + j] = FFT(j * N + i, spectrumTextures[k][i * N + j]);
            }
        }
    }
}

void WaterFFT::setSpectrumSettings(spectrumSettings& spectrumSetting, spectrumParameter& spectrumParameter) {
    spectrumSetting.scale = spectrumParameter.scale;
    spectrumSetting.angle = spectrumParameter.windDirection / 180 * PI;
    spectrumSetting.spreadBlend = spectrumParameter.spreadBlend;
    spectrumSetting.swell = glm::clamp(spectrumParameter.swell, 0.01f, 1.0f);
    spectrumSetting.alpha = JonswapAlpha(spectrumParameter.fetch, spectrumParameter.windSpeed);
    spectrumSetting.peakOmega = JonswapPeakFrequency(spectrumParameter.fetch, spectrumParameter.windSpeed);
    spectrumSetting.gamma = spectrumParameter.peakEnhancement;
    spectrumSetting.shortWavesFade = spectrumParameter.shortWavesFade;
}

void WaterFFT::update(float frameTime) {
    updateSpectrum(200.0f, frameTime);
    fftGroupBuffer[0].resize(N * N);
    fftGroupBuffer[1].resize(N * N);
    horizontalFFT();
    verticalFFT();
    assembleMaps({ 1,1 }, 1.0f, 1.0f);
    for (int k = 0; k < 9; k++) {
        createQuad(0, k);
    }
}


glm::vec4 WaterFFT::Permute(glm::vec4 data, glm::vec3 ids) {
    float t = (1.0f - 2.0f * fmod((ids.x + ids.y), 2.0));
    return data * t;
}

void WaterFFT::assembleMaps(glm::vec2 lambda, float foamDecayRate, float foamBias) {

    displacementTextures.resize(N * N);
    slopeTextures.resize(N * N);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            initSpectrumTextures[6][i * N + j] = spectrumTextures[0][i * N + j];
            glm::vec4 htildeDisplacement = Permute(spectrumTextures[0][i * N + j], glm::vec3(i, j, 0.0f));
            glm::vec4 htildeSlope = Permute(spectrumTextures[1][i * N + j], glm::vec3(i, j, 0.0f));

            glm::vec2 dxdz = glm::vec2(htildeDisplacement.r, htildeDisplacement.g);
            glm::vec2 dydxz = glm::vec2(htildeDisplacement.b, htildeDisplacement.a);
            glm::vec2 dyxdyz = glm::vec2(htildeSlope.r, htildeSlope.g);
            glm::vec2 dxxdzz = glm::vec2(htildeSlope.b, htildeSlope.a);

            float jacobian = (1.0f + lambda.x * dxxdzz.x) * (1.0f + lambda.y * dxxdzz.y) - lambda.x * lambda.y * dydxz.y * dydxz.y;

            glm::vec3 displacement = glm::vec3(lambda.x * dxdz.x, dydxz.x, lambda.y * dxdz.y);

            glm::vec2 slopes = glm::vec2(dyxdyz.x, dyxdyz.y) / (glm::abs(dxxdzz * lambda) + glm::vec2(1.0));
            float covariance = slopes.x * slopes.y;

            float foam = displacementTextures[i * N + j].a;
            foam *= exp(-foamDecayRate);
            foam = std::max(0.0f, std::min(1.0f, foam));

            float biasedJacobian = std::max(0.0f, -(jacobian - foamBias));

            displacementTextures[i * N + j] = glm::vec4(displacement, foam);
            // printf("%f %f %f\n", displacementTextures[i * N + j].r, displacementTextures[i * N + j].g, displacementTextures[i * N + j].b);
            slopeTextures[i * N + j] = glm::vec4(slopes, 0.0f, 0.0f);
            initSpectrumTextures[7][i * N + j] = glm::vec4(displacementTextures[i * N + j].r, 0.0f, 0.0f, 0.0f) * 10000000.0f;
            initSpectrumTextures[8][i * N + j] = slopeTextures[i * N + j];
        }
    }

}

void WaterFFT::createQuad(int lod, int k) {
    glm::vec3 pos[4] = { 
        {-0.5f, 0.0f, -0.5f},
        { 0.5f, 0.0f, -0.5f},
        { 0.5f, 0.0f,  0.5f},
        {-0.5f, 0.0f,  0.5f},
    };
    unsigned int indices[6] = {
        0,2,3,  1,2,0
    };
    quadPlane qp;
    glGenVertexArrays(1, &qp.vao);
    glBindVertexArray(qp.vao);

    glGenBuffers(1, &qp.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, qp.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec3), &pos[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
    glEnableVertexAttribArray(0);

    glGenTextures(1, &qp.tex);
    glBindTexture(GL_TEXTURE_2D, qp.tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, N, N, 0, GL_RGBA, GL_FLOAT, &initSpectrumTextures[k].at(0));
    
    glBindVertexArray(0);
    m_Quad.push_back(qp);
}


void WaterFFT::drawTexture(Shader* shader, glm::mat4 projection, glm::mat4 view) {

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    shader->use();

    float temp = 1.1f;

    for (int i = 0; i < m_Quad.size(); i++) {
        quadPlane qp = m_Quad[i];
        glBindVertexArray(qp.vao);

        glm::mat4 m(1.0f);
        m = glm::translate(m, glm::vec3(0.0f, 4.0f, 0.0f));
        m = glm::scale(m, glm::vec3(1.0f));

        m = glm::translate(m, glm::vec3(temp * i, 0.0f, 0.0f));

        shader->setMat4("projection", projection);
        shader->setMat4("view", view);
        shader->setMat4("model", m);

        shader->setInt("Textures", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, qp.tex);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, qp.ebo);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);

        glBindVertexArray(0);
    }

    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
