#version 330 core

in vec2 uv;
out vec4 FragColor;

uniform sampler2D tex;
uniform bool horizontal;

float linearizeDepth(float depth)
{
    const float near = 1.f;
    const float far = 200.f;

    depth = 2.f * depth - 1.f;
    return (2.f * near) / (far + near - depth * (far - near));
}

// Courtesy of mrharicot at shadertoy: https://www.shadertoy.com/view/4dfGDH#
#define SIGMA 100.0
#define BSIGMA 0.1
#define SIZE 50

float normpdf(in float x, in float sigma)
{
        return 0.39894*exp(-0.5*x*x/(sigma*sigma))/sigma;
}

float normpdf3(in vec3 v, in float sigma)
{
        return 0.39894*exp(-0.5*dot(v,v)/(sigma*sigma))/sigma;
}

void main()
{
    vec3 initialColor = texture(tex, uv).rgb;
    if (initialColor.x >= 1.f)
    {
        discard;
        return;
    }

    vec3 c = vec3(linearizeDepth(texture(tex, uv).r));
    vec2 texelSize = 1.f / textureSize(tex, 0);

    const int size = (SIZE-1) / 2;
    float kernel[SIZE];
    vec3 color = vec3(0.f);

    float z = 0.0;
    for (int j = 0; j <= size; ++j)
    {
        float norm = normpdf(float(j), SIGMA);
        kernel[size+j] = norm;
        kernel[size-j] = norm;
    }

    vec3 cc;
    float factor;
    float bz = 1.0/normpdf(0.0, BSIGMA);
    for (int i=-size; i <= size; ++i)
    {
        for (int j=-size; j <= size; ++j)
        {
            vec2 offset = (vec2(float(i), float(j)) + size) * 0.5f * texelSize;
            cc = vec3(linearizeDepth(texture(tex, uv + offset).r));
            factor = normpdf3(cc-c, BSIGMA)*bz*kernel[size+j]*kernel[size+i];
            z += factor;
            color += factor*cc;

        }
    }

    color = color / z;

    FragColor = vec4(vec3(color) * 1.f, 1.0);
    return;
}

