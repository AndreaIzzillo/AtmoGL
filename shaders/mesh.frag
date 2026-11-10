#version 430 core

in vec3 vWorldNormal;
in vec3 vColor;

out vec4 fragColor;

void main()
{
    vec3 lightDirection = normalize(vec3(-1.0, 1.0, 0.5));
    float diffuse = max(dot(normalize(vWorldNormal), lightDirection), 0.0);
    fragColor = vec4(vColor * (0.2 + 0.8 * diffuse), 1.0);
}
