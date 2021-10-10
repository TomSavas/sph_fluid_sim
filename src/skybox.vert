#version 330
layout (location = 0) in vec3 pos;

out vec3 TexPos;

uniform mat4 viewProjection;

void main()
{
    TexPos = pos;
    vec4 position = viewProjection * vec4(pos, 1.f);
    gl_Position = position.xyww; //?
}
