#pragma once

#include "atmosphere/AtmosphereParameters.hpp"
#include "graphics/Shader.hpp"
#include "graphics/Texture2D.hpp"

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

        const Texture2D &getTransmittanceLut() const;
        const Texture2D &getSkyViewLut() const;

    private:
        Shader transmittanceShader;
        Texture2D transmittanceLut;

        Shader skyViewShader;
        Texture2D skyViewLut;

        void setCommonUniforms(Shader &shader, const AtmosphereParameters &parameters);
    };
}
