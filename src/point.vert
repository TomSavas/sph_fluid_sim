#version 330 core
//layout (location = 0) in vec3 pos;
layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 velocity;
layout (location = 2) in vec4 props;
layout (location = 3) in vec4 force;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
//uniform float scale;

out vec4 FragPos;
out float Density;

void main()
{
    Density = props.x;

    FragPos = projection * view * model * vec4(pos.xyz, 1.f);
    //FragPos = vec4(vec3(-.5f, 0.f, 0.f), 1.f);
    gl_Position = FragPos;
    gl_PointSize = 10.f;
}
