#version 430 core

#include "atmosphere_common.glsl"

in vec2 vUv;

layout(binding = 1) uniform sampler2D uSkyViewLut;
layout(binding = 2) uniform sampler3D uAerialPerspectiveLut;
layout(binding = 3) uniform sampler2D uScene;
layout(binding = 4) uniform sampler2D uDepth;
layout(binding = 5) uniform sampler2D uSunView;

uniform float uExposure;

uniform bool uShowTransmittanceLut;
uniform bool uShowSkyViewLut;
uniform bool uShowSunView;
uniform bool uShowAerialPerspectiveLut;
uniform float uAerialPerspectiveDebugDepth;
uniform bool uShowDistance;
uniform mat4 uInverseProjection;
uniform float uDistanceMax;

uniform bool uEnableAerialPerspective;
uniform bool uUseAerialPerspectiveLut;
uniform float uAerialPerspectiveMaxDepth;
uniform mat4 uInverseView;
uniform vec3 uCameraPosition;

out vec4 fragColor;

vec3 linearToSrgb(vec3 color)
{
    return mix(
        1.055 * pow(color, vec3(1.0 / 2.4)) - 0.055,
        12.92 * color,
        lessThanEqual(color, vec3(0.0031308))
    );
}

vec3 reconstructCameraPosition(vec2 uv, float depth)
{
    vec3 ndc = vec3(uv, depth) * 2.0 - 1.0;
    vec4 position = uInverseProjection * vec4(ndc, 1.0);

    return position.xyz / position.w;
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

        vec3 cameraPosition = reconstructCameraPosition(vUv, depth);
        float distanceToCamera = length(cameraPosition);
        float gray = clamp(distanceToCamera / uDistanceMax, 0.0, 1.0);

        fragColor = vec4(vec3(gray), 1.0);
        return;
    }

    vec3 color;
    if (uShowTransmittanceLut)
    {
        color = texture(uTransmittanceLut, vUv).rgb;
    }
    else if (uShowSkyViewLut)
    {
        color = texture(uSkyViewLut, vec2(vUv.x, 1.0 - vUv.y)).rgb;
    }
    else if (uShowAerialPerspectiveLut)
    {
        ivec3 lutSize = textureSize(uAerialPerspectiveLut, 0);
        float depthCoordinate = clamp(
            uAerialPerspectiveDebugDepth / uAerialPerspectiveMaxDepth,
            0.0,
            1.0
        );
        float w = (0.5 + depthCoordinate * float(lutSize.z - 1)) / float(lutSize.z);

        vec4 aerialPerspective = textureLod(
            uAerialPerspectiveLut,
            vec3(vUv, w),
            0.0
        );

        fragColor = vec4(vec3(aerialPerspective.a), 1.0);
        // fragColor = vec4(aerialPerspective.rgb, 1.0);

        return;
    }
    else if (uShowSunView)
    {
        color = texture(uSunView, vUv).rgb;
    }
    else
    {
        color = texture(uScene, vUv).rgb;

        if (uEnableAerialPerspective)
        {
            float depth = texture(uDepth, vUv).r;

            if (depth < 1.0)
            {
                vec3 cameraPosition = reconstructCameraPosition(vUv, depth);
                if (uUseAerialPerspectiveLut)
                {
                    ivec3 lutSize = textureSize(uAerialPerspectiveLut, 0);
                    float depthCoordinate = clamp(
                        -cameraPosition.z / uAerialPerspectiveMaxDepth,
                        0.0,
                        1.0
                    );
                    float w = (0.5 + depthCoordinate * float(lutSize.z - 1))
                              / float(lutSize.z);

                    vec4 aerialPerspective = textureLod(
                        uAerialPerspectiveLut,
                        vec3(vUv, w),
                        0.0
                    );

                    color = aerialPerspective.a * color + aerialPerspective.rgb;
                }
                else
                {
                    float distanceToTerrain = length(cameraPosition);
                    vec3 worldDirection = normalize(mat3(uInverseView) * cameraPosition);

                    AtmosphereResult result = integrateAtmosphere(
                        uCameraPosition,
                        worldDirection,
                        distanceToTerrain,
                        true
                    );

                    color = result.transmittance * color + result.radiance;
                }
            }
        }
    }

    color = max(color, vec3(0.0));

    if (!uShowTransmittanceLut)
    {
        vec3 exposed = color * uExposure;
        color = vec3(1.0) - exp(-exposed);
    }

    fragColor = vec4(linearToSrgb(color), 1.0);
}
