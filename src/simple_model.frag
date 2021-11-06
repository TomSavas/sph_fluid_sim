#version 330 core

in vec4 FragPos;
in float Density;

out vec4 FragColor;

uniform vec3 cameraPos;

uniform float pressure;
uniform vec3 force;

uniform bool useDensityForColor;
uniform vec3 color;

void main()
{
    vec3 dFdxPos = dFdx(FragPos.xyz);
    vec3 dFdyPos = dFdy(FragPos.xyz);
    vec3 screenSpaceFaceNormal = normalize(cross(dFdxPos,dFdyPos));

    vec3 fromCamera = normalize(FragPos.xyz - cameraPos);
    float intensity = abs(dot(screenSpaceFaceNormal, fromCamera));
    intensity = max(0.f, intensity);
    intensity = min(1.f, intensity + 0.1f);

    if (useDensityForColor)
    {
        FragColor = vec4(Density / 4.f, 0.4f, 0.4f, 1.f) * intensity;
    }
    else
    {
        FragColor = vec4(color, 1.f);
    }
}
