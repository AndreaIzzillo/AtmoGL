#version 430 core

#include "atmosphere_common.glsl"

in vec2 vUv;

layout(binding = 0) uniform sampler2D uSkyViewLut;
layout(binding = 1) uniform sampler2D uTransmittanceLut;

uniform mat4 uInverseProjection;
uniform mat4 uInverseView;
uniform vec3 uCameraPosition;
uniform vec3 uSunDirection;
uniform vec3 uSunIlluminance;
uniform float uSunAngularRadius;

out vec4 fragColor;

void main()
{
    vec2 ndc = vUv * 2.0 - 1.0;

    vec4 cameraPoint =
        uInverseProjection * vec4(ndc, -1.0, 1.0);

    vec3 cameraDirection =
        normalize(cameraPoint.xyz / cameraPoint.w);

    vec3 worldDirection =
        normalize(mat3(uInverseView) * cameraDirection);

    vec2 uv = skyViewDirectionToUv(
        worldDirection,
        uCameraPosition,
        textureSize(uSkyViewLut, 0)
    );

    vec3 radiance = texture(uSkyViewLut, uv).rgb;

    float r = length(uCameraPosition);
    float muView = dot(uCameraPosition / r, worldDirection);
    bool inSunDisk = dot(worldDirection, uSunDirection) >= cos(uSunAngularRadius);

    if (inSunDisk && !rayIntersectsGround(r, muView))
    {
        vec2 transmittanceUv = transmittanceRMuToUv(
            r,
            muView,
            textureSize(uTransmittanceLut, 0)
        );

        vec3 transmittance =
            textureLod(uTransmittanceLut, transmittanceUv, 0.0).rgb;

        radiance += transmittance * uSunIlluminance * 20.0;
    }

    fragColor = vec4(radiance, 1.0);
}