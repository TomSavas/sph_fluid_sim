#version 330 core

in vec4 FragPos;
in vec2 Uv;

out vec4 FragColor;

uniform float size;

void main()
{
    float distFromCentreSq = dot(Uv, Uv);
    if (distFromCentreSq > size)
    {
        discard;
        return;
    }

    FragColor = vec4(vec3(1.f), 0.05f);
}
