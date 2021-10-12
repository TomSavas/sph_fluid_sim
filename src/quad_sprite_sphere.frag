#version 330 core

in vec4 FragPos;
in float Density;
in vec2 Uv;
in vec4 EyeSpacePos;

out vec4 FragColor;

uniform vec3 cameraPos;

uniform mat4 projection;
uniform mat4 view;

uniform float pressure;
uniform vec3 force;

void main()
{
    float distFromCentreSq = dot(Uv, Uv);
    if (distFromCentreSq > 0.4f)
    {
        discard;
        return;
    }

    vec3 normal = normalize(vec3(Uv, sqrt(1.f - distFromCentreSq)));
    vec4 pixelPos = EyeSpacePos + vec4(normal, 0.f) * 0.4f;
    vec4 clipSpacePos = projection * pixelPos;
    float z = clipSpacePos.z / clipSpacePos.w;
    const float near = 0.1f;
    const float far = 100000.f;
    float depth = z;
    // In practice, this would be better than just doing linear depth.
    //float depth = ((1.f / z) - (1.f / near)) / ((1.f / far) - (1.f / near));
    gl_FragDepth = depth;

    const vec3 lightDir = vec3(-1.f, 1.f, -1.f);
    float intensity = max(0.f, dot(normal, -lightDir)) + 0.3f;
    FragColor = vec4(Density / 4.f, 0.4f, 0.4f, 1.f) * intensity;
}
