#include "water.h"
#include "interpolate.h"
#define PI 3.14159265358979323846

Water::Water() {
    m_Vao = m_Ebo = 0;
    m_Vertices.clear();
    m_Indices.clear();
}

Water::~Water() {

}

void Water::initialize(const int& width, const int& height, const float& scale) {
    m_Model = glm::mat4(1.0f);
    //m_Model = glm::scale(m_Model, glm::vec3(0.1));
    m_Model = glm::translate(m_Model, { -width / (scale * 2.0), 5.0f, -height / (scale * 1.05)});
    
    m_Width = width;
    m_Height = height;
    m_Scale = scale;

    glGenVertexArrays(1, &m_Vao);
    glBindVertexArray(m_Vao);

    setupIndexMap();
    setupIndices();
    setupVectices();

    glBindVertexArray(0);
}

void Water::setParameter(Shader *shader, float& _a, float& _f, float& _t, float& _s, float& seed, float& iter, int& waveCount, glm::vec3 &cameraPos) {
    shader->use();
    shader->setFloat("_amplitude", _a);
    shader->setFloat("_frequency", 2.0f/_f);
    shader->setFloat("_time", _t);
    shader->setFloat("_speed", _s);
    shader->setFloat("_seed", seed);
    shader->setFloat("_iter", 1.234f);
    shader->setInt("_waveCount", waveCount);
    shader->setVec3("lightDirection", glm::vec3(1.0f, -1.0f, 1.0f));
    shader->setVec3("viewPos", cameraPos);
}


void Water::draw(Shader* shader, glm::mat4 projection, glm::mat4 view) {

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    shader->use();

    shader->setMat4("model", m_Model);
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);

    glBindVertexArray(m_Vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ebo);
    glDrawElements(GL_TRIANGLE_STRIP, m_Indices.size(), GL_UNSIGNED_INT, (void*)0);

    glBindVertexArray(0);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Water::drawNormalLine(Shader* shader, glm::mat4 projection, glm::mat4 view) {

    shader->use();

    shader->setMat4("model", m_Model);
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);

    glBindVertexArray(m_Vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ebo);
    glDrawElements(GL_POINTS, m_Indices.size(), GL_UNSIGNED_INT, (void*)0);

    glBindVertexArray(0);

}
////////////////////////////////////////////////////////////////////////////////
//buat di GUI
float _Gravity = 9.81f;
float _Depth = 20.0f;
float _swell = 0.5f;
float _peakOmega = 10.0f;
float _angle = 3.14f;
float _spreadBlend = 0.5f;

float _fetch = 800.0f;
float _windSpeed = 800.0f;
float _gamma = 3.3f;
float _scale = 1.0f;
//
float JonswapAlpha(float fetch, float windSpeed) {
    return 0.076f * std::pow(_Gravity * fetch / windSpeed / windSpeed, -0.22f);
}


float JonswapPeakFrequency(float fetch, float windSpeed) {
    return 22.0f * std::pow(windSpeed * fetch / _Gravity / _Gravity, -0.33f);
}

float min(float a, float b) {
    return a <= b ? a : b;
}

float max(float a, float b) {
    return a >= b ? a : b;
}

float hash(unsigned int n) {
    // integer hash copied from Hugo Elias
    n = (n << 13U) ^ n;
    n = n * (n * n * 15731U + 0x789221U) + 0x1376312589U;
    return float(n & unsigned int(0x7fffffffU)) / float(0x7fffffff);
}

glm::vec2 ComplexMult(glm::vec2 a, glm::vec2 b) {
    return glm::vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

glm::vec2 UniformToGaussian(float u1, float u2) {
    float R = sqrt(-2.0f * log(u1));
    float theta = 2.0f * PI * u2;

    return glm::vec2(R * cos(theta), R * sin(theta));
}

float NormalizationFactor(float s) {
    float s2 = s * s;
    float s3 = s2 * s;
    float s4 = s3 * s;
    if (s < 5) return -0.000564f * s4 + 0.00776f * s3 - 0.044f * s2 + 0.192f * s + 0.163f;
    else return -4.80e-08f * s4 + 1.07e-05f * s3 - 9.53e-04f * s2 + 5.90e-02f * s + 3.93e-01f;
}

float Cosine2s(float theta, float s) {
    return NormalizationFactor(s) * pow(abs(cos(0.5f * theta)), 2.0f * s);
}

float SpreadPower(float omega, float peakOmega) {
    if (omega > peakOmega)
        return 9.77f * pow(abs(omega / peakOmega), -2.5f);
    else
        return 6.97f * pow(abs(omega / peakOmega), 5.0f);
}

float DirectionSpectrum(float theta, float omega) {
    float s = SpreadPower(omega, _peakOmega) + 16 * tanh(min(omega / _peakOmega, 20)) * _swell * _swell;
    return interpolate::lerp(2.0f / 3.1415f * cos(theta) * cos(theta), Cosine2s(theta - _angle, s), _spreadBlend);
}

float ShortWavesFade(float kLength, float shortWavesFade) {
    return exp(-shortWavesFade * shortWavesFade * kLength * kLength);
}

float Dispersion(float kMag) {
    return sqrt(_Gravity * kMag * tanh(min(kMag * _Depth, 20)));
}

float DispersionDerivative(float kMag) {
    float th = tanh(min(kMag * _Depth, 20));
    float ch = cosh(kMag * _Depth);
    return _Gravity * (_Depth * kMag / ch / ch + th) / Dispersion(kMag) / 2.0f;
}

//float EulerFormula(float w) {
//    return glm::cos(w) + glm::sin(w);
//}

glm::vec2 EulerFormula(float w) {
    return glm::vec2(cos(w), sin(w));
}

const int N_SIZE = 64;
const int LOG_SIZE = 6;
const int MAPSIZE = N_SIZE * N_SIZE;
glm::vec4 spectra2D[MAPSIZE], spectrumTextures[2][MAPSIZE];
glm::vec4 fourierTarget[2][MAPSIZE];
glm::vec4 displacementTextures[MAPSIZE], slopeTextures[MAPSIZE];
float heigthMap[MAPSIZE];

void Water::createSpectrum(int N) {
    float halfN = (float)N * 0.5f;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            float x = (j - halfN);
            float z = (i - halfN);

            float deltaK = 2.0f * PI / N_SIZE;

            float kLength = glm::length(glm::vec2(z, x)) * deltaK;
            float kAngle = std::atan2(z, x);

            float omega = Dispersion(kLength);
            float dOmegadk = DispersionDerivative(kLength);

            _peakOmega = JonswapPeakFrequency(_fetch, _windSpeed);

            //float V = 200.0f;
            //float spectrum = PhillipsSpectrum(omega, (V * V) / _Gravity) * ShortWavesFade(kLength, 0.01f) * DirectionSpectrum(kAngle, omega);
            float spectrum = JONSWAPSpectrum(omega) * ShortWavesFade(kLength, 0.001f) * DirectionSpectrum(kAngle, omega);

            unsigned int seed = x + N * z + N;
            seed += 72637;
            glm::vec4 uniformRandSamples = glm::vec4(hash(seed), hash(seed * 2), hash(seed * 3), hash(seed * 4));
            glm::vec2 gauss1 = UniformToGaussian(uniformRandSamples.x, uniformRandSamples.y);
            glm::vec2 gauss2 = UniformToGaussian(uniformRandSamples.z, uniformRandSamples.w);
            float g = gauss1.y + gauss2.x;
            glm::vec2 ht = glm::vec2(gauss2.x, gauss1.y) * sqrt(2 * spectrum * abs(dOmegadk) / kLength * deltaK * deltaK);
            float h = 1 / sqrt(2) * g * sqrt(spectrum);
            if (0.0001f > kLength || kLength > 9000.0f) {
                printf("lel\n");
                h = 0.0f;
                ht = glm::vec2(0.0f);
            }
            //printf("spectrum - %f\n", spectrum);
            //printf("h - %f\n", h);
            //
            //printf("ht - %f %f\n", ht.x, ht.y);
            spectra2D[i * N + j] = glm::vec4(ht.x, ht.y, 0.0f, 0.0f);
        }
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {

            glm::vec2 h0 = glm::vec2(spectra2D[i * N + j].r, spectra2D[i * N + j].g);
            int invI = (N - i) % N;
            int invJ = (N - j) % N;
            glm::vec2 h0conj = glm::vec2(spectra2D[invI * N + invJ].r, spectra2D[invI * N + invJ].g);

            spectra2D[i * N + j] = glm::vec4(h0, h0conj.x, -h0conj.y);
        }
    }
}

void Water::spectrumPlane(int N) {
    N *= 2;
    float halfN = (float)N * 0.5f * 0.5f;
    int nn = N * N;

    std::vector<glm::vec3> tpos(N * N);
    std::vector<unsigned int> tIndices;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            float x = (j * 0.5f) - halfN;
            float z = (i * 0.5f) - halfN;

            tpos[i * N + j] = { x, 0.0f, z };
        }
    }

    for (int i = 0; i < N - 1; i++) {
        for (int j = 0; j < N - 1; j++) {
            unsigned int indx1 = i * N + j;
            unsigned int indx2 = i * N + j + 1;
            unsigned int indx3 = (i + 1) * N + j;
            unsigned int indx4 = (i + 1) * N + j + 1;

            tIndices.push_back(indx1);
            tIndices.push_back(indx2);
            tIndices.push_back(indx3);

            tIndices.push_back(indx4);
            tIndices.push_back(indx3);
            tIndices.push_back(indx2);
        }
    }

    printf("asu\n");
    printf("%d %d\n", tpos.size(), tIndices.size());
    tsizeIndices = tIndices.size();

    glGenVertexArrays(1, &tvao);
    glBindVertexArray(tvao);

    glGenBuffers(1, &tebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, tIndices.size() * sizeof(unsigned int), &tIndices.at(0), GL_STATIC_DRAW);

    unsigned int tvbo;
    glGenBuffers(1, &tvbo);
    glBindBuffer(GL_ARRAY_BUFFER, tvbo);
    glBufferData(GL_ARRAY_BUFFER, tpos.size() * sizeof(glm::vec3), &tpos.at(0), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glGenTextures(1, &ttex);
    glBindTexture(GL_TEXTURE_2D, ttex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, N_SIZE, N_SIZE, 0, GL_RGB, GL_FLOAT, spectra2D);

    glBindVertexArray(0);
}

void Water::updateSpectrum(int N, float repeatTime, float frameTime) {

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            glm::vec4 initSignal = spectra2D[i * N + j];
            glm::vec2 h0 = glm::vec2(initSignal.r, initSignal.g);
            glm::vec2 h0conj = glm::vec2(initSignal.b, initSignal.a);

            float halfN = N / 2.0f;
            float x = (float)j - halfN;
            float y = (float)j - halfN;
            glm::vec2 K = glm::vec2(x, y);
            K *= (2.0f * PI / N_SIZE);
            float kMag = glm::length(K);
            float kMagRcp = 1.0f / kMag;

            if (kMag < 0.0001f) {
                kMagRcp = 1.0f;
            }

            float w_0 = 2.0f * PI / repeatTime;
            float dispersion = floor(sqrt(_Gravity * kMag) / w_0) * w_0 * frameTime;

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

            fourierTarget[0][i * N + j] = glm::vec4(htildeDisplacementX, htildeDisplacementZ);
            fourierTarget[1][i * N + j] = glm::vec4(htildeSlopeX, htildeSlopeZ);
        }
    }
}
const int SIZE = N_SIZE;
void Water::ButterflyValues(unsigned int step, unsigned int index, glm::ivec2& indices, glm::vec2& twiddle) {
    const float twoPi = 6.28318530718;
    unsigned int b = SIZE >> (step + 1);
    unsigned int w = b * (index / b);
    unsigned int i = (w + index) % SIZE;

    twiddle.y = glm::sin(-twoPi / SIZE * w);
    twiddle.x = glm::cos(-twoPi / SIZE * w);

    //This is what makes it the inverse FFT
    twiddle.y = -twiddle.y;
    indices = glm::ivec2(i, i + b);
}
glm::vec4 fftGroupBuffer[2][SIZE];
glm::vec4 Water::FFT(unsigned int Index, glm::vec4 input) {
    fftGroupBuffer[0][Index] = input;
    bool flag = false;

    for (int step = 0; step < LOG_SIZE; ++step) {
        glm::ivec2 inputsIndices;
        glm::vec2 twiddle;
        ButterflyValues(step, Index, inputsIndices, twiddle);

        glm::vec4 v = fftGroupBuffer[flag][inputsIndices.y];
        fftGroupBuffer[!flag][Index] = fftGroupBuffer[flag][inputsIndices.x] + glm::vec4(ComplexMult(twiddle, glm::vec2(v.x,v.y)), ComplexMult(twiddle, glm::vec2(v.z,v.w)));

        flag = !flag;
    }

    return fftGroupBuffer[flag][Index];
}

void Water::horizontalFFT(int N) {
    for (int k = 0; k < 2; k++) {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < j < N; j++) {
                fourierTarget[k][i * N + j] = FFT(i * N + j, fourierTarget[k][i * N + j]);
            }
        }
    }
}

void Water::verticalFFT(int N) {
    for (int k = 0; k < 2; k++) {
        for (int j = 0; j < N; j++) {
            for (int i = 0; i < N; i++) {
                fourierTarget[k][i * N + j] = FFT(j * N + i, fourierTarget[k][i * N + j]);
            }
        }
    }
}

glm::vec4 Permute(glm::vec4 data, glm::vec3 ids) {
    float t = (1.0f - 2.0f * fmod((ids.x + ids.y), 2.0));
    return data * t;
}

void assembleMaps(int N, glm::vec2 lambda, float foamDecayRate, float foamBias) {

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            glm::vec4 htildeDisplacement = Permute(spectrumTextures[0][i * N + j], glm::vec3(i,j,0.0f));
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
            foam = max(0.0f, min(1.0f, foam));

            float biasedJacobian = max(0.0f, -(jacobian - foamBias));

            displacementTextures[i * N + j] = glm::vec4(displacement, foam);
            slopeTextures[i * N + j] = glm::vec4(slopes, 0.0f, 0.0f);
        }
    }

}

void Water::drawSpectrum(Shader* shader, glm::mat4 projection, glm::mat4 view) {

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    shader->use();

    glm::mat4 m(1.0f);
    m = glm::translate(m, glm::vec3(0.0f, 4.0f, 0.0f));
    m = glm::scale(m, glm::vec3(1.0f));
    shader->setMat4("model", m);
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);
    shader->setFloat("nsize", 128);

    glBindVertexArray(tvao);

    shader->setInt("Textures", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ttex);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tebo);
    glDrawElements(GL_TRIANGLES, tsizeIndices, GL_UNSIGNED_INT, (void*)0);

    glBindVertexArray(0);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

float TMACorrection(float omega) {
    float omegaH = omega * sqrt(_Depth / _Gravity);
    if (omegaH <= 1.0f)
        return 0.5f * omegaH * omegaH;
    if (omegaH < 2.0f)
        return 1.0f - 0.5f * (2.0f - omegaH) * (2.0f - omegaH);

    return 1.0f;
}


float Water::JONSWAPSpectrum(float omega) {
    
    float _alpha = JonswapAlpha(_fetch, _windSpeed);

    float sigma = (omega <= _peakOmega) ? 0.07f : 0.09f;

    float r = exp(-(omega - _peakOmega) * (omega - _peakOmega) / 2.0f / sigma / sigma / _peakOmega / _peakOmega);

    float oneOverOmega = 1.0f / omega;
    float peakOmegaOverOmega = _peakOmega / omega;
    float TMA = _scale * TMACorrection(omega);
    return TMA * _alpha * _Gravity * _Gravity
        * oneOverOmega * oneOverOmega * oneOverOmega * oneOverOmega * oneOverOmega
        * exp(-1.25f * peakOmegaOverOmega * peakOmegaOverOmega * peakOmegaOverOmega * peakOmegaOverOmega)
        * pow(abs(_gamma), r);
}

float Water::PhillipsSpectrum(float k, float L) {
    float kL = (k * L);
    return glm::exp(-1 / (kL * kL)) / (k * k * k * k);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Water::setupIndexMap() {
    m_IndexMap.clear();
    m_IndexMap.resize(m_Height);
    int idx = 0;
    for (int i = 0; i < m_Height; i++) {
        for (int j = 0; j < m_Width + 2; j++) {
            m_IndexMap[i].push_back(idx);
            if (i == m_Height - 1) {
                if (j == 0 || j == m_Width + 1) {
                    m_IndexMap[i][j] = -1;
                    continue;
                }
            }
            idx++;
        }
    }
}

void Water::setupIndices() {

    glGenBuffers(1, &m_Ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ebo);
    for (int i = 0; i < m_Height - 1; i++) {
        for (int j = 0; j < m_Width + 2; j++) {
            m_Indices.push_back(m_IndexMap[i][j]);
            if (j > 0 && j < m_Width + 1) {
                m_Indices.push_back(m_IndexMap[i + 1][j]);
            }
        }
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(unsigned int), &m_Indices.at(0), GL_STATIC_DRAW);
}

void Water::setupVectices() {
    for (int i = 0; i < m_Height; i++) {
        for (int j = 0; j < m_Width + 2; j++) {
            if (m_IndexMap[i][j] == -1) continue;
            m_Vertices.push_back(glm::vec3(j - 1, 0.0, i));
            if (j == 0) {
                m_Vertices[m_Vertices.size() - 1].x = j;
            }
            if (j == m_Width + 1) {
                m_Vertices[m_Vertices.size() - 1] = glm::vec3(j - 2, 0.0, i + 1);
            }
            m_Vertices[m_Vertices.size() - 1] = m_Vertices[m_Vertices.size() - 1] / m_Scale;
        }
    }
    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    unsigned int sizeBuffer = m_Vertices.size() * 3 * sizeof(float);
    glBufferData(GL_ARRAY_BUFFER, sizeBuffer, &m_Vertices.at(0), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
    glEnableVertexAttribArray(0);

}

