#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 particlePos;
layout (location = 2) in vec4 velocity;
layout (location = 3) in vec4 props;
layout (location = 4) in vec4 force;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
//uniform float scale;

out vec4 FragPos;
out float Density;

void main()
{
    //mat4 model = mat4(scale);
    //model[3][3] = 1;

    mat4 localModel = model;
    localModel[3][0] = particlePos.x;
    localModel[3][1] = particlePos.y;
    localModel[3][2] = particlePos.z;

    Density = props.x;

    FragPos = projection * view * localModel * vec4(pos.xyz, 1.f);
    //FragPos = vec4(vec3(-.5f, 0.f, 0.f), 1.f);
    gl_Position = FragPos;
}
