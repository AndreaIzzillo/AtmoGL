#pragma once

#include "atmosphere/AtmosphereParameters.hpp"
#include "graphics/Shader.hpp"
#include "graphics/Texture2D.hpp"
#include "graphics/Texture3D.hpp"

namespace atmogl
{
    class Atmosphere
    {
    public:
        Atmosphere();
        ~Atmosphere() = default;

        Atmosphere(const Atmosphere &) = delete;
        Atmosphere &operator=(const Atmosphere &) = delete;

        void computeTransmittance(const AtmosphereParameters &parameters);
        void computeSkyView(const AtmosphereParameters &parameters,
                            const glm::vec3 &cameraPosition);
        void computeAerialPerspective(const AtmosphereParameters &parameters,
                                      const glm::vec3 &cameraPosition,
                                      const glm::mat4 &inverseProjection,
                                      const glm::mat4 &inverseView, float maxDepth,
                                      const glm::mat4 &lightViewProjection, float shadowBias);

        void setCommonUniforms(Shader &shader, const AtmosphereParameters &parameters);

        const Texture2D &getTransmittanceLut() const;
        const Texture2D &getSkyViewLut() const;
        const Texture3D &getAerialPerspectiveLut() const;

    private:
        Shader transmittanceShader;
        Texture2D transmittanceLut;

        Shader skyViewShader;
        Texture2D skyViewLut;

        Shader aerialPerspectiveShader;
        Texture3D aerialPerspectiveLut;
    };
}
