#version 430 core

in vec2 vUv;

layout(binding = 0) uniform sampler2D uScene;
layout(binding = 1) uniform sampler2D uDepth;

uniform float uExposure;

uniform bool uShowTransmittanceLut;
uniform bool uShowSkyViewLut;
uniform bool uShowDistance;
uniform mat4 uInverseProjection;
uniform float uDistanceMax;

out vec4 fragColor;

vec3 linearToSrgb(vec3 color)
{
    return mix(
        1.055 * pow(color, vec3(1.0 / 2.4)) - 0.055,
        12.92 * color,
        lessThanEqual(color, vec3(0.0031308))
    );
}

void main()
{
    if (uShowDistance)
    {
        float depth = texture(uDepth, vUv).r;

        if (depth >= 1.0)
        {
            fragColor = vec4(0.0, 0.0, 0.0, 1.0);
            return;
        }

        vec3 ndc = vec3(vUv, depth) * 2.0 - 1.0;
        vec4 cameraPosition = uInverseProjection * vec4(ndc, 1.0);
        cameraPosition /= cameraPosition.w;

        float distanceToCamera = length(cameraPosition.xyz);
        float gray = clamp(distanceToCamera / uDistanceMax, 0.0, 1.0);

        fragColor = vec4(vec3(gray), 1.0);
        return;
    }

    vec2 uv = vUv;

    if (uShowSkyViewLut)
    {
        uv.y = 1.0 - uv.y;
    }

    vec3 color = max(texture(uScene, uv).rgb, vec3(0.0));

    if (!uShowTransmittanceLut)
    {
        vec3 exposed = color * uExposure;
        color = vec3(1.0) - exp(-exposed);
    }

    fragColor = vec4(linearToSrgb(color), 1.0);
}
