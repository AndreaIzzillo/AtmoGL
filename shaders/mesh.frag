#version 430 core

uniform vec3 uSunDirection;
uniform vec3 uSunIlluminance;

in vec3 vWorldNormal;
in vec3 vColor;

out vec4 fragColor;

void main()
{
    const float ambient = 0.05;

    float diffuse = max(dot(normalize(vWorldNormal), uSunDirection), 0.0);
    diffuse = (1.0 - ambient) * diffuse + ambient;

    fragColor = vec4(vColor * uSunIlluminance * diffuse, 1.0);
}
