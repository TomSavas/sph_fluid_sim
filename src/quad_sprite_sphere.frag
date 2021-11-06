#version 330 core

in vec4 FragPos;
in float Density;
in vec2 Uv;
in vec4 EyeSpacePos;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 WorldPos;

uniform vec3 cameraPos;

uniform mat4 projection;
uniform mat4 view;

uniform float pressure;
uniform vec3 force;

uniform float size;
uniform vec3 lightDir;

void main()
{
    float distFromCentreSq = dot(Uv, Uv);
    if (distFromCentreSq > size)
    {
        discard;
        return;
    }

    vec3 normal = normalize(vec3(Uv, sqrt(1.f - distFromCentreSq)));
    vec4 pixelPos = EyeSpacePos + vec4(normal, 0.f) * size;
    vec4 clipSpacePos = projection * pixelPos;
    float z = clipSpacePos.z / clipSpacePos.w;
    gl_FragDepth = z;

    vec4 worldNormal = inverse(view) * vec4(normal, 0.f);
    float intensity = max(0.f, dot(vec3(worldNormal), lightDir)) + 0.3f;

    FragColor = vec4(vec3(Density / 4.f, 0.4f, 0.4f) * intensity, 1.f);
    WorldPos = inverse(view) * vec4(pixelPos.xyz, 1.f);
}
