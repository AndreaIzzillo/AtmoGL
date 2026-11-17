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

        const Texture2D &getTransmittanceLut() const;

    private:
        Shader transmittanceShader;
        Texture2D transmittanceLut;
    };
}
