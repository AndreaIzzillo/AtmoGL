#ifndef COMMON_GLSL
#define COMMON_GLSL

layout(binding = 6) uniform sampler2D uSunDepth;

uniform mat4 uLightViewProjection;
uniform float uShadowBias;

float computeSunVisibility(vec3 worldPosition)
{
    vec4 lightPosition = uLightViewProjection * vec4(worldPosition, 1.0);
    vec3 shadowPosition = lightPosition.xyz / lightPosition.w;
    shadowPosition = shadowPosition * 0.5 + 0.5;

    if (any(lessThan(shadowPosition, vec3(0.0)))
        || any(greaterThan(shadowPosition, vec3(1.0))))
    {
        return 1.0;
    }

    vec2 texelSize = 1.0 / vec2(textureSize(uSunDepth, 0));
    float fragmentDepth = shadowPosition.z - uShadowBias;
    float visibility = 0.0;

    const int kernelSize = 1;
    for (int y = -kernelSize; y <= kernelSize; ++y)
    {
        for (int x = -kernelSize; x <= kernelSize; ++x)
        {
            vec2 uv = shadowPosition.xy + vec2(x, y) * texelSize;
            if (any(lessThan(uv, vec2(0.0))) || any(greaterThan(uv, vec2(1.0))))
            {
                visibility += 1.0;
                continue;
            }

            float closestDepth = textureLod(uSunDepth, uv, 0.0).r;
            visibility += fragmentDepth <= closestDepth ? 1.0 : 0.0;
        }
    }

    return visibility / float((kernelSize * 2 + 1) * (kernelSize * 2 + 1));
}

#endif
