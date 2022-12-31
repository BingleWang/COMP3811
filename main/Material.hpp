#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Material {
    glm::vec3 Kd;
    glm::vec3 Ka;
    glm::vec3 Ks;
    glm::vec3 Ke;
    float shininess;
    float reflectionFactor;
    float refractionFactor;
    // relative index of refraction(n1/n2)
    float ior;
    float eta;
    bool hasNormalMap = false;
};