#version 430 core

uniform vec3 uSunDirection;

in vec3 vWorldNormal;
in vec3 vColor;

out vec4 fragColor;

void main()
{
    float diffuse = max(
        dot(normalize(vWorldNormal), uSunDirection),
        0.0
    );

    vec3 color = vColor * (0.1 + 0.9 * diffuse);

    fragColor = vec4(color, 1.0);
}
