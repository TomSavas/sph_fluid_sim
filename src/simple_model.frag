#version 330 core

in vec4 FragPos;

out vec4 FragColor;

uniform vec3 cameraPos;

uniform float pressure;
uniform float density;
uniform vec3 force;

void main()
{
    //FragColor = vec4(1.f, 0.f, 0.f, 1.f);
    //FragColor = vec4((FragPos.xyz + 1.f) / 2.f, 1.f);

    vec3 dFdxPos = dFdx(FragPos.xyz);
    vec3 dFdyPos = dFdy(FragPos.xyz);
    vec3 faceNormal = normalize(cross(dFdxPos,dFdyPos));
    //FragColor = vec4(faceNormal*0.5 + 0.5,1.0);

    vec3 fromCamera = normalize(FragPos.xyz - cameraPos);
    float intensity = dot(faceNormal, fromCamera);
    intensity = max(0.f, intensity);
    intensity = min(1.f, intensity + 0.1f);

    FragColor = vec4(density / 2.f, 0.4f, 0.4f, 1.f) * intensity;
    //FragColor = vec4(vec3(0.3f) + force/100.f, 1.f) * intensity;
}
