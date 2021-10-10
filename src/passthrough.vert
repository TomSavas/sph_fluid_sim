#version 330 core

layout (location = 0) in vec4 pos;
//layout (location = 1) in vec4 vel;
//layout (location = 2) in vec4 accel;
//layout (location = 3) in vec4 penis;

uniform mat4 projection;
uniform mat4 view;

out vec4 FragPos;

void main()
{
    FragPos =  projection * view * vec4(pos.xyz, 1.f);
    //FragPos = vec4(vec3(-.5f, 0.f, 0.f), 1.f);
    gl_Position = FragPos;
    gl_PointSize = 4.f;
}
