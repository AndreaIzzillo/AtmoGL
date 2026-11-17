#include "atmosphere/Atmosphere.hpp"

namespace atmogl
{
    Atmosphere::Atmosphere()
        : transmittanceShader("shaders/transmittance.comp")
        , transmittanceLut(256, 64)
    {}

    void Atmosphere::computeTransmittance(const AtmosphereParameters &parameters)
    {
        transmittanceShader.use();
        transmittanceShader.setFloat("uGroundRadius", parameters.groundRadius);
        transmittanceShader.setFloat("uTopRadius", parameters.AtmosphereRadius);
        transmittanceShader.setFloat("uRayleighScaleHeight", parameters.rayleighScaleHeight);
        transmittanceShader.setFloat("uMieScaleHeight", parameters.mieScaleHeight);
        transmittanceShader.setFloat("uOzoneCenterHeight", parameters.ozoneCenterHeight);
        transmittanceShader.setFloat("uOzoneHalfWidth", parameters.ozoneHalfWidth);

        transmittanceShader.setVec3("uRayleighExtinction",
                                    parameters.rayleighScattering + parameters.rayleighAbsorption);
        transmittanceShader.setVec3("uMieExtinction",
                                    parameters.mieScattering + parameters.mieAbsorption);
        transmittanceShader.setVec3("uOzoneExtinction",
                                    parameters.ozoneScattering + parameters.ozoneAbsorption);

        transmittanceLut.bindImage(0, GL_WRITE_ONLY);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glDispatchCompute((transmittanceLut.getWidth() + 7) / 8,
                          (transmittanceLut.getHeight() + 7) / 8, 1);
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    }

    const Texture2D &Atmosphere::getTransmittanceLut() const
    {
        return transmittanceLut;
    }
}
