#version 430 core

in vec2 vUv;

layout(binding = 0) uniform sampler2D uScene;

out vec4 fragColor;

void main()
{
    fragColor = texture(uScene, vUv);
}
