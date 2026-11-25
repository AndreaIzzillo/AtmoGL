#version 430 core

#include "atmosphere_common.glsl"

in vec3 vWorldPosition;
in vec3 vWorldNormal;
in vec3 vColor;

out vec4 fragColor;

void main()
{
    const float ambient = 0.01;

    vec3 planetPosition = vWorldPosition + vec3(0.0, uGroundRadius, 0.0);

    float r = length(planetPosition);
    float muSun = clamp(
        dot(planetPosition / r, uSunDirection),
        -1.0,
        1.0
    );

    vec3 sunTransmittance = vec3(0.0);

    if (!rayIntersectsGround(r, muSun))
    {
        vec2 uv = transmittanceRMuToUv(
            r,
            muSun,
            textureSize(uTransmittanceLut, 0)
        );

        sunTransmittance = textureLod(uTransmittanceLut, uv, 0.0).rgb;
    }

    float diffuse = max(dot(normalize(vWorldNormal), uSunDirection), 0.0);

    float visibility = computeSunVisibility(vWorldPosition);
    vec3 directLight = uSunIlluminance * sunTransmittance * diffuse * visibility;
    vec3 color = vColor * (vec3(ambient) + (1.0 - ambient) * directLight);

    fragColor = vec4(color, 1.0);
}
