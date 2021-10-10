#pragma once

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <functional>
#include <string.h>

#include <GL/glew.h>

#include "glm/glm.hpp"

class Shader
{
public:
    unsigned int shaderProgramId;
    bool compilationSucceeded;
    char compilationErrorMsg[1024];

    // Yes mixing and matching is a pain in the ass, but so is calculating a hash of c style string
    std::unordered_set<std::string> boundUniforms;
    const char* name;

    Shader();
    Shader(const char *compFilepath);
    Shader(const char *vsFilepath, const char *fsFilepath);
    Shader(const char *vsFilepath, const char *gsFilepath, const char *fsFilepath);
    ~Shader();

    void Use();

    GLint GetUniformLocation(const char* name);
    void SetUniform(const char *name, bool data);
    void SetUniform(const char *name, int data);
    void SetUniform(const char *name, float data);
    void SetUniform(const char *name, glm::vec2 data);
    void SetUniform(const char *name, glm::vec3 data);
    void SetUniform(const char *name, glm::vec4 data);
    void SetUniform(const char *name, glm::mat2 data);
    void SetUniform(const char *name, glm::mat3 data);
    void SetUniform(const char *name, glm::mat4 data);

    void ReportUnboundUniforms();

private:
    bool CheckCompileErrors(unsigned int shaderId, const char *shaderType);
};
