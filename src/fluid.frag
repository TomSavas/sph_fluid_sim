#version 330 core

in vec2 uv;
out vec4 FragColor;

uniform sampler2D thicknessTex;
uniform sampler2D normalTex;
uniform sampler2D worldPosTex;

uniform vec3 lightDir;
uniform vec3 cameraPos;

void main()
{
    float thickness = texture(thicknessTex, uv).r;
    if (thickness <= 0.f)
    {
        discard;
        return;
    }
    vec3 normal = texture(normalTex, uv).rgb;
    vec3 worldPos = texture(worldPosTex, uv).rgb;

    const vec3 color = vec3(0.f / 255.f, 30.f / 255.f, 80.f / 255.f);

    float intensity = max(dot(normal, lightDir), 0.f);
    vec3 ambient = color * 0.6f;
    vec3 diffuse = color * intensity * 0.8f;

    vec3 cameraToFrag = normalize(worldPos - cameraPos);
    float specularIntensity = pow(max(dot(cameraToFrag, reflect(lightDir, normal)), 0.f), 16.f);
    vec3 specular = vec3(1.f, 1.f, 1.f) * specularIntensity;

    FragColor = vec4(ambient + diffuse + specular, thickness);
}

