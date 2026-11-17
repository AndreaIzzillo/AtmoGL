#version 430 core

in vec2 vUv;

layout(binding = 0) uniform sampler2D uScene;

uniform float uExposure;
uniform bool uShowTransmittanceLut;

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
    vec3 color = max(texture(uScene, vUv).rgb, vec3(0.0));

    if (!uShowTransmittanceLut)
    {
        vec3 exposed = color * uExposure;
        color = vec3(1.0) - exp(-exposed);
    }

    fragColor = vec4(linearToSrgb(color), 1.0);
}