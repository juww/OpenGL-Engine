#include "water.h"

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
float hash(unsigned int n) {
    // integer hash copied from Hugo Elias
    n = (n << 13U) ^ n;
    n = n * (n * n * 15731U + 0x789221U) + 0x1376312589U;
    return float(n & unsigned int(0x7fffffffU)) / float(0x7fffffff);
}

glm::vec2 UniformToGaussian(float u1, float u2) {
    float R = sqrt(-2.0f * log(u1));
    float theta = 2.0f * PI * u2;

    return glm::vec2(R * cos(theta), R * sin(theta));
}

float ShortWavesFade(float kLength, float shortWavesFade) {
    return exp(-shortWavesFade * shortWavesFade * kLength * kLength);
}

float min(float a, float b) {
    return a <= b ? a : b;
}

float _Gravity = 9.81f;
float _Depth = 20.0f;

float Dispersion(float kMag) {
    return sqrt(_Gravity * kMag * tanh(min(kMag * _Depth, 20)));
}

glm::vec3 spectra2D[129 * 129];
void Water::createSpectrum(int N) {
    float halfN = (float)N * 0.5f;
    int n = (N + 1);

    std::vector<glm::vec3> tpos(n*n);
    std::vector<unsigned int> tIndices;

    for (int i = 0; i <= N; i++) {
        for (int j = 0; j <= N; j++) {
            float x = j - halfN;
            float z = i - halfN;

            float kLength = glm::length(glm::vec2(x, z));
            float kAngle = std::atan2(x, z);
            float omega = Dispersion(kLength);
            float V = 200.0f;
            float spectrum = PhillipsSpectrum(omega, (V * V) / _Gravity) * ShortWavesFade(kLength, 0.001f);

            unsigned int seed = x + N * z + N;
            seed += 1234;
            glm::vec4 uniformRandSamples = glm::vec4(hash(seed), hash(seed * 2), hash(seed * 3), hash(seed * 4));
            glm::vec2 gauss1 = UniformToGaussian(uniformRandSamples.x, uniformRandSamples.y);
            glm::vec2 gauss2 = UniformToGaussian(uniformRandSamples.z, uniformRandSamples.w);
            float g = gauss1.y + gauss2.x;
            float h = 1 / sqrt(2) * g * sqrt(spectrum);
            if (0.0001f > kLength || kLength > 9000.0f) {
                printf("lel\n");
                h = 0.0f;
            }
            h = glm::max(h, 0.0f) * 10.0f;
            printf("h - %f\n", h);
            spectra2D[i * n + j] = glm::vec3(h, 0.0f , 0.0f);
            tpos[i * n + j] = { x, 0.0f, z };
        }
    }


    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            unsigned int indx1 = i * n + j;
            unsigned int indx2 = i * n + j + 1;
            unsigned int indx3 = (i + 1) * n + j;
            unsigned int indx4 = (i + 1) * n + j + 1;

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

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 129, 129, 0, GL_RGB, GL_FLOAT, spectra2D);

    glBindVertexArray(0);
}

void Water::drawSpectrum(Shader* shader, glm::mat4 projection, glm::mat4 view) {

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    shader->use();

    glm::mat4 m(1.0f);
    m = glm::translate(m, glm::vec3(0.0f, 4.0f, 0.0f));
    m = glm::scale(m, glm::vec3(0.1f, 0.1f, 0.1f));
    shader->setMat4("model", m);
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);

    glBindVertexArray(tvao);

    shader->setInt("Textures", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ttex);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tebo);
    glDrawElements(GL_TRIANGLES, tsizeIndices, GL_UNSIGNED_INT, (void*)0);

    glBindVertexArray(0);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}



//void Water::JONSWAPSpectrum(float alpha, float beta, float wpeak, float gamma, float g, float rho) {
//
//}

float Water::PhillipsSpectrum(float k, float L) {
    float kL = (k * L);
    return glm::exp(-1 / (kL * kL)) / std::pow(k, 4);
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

