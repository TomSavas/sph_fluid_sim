#version 330 core

in vec2 uv;
out vec4 FragColor;

uniform sampler2D tex;

float absorption(float value)
{
    // some freedom was taken on interpreting the Beer's law lol
    return pow(value, 3) * 0.8f + 0.3f;
}

// single pass gaussian blur, courtesy of: https://www.shadertoy.com/view/Xltfzj
void main()
{
    float thickness = texture(tex, uv).r;
    if (thickness <= 0.f)
    {
        discard;
        return;
    }

    const float pi = 6.28318530718f;
    const float directions = 16.f;
    const float quality = 3.f;
    const float size = 8.f;

    vec2 radius = size / vec2(1920.f, 1080.f);
    vec4 color = vec4(absorption(thickness));

    for (float d = 0.f; d < pi; d += pi / directions)
    {
        for(float i = 1.f / quality; i <= 1.f; i += 1.f / quality)
        {
            color += vec4(absorption(texture(tex, uv + vec2(cos(d), sin(d)) * radius * i).r));
        }
    }

    color /= quality * directions - 15.0;
    FragColor = color;
}
