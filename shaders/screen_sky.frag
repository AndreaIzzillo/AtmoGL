#version 430 core

#include "atmosphere_common.glsl"

in vec2 vUv;

layout(binding = 0) uniform sampler2D uSkyViewLut;

uniform mat4 uInverseProjection;
uniform mat4 uInverseView;
uniform vec3 uCameraPosition;

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

    fragColor = vec4(texture(uSkyViewLut, uv).rgb, 1.0);
}