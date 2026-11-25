#pragma once

#include <glm/glm.hpp>

namespace atmogl
{
    struct AtmosphereParameters
    {
        float groundRadius = 6360e3f;
        float AtmosphereRadius = 6460e3f;

        float rayleighScaleHeight = 8e3f;
        float mieScaleHeight = 1.2e3f;

        float ozoneCenterHeight = 25e3f;
        float ozoneHalfWidth = 15e3f;

        float mieAsymmetry = 0.8f;
        float mieDensityMultiplier = 1.0f;

        glm::vec3 rayleighScattering = glm::vec3(5.802e-6f, 13.558e-6f, 33.1e-6f);
        glm::vec3 rayleighAbsorption = glm::vec3(0.0f);

        glm::vec3 mieScattering = glm::vec3(3.996e-6f);
        // glm::vec3 mieAbsorption = glm::vec3(4.40e-6f);
        glm::vec3 mieAbsorption = glm::vec3(0.444e-6f);

        glm::vec3 ozoneAbsorption = glm::vec3(0.650e-6f, 1.881e-6f, 0.085e-6f);
        glm::vec3 ozoneScattering = glm::vec3(0.0f);

        float sunElevation = 45.0f;
        float sunAzimuth = 0.0f;
        float sunAngularRadius = glm::radians(0.265f);

        glm::vec3 sunIlluminance = glm::vec3(1.0f);

        glm::vec3 sunDirection() const
        {
            return glm::normalize(
                glm::vec3(cos(glm::radians(sunAzimuth)) * cos(glm::radians(sunElevation)),
                          sin(glm::radians(sunElevation)),
                          sin(glm::radians(sunAzimuth)) * cos(glm::radians(sunElevation))));
        }
    };
}
