#version 330 core

layout (location = 0) in vec3 pos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform float density;

out vec4 FragPos;
out float Density;

void main()
{
    Density = density;

    FragPos = projection * view * model * vec4(pos.xyz, 1.f);
    gl_Position = FragPos;
}
