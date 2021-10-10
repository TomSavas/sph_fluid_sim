#version 330 core

in vec4 FragPos;

out vec4 FragColor;

void main()
{
    FragColor = vec4(1.f, 0.f, 0.f, 1.f);
    //FragColor = vec4((FragPos.xyz + 1.f) / 2.f, 1.f);
}
