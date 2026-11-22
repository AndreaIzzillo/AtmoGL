#include "atmosphere/Atmosphere.hpp"

namespace atmogl
{
    Atmosphere::Atmosphere()
        : transmittanceShader("shaders/transmittance.comp")
        , transmittanceLut(256, 64)
        , skyViewShader("shaders/sky_view.comp")
        , skyViewLut(200, 100)
        , aerialPerspectiveShader("shaders/aerial_perspective.comp")
        , aerialPerspectiveLut(32, 32, 32)
    {
        skyViewLut.bind(1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Atmosphere::computeTransmittance(const AtmosphereParameters &parameters)
    {
        transmittanceShader.use();
        setCommonUniforms(transmittanceShader, parameters);

        transmittanceLut.bindImage(0, GL_WRITE_ONLY);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glDispatchCompute((transmittanceLut.getWidth() + 7) / 8,
                          (transmittanceLut.getHeight() + 7) / 8, 1);
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    }

    void Atmosphere::computeSkyView(const AtmosphereParameters &parameters,
                                    const glm::vec3 &cameraPosition)
    {
        skyViewShader.use();
        setCommonUniforms(skyViewShader, parameters);

        skyViewShader.setVec3("uCameraPosition", cameraPosition);
        skyViewShader.setVec3("uSunDirection", parameters.sunDirection());
        skyViewShader.setVec3("uSunIlluminance", parameters.sunIlluminance);
        skyViewShader.setFloat("uMieAsymmetry", parameters.mieAsymmetry);

        transmittanceLut.bind(0);
        skyViewLut.bindImage(1, GL_WRITE_ONLY);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glDispatchCompute((skyViewLut.getWidth() + 7) / 8, (skyViewLut.getHeight() + 7) / 8, 1);
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    }

    void Atmosphere::computeAerialPerspective(const AtmosphereParameters &parameters,
                                              const glm::vec3 &cameraPosition,
                                              const glm::mat4 &inverseProjection,
                                              const glm::mat4 &inverseView, float maxDepth)
    {
        aerialPerspectiveShader.use();
        setCommonUniforms(aerialPerspectiveShader, parameters);

        aerialPerspectiveShader.setVec3("uCameraPosition", cameraPosition);
        aerialPerspectiveShader.setMat4("uInverseProjection", inverseProjection);
        aerialPerspectiveShader.setMat4("uInverseView", inverseView);
        aerialPerspectiveShader.setFloat("uAerialPerspectiveMaxDepth", maxDepth);

        aerialPerspectiveShader.setVec3("uSunDirection", parameters.sunDirection());
        aerialPerspectiveShader.setVec3("uSunIlluminance", parameters.sunIlluminance);
        aerialPerspectiveShader.setFloat("uMieAsymmetry", parameters.mieAsymmetry);

        transmittanceLut.bind(0);
        aerialPerspectiveLut.bindImage(2, GL_WRITE_ONLY);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glDispatchCompute((aerialPerspectiveLut.getWidth() + 3) / 4,
                          (aerialPerspectiveLut.getHeight() + 3) / 4,
                          (aerialPerspectiveLut.getDepth() + 3) / 4);

        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    }

    const Texture2D &Atmosphere::getTransmittanceLut() const
    {
        return transmittanceLut;
    }

    const Texture2D &Atmosphere::getSkyViewLut() const
    {
        return skyViewLut;
    }

    const Texture3D &Atmosphere::getAerialPerspectiveLut() const
    {
        return aerialPerspectiveLut;
    }

    void Atmosphere::setCommonUniforms(Shader &shader, const AtmosphereParameters &parameters)
    {
        shader.setFloat("uGroundRadius", parameters.groundRadius);
        shader.setFloat("uTopRadius", parameters.AtmosphereRadius);
        shader.setFloat("uRayleighScaleHeight", parameters.rayleighScaleHeight);
        shader.setFloat("uMieScaleHeight", parameters.mieScaleHeight);
        shader.setFloat("uOzoneCenterHeight", parameters.ozoneCenterHeight);
        shader.setFloat("uOzoneHalfWidth", parameters.ozoneHalfWidth);
        shader.setVec3("uRayleighScattering", parameters.rayleighScattering);
        shader.setVec3("uMieScattering", parameters.mieScattering);
        shader.setVec3("uRayleighExtinction",
                       parameters.rayleighScattering + parameters.rayleighAbsorption);
        shader.setVec3("uMieExtinction", parameters.mieScattering + parameters.mieAbsorption);
        shader.setVec3("uOzoneExtinction", parameters.ozoneScattering + parameters.ozoneAbsorption);
    }
}
