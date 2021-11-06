#version 330 core

in vec2 uv;
out vec4 FragColor;

uniform mat4 invV;
uniform mat4 invP;
uniform sampler2D tex;

vec3 worldPosFromDepth(float depth, vec2 uv)
{
    vec3 ndcPos = vec3(uv, depth) * 2.f - 1.f;
    vec4 viewSpacePos = invP * vec4(ndcPos, 1.f);
    viewSpacePos /= viewSpacePos.w;
    vec4 worldSpacePos = invV * viewSpacePos;

    return worldSpacePos.xyz;
}

void main()
{
    float depth = texture(tex, uv).r;
    
    if (depth >= 1.f)
    {
        discard;
        return;
    }

    FragColor = vec4(vec3(depth), 1.f);
    vec2 texelSize = vec2(1.f) / textureSize(tex, 0);

    vec3 worldPos = worldPosFromDepth(depth, uv);
    FragColor = vec4(worldPos, 1.f);

    vec2 uvXOffset = vec2(texelSize.x, 0.f);
    float depthLeft = texture(tex, uv + uvXOffset).r;
    vec3 ddxLeft = worldPosFromDepth(depthLeft, uv + uvXOffset) - worldPos;
    float depthRight = texture(tex, uv + uvXOffset).r;
    vec3 ddxRight = worldPos - worldPosFromDepth(depthRight, uv - uvXOffset);
    vec3 ddx = ddxLeft;
    if (abs(ddxRight.z) < (ddxLeft.z))
    {
        ddx = ddxRight;
    }

    vec2 uvYOffset = vec2(0.f, texelSize.y);
    float depthTop = texture(tex, uv + uvYOffset).r;
    vec3 ddyTop = worldPosFromDepth(depthTop, uv + uvYOffset) - worldPos;
    float depthBot = texture(tex, uv - uvYOffset).r;
    vec3 ddyBot = worldPos - worldPosFromDepth(depthBot, uv - uvYOffset);
    vec3 ddy = ddyTop;
    if (abs(ddyBot.z) < (ddyTop.z))
    {
        ddy = ddyBot;
    }

    vec3 n = normalize(cross(ddx, ddy));
    FragColor = vec4(n, 1.f);
}
