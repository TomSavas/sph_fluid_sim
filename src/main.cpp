#include <stdio.h>
#include <vector>
#include <cstdlib>
#include <math.h>
#include <signal.h>

#define GLX_GLXEXT_PROTOTYPES
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include "GLFW/glfw3.h"

#include "imgui_wrapper.h"
#include "shader.h"
#include "camera.h"

#include "glm/glm.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

static bool breakGlError = false;
void GLAPIENTRY
MessageCallback(GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar* message,
        const void* userParam)
{
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, \n\tmessage = %s\n",
            ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
    if (breakGlError)
        raise(SIGTRAP);
}

void ShowFPS(Camera& camera)
{
    static int frameCount = 0;

    static bool open = true;
    const float PAD = 10.0f;

    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
    ImVec2 work_size = viewport->WorkSize;
    ImVec2 window_pos, window_pos_pivot;
    window_pos.x = work_pos.x + work_size.x - PAD;
    window_pos.y = work_pos.y + PAD;
    window_pos_pivot.x = 1.0f;
    window_pos_pivot.y = 0.0f;
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    window_flags |= ImGuiWindowFlags_NoMove;

    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    if (ImGui::Begin("Example: Simple overlay", &open, window_flags))
    {
        ImGui::Text("Particles");
        ImGui::SameLine();
#ifdef DEBUG
        ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "DEBUG");
#else
        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "RELEASE");
#endif
        ImGui::Separator();

        static const unsigned char *vendor = glGetString(GL_VENDOR);
        static const unsigned char *renderer = glGetString(GL_RENDERER);
        static const unsigned char *version = glGetString(GL_VERSION);
        ImGui::Text("%s\n%s\n%s", vendor, renderer, version);

        ImGui::Separator();
        ImGui::Text("Frametime: %.2fms", io.DeltaTime * 1000);
        ImGui::Text("FPS: %.1f", io.Framerate);
        ImGui::Separator();

        ImGui::Text("Camera position: %.4f, %.4f, %.4f", camera.transform.pos.x, camera.transform.pos.y, camera.transform.pos.z);
    }
    ImGui::End();
}

#define PI 3.14159265358979323846f
#define DEG2RAD (PI / 180.f)
std::vector<glm::vec3> SphereModel(int rings, int slices)
{
    std::vector<glm::vec3> verts;

    for (int i = 0; i < (rings + 2); i++)
    {
        for (int j = 0; j < slices; j++)
        {
            verts.push_back(glm::vec3(cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*i))*sinf(DEG2RAD*(360.0f*j/slices)),
                    sinf(DEG2RAD*(270 + (180.0f/(rings + 1))*i)),
                    cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*i))*cosf(DEG2RAD*(360.0f*j/slices))));
            verts.push_back(glm::vec3(cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1)))*sinf(DEG2RAD*(360.0f*(j + 1)/slices)),
                    sinf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1))),
                    cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1)))*cosf(DEG2RAD*(360.0f*(j + 1)/slices))));
            verts.push_back(glm::vec3(cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1)))*sinf(DEG2RAD*(360.0f*j/slices)),
                    sinf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1))),
                    cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1)))*cosf(DEG2RAD*(360.0f*j/slices))));

            verts.push_back(glm::vec3(cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*i))*sinf(DEG2RAD*(360.0f*j/slices)),
                    sinf(DEG2RAD*(270 + (180.0f/(rings + 1))*i)),
                    cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*i))*cosf(DEG2RAD*(360.0f*j/slices))));
            verts.push_back(glm::vec3(cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i)))*sinf(DEG2RAD*(360.0f*(j + 1)/slices)),
                    sinf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i))),
                    cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i)))*cosf(DEG2RAD*(360.0f*(j + 1)/slices))));
            verts.push_back(glm::vec3(cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1)))*sinf(DEG2RAD*(360.0f*(j + 1)/slices)),
                    sinf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1))),
                    cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1)))*cosf(DEG2RAD*(360.0f*(j + 1)/slices))));
        }
    }

    return verts;
}

#define PARTICLE_COUNT 1024*3
#define MAX_PARTICLE_COUNT 4096*2
struct ParticleData
{
    glm::vec4 posAndMass;
    glm::vec4 vel;
    glm::vec4 props;
    glm::vec4 force;
}; 

void runSimulationCompute(GLFWwindow* window, Shader& densityCompShader, Shader& forceCompShader, Shader& integrationCompShader, int particleCount, std::vector<ParticleData>& particleData, unsigned int particleDataBufId, glm::vec3 gravity, glm::vec3 box)
{
        static const float smoothingRadius = 1.0f;
        static const float poly6Const = 315.f / (64.f * PI * pow(smoothingRadius, 9.f));

        densityCompShader.Use();
        densityCompShader.SetUniform("particleCount", particleCount);
        densityCompShader.SetUniform("smoothingRadius", smoothingRadius);
        densityCompShader.SetUniform("densityReference", 1.f);
        densityCompShader.SetUniform("pressureConst", 250.f);
        densityCompShader.SetUniform("poly6Const", poly6Const);
        glDispatchCompute(particleCount, 1, 1);
        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

        static const float spikyConst = -45.f / (PI * pow(smoothingRadius, 6));

        forceCompShader.Use();
        forceCompShader.SetUniform("particleCount", particleCount);
        forceCompShader.SetUniform("smoothingRadius", smoothingRadius);
        forceCompShader.SetUniform("viscosityConst", 0.188f);
        forceCompShader.SetUniform("spikyConst", spikyConst);
        forceCompShader.SetUniform("laplacianConst", -spikyConst);
        forceCompShader.SetUniform("gravity", gravity);
        glDispatchCompute(particleCount, 1, 1);
        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

        integrationCompShader.Use();
        integrationCompShader.SetUniform("timestep", 0.016f);
        integrationCompShader.SetUniform("gravity", gravity);
        integrationCompShader.SetUniform("box", box);
        glDispatchCompute(particleCount, 1, 1);
        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

        glGetNamedBufferSubData(particleDataBufId, 0, sizeof(ParticleData) * MAX_PARTICLE_COUNT, particleData.data());
}

int main(void) 
{
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    GLFWwindow *window = glfwCreateWindow(1920, 1080, "Ignoramus", NULL, NULL);
    if (!window) 
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed initializing glew\n");
        glfwTerminate();
        return -1;
    }

    glEnable (GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);

    //glfwSwapInterval(0.f);

    ImGuiWrapper::Init(window);
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad;

    float cubeVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };

    const char* skyboxImages[] = 
    { 
        "../assets/skybox/right.jpg",
        "../assets/skybox/left.jpg",
        "../assets/skybox/top.jpg",
        "../assets/skybox/bottom.jpg",
        "../assets/skybox/front.jpg",
        "../assets/skybox/back.jpg"
    };

    unsigned int skyboxCubemapId;
    glGenTextures(1, &skyboxCubemapId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemapId);
    for (int i = 0; i < 6; i++)
    {
        int width, height, n;
        unsigned char* img = stbi_load(skyboxImages[i], &width, &height, &n, 3);
        if (!img)
        {
            printf("lmao\n");
            return 0; // For the time being just die
        }

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
        stbi_image_free(img);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);  

    unsigned int cubeVao, cubeVbo;
    glGenVertexArrays(1, &cubeVao);
    glGenBuffers(1, &cubeVbo);
    glBindVertexArray(cubeVao);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    int workGroupCount[3];
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &workGroupCount[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &workGroupCount[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &workGroupCount[2]);
    printf("max global (total) work group counts x:%i y:%i z:%i\n",
              workGroupCount[0], workGroupCount[1], workGroupCount[2]);

    int workGroupSize[3];
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &workGroupSize[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &workGroupSize[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &workGroupSize[2]);
    printf("max global (in one shader) work group sizes x:%i y:%i z:%i\n",
              workGroupSize[0], workGroupSize[1], workGroupSize[2]);

    int workGroupInvocations;
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &workGroupInvocations);
    printf("max local work group invocations %i\n", workGroupInvocations);

    printf("sizeof(glm::vec4) = %d\n", sizeof(glm::vec4));
    printf("sizeof(ParticleData) = %d\n", sizeof(ParticleData));
    std::vector<ParticleData> particleData(MAX_PARTICLE_COUNT);
    printf("particleData.size() = %d\n", particleData.size());
    printf("sizeof(ParticleData) * particleData.size() = %d\n", sizeof(ParticleData) * particleData.size());

    int particleCount = PARTICLE_COUNT;
    glm::vec3 box(35.f, 100.f, 25.f);
    for (int i = 0; i < MAX_PARTICLE_COUNT; i++)
    {
        if (i < PARTICLE_COUNT)
        {
            particleData[i].posAndMass = glm::vec4(
                    (float)(std::rand() % 1000) / 1000.f * box.x,
                    (float)(std::rand() % 1000) / 1000.f * (box.y / 50.f) + box.y / 5.f,
                    (float)(std::rand() % 1000) / 1000.f * box.z,
                    1.f);
            particleData[i].vel = glm::vec4(0.f);
        }
        else
        {
            particleData[i].posAndMass = glm::vec4(0.5f,
                    (float)(std::rand() % 1000) / 1000.f * 5.f + 30.f,
                    (float)(std::rand() % 1000) / 1000.f * 5.f + (box.z / 2),
                    1.f);
            particleData[i].vel = glm::vec4(20.f, -15.f, 0.f, 0.f);
        }

        particleData[i].props = glm::vec4(0.f, 0.f, 0.f, 0.f); 
        particleData[i].force = glm::vec4(0.f, 0.f, 0.f, 0.f); 
    }

    unsigned int particleDataId;
    glGenBuffers(1, &particleDataId);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleDataId);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ParticleData) * MAX_PARTICLE_COUNT, particleData.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particleDataId);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    Shader compShader("../src/fluid_sim.comp");
    Shader densityCompShader("../src/sph_density.comp");
    Shader forceCompShader("../src/sph_force.comp");
    Shader integrationCompShader("../src/sph_integration.comp");
    Shader passthroughShader("../src/passthrough.vert", "../src/passthrough.frag");
    Shader simpleModelShader("../src/simple_model.vert", "../src/simple_model.frag");
    Shader skyboxShader("../src/skybox.vert", "../src/skybox.frag");

    breakGlError = true;

    glClearColor(0.0f, 0.0f, 0.0f, 1.f);
    bool showDemoWindow = true;
    bool initialStart = true;

    std::vector<glm::vec3> sphere = SphereModel(16, 16);
    unsigned int sphereVao;
    glGenVertexArrays(1, &sphereVao);
    glBindVertexArray(sphereVao);

    unsigned int sphereVbo;
    glGenBuffers(1, &sphereVbo);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * sphere.size(), sphere.data(), GL_DYNAMIC_DRAW);

    glEnable(GL_DEPTH_TEST);

    Camera camera;
    camera.transform.pos = glm::vec3(15, 30, -90);

    while (!glfwWindowShouldClose(window)) 
    {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.Update(window);
        ImGuiWrapper::PreRender();

        static float gravityScale = 20;
        static bool simulationPaused = true;
        if (!simulationPaused)
            runSimulationCompute(window, densityCompShader, forceCompShader, integrationCompShader, particleCount, particleData, particleDataId, glm::vec3(0.f, -1.f, 0.f) * gravityScale, box);

        bool keyDown = glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS;
        for (int i = 0; i < particleCount; i++)
        {
            if (keyDown)
            {
                printf("x: %f, y: %f, z: %f, mass: %f\n", particleData[i].posAndMass.x, particleData[i].posAndMass.y, particleData[i].posAndMass.z, particleData[i].posAndMass.w);
                printf("\tvel x: %f, y: %f, z: %f\n", particleData[i].vel.x, particleData[i].vel.y, particleData[i].vel.z);
                printf("\tdensity %f, pressure: %f\n", particleData[i].props.x, particleData[i].props.y);
                printf("\tforce : %f, %f, %f\n", particleData[i].force.x, particleData[i].force.y, particleData[i].force.z);
            }
        }
        if (keyDown)
            printf("\n");

        simpleModelShader.Use();
        simpleModelShader.SetUniform("projection", camera.projection);
        simpleModelShader.SetUniform("view", camera.View());
        Transform t;

        glBindVertexArray(sphereVao);
        //for (int i = 1; i < 4; i++)
        //    glDisableVertexAttribArray(i);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

        simpleModelShader.SetUniform("cameraPos", camera.transform.pos);
        for (int i = 0; i < particleData.size(); i++)
        {
            simpleModelShader.SetUniform("density", particleData[i].props.x / 2);
            simpleModelShader.SetUniform("pressure", particleData[i].props.y /2);
            simpleModelShader.SetUniform("force", glm::vec3(particleData[i].force.x, particleData[i].force.y, particleData[i].force.z));

            t.pos = glm::vec3(particleData[i].posAndMass.x,
                    particleData[i].posAndMass.y,
                    particleData[i].posAndMass.z);
            t.scale = glm::vec3(0.4f);
            simpleModelShader.SetUniform("model", t.Model());
            glDrawArrays(GL_TRIANGLES, 0, sphere.size());
        }

        // Ground
        glBindVertexArray(cubeVao);
        simpleModelShader.SetUniform("density", 0.5f);
        static glm::vec3 groundPos(box.x/ 2.f, -0.6, box.z / 2.f);
        static glm::vec3 groundScale(40.f, 0.2f, 30.f);
        t.pos = groundPos;
        t.scale = groundScale;
        simpleModelShader.SetUniform("model", t.Model());
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Skybox
        static bool skyboxEnabled = true;
        if (skyboxEnabled)
        {
            glDepthFunc(GL_LEQUAL);
            skyboxShader.Use();
            glm::mat4 viewProjection = camera.projection * glm::mat4(glm::mat3(camera.View()));
            skyboxShader.SetUniform("viewProjection", viewProjection);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemapId);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glDepthFunc(GL_LESS);
        }

        if (particleCount < MAX_PARTICLE_COUNT && glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
            particleCount += 2;

        static bool settingsOpen = true;
        if (ImGui::Begin("Settings", &settingsOpen))
        {
            ImGui::Checkbox("Pause simulation", &simulationPaused);
            ImGui::SliderFloat("Gravity", &gravityScale, 0.f, 100.f);
            ImGui::Checkbox("Skybox", &skyboxEnabled);
        }
        ImGui::End();

        ShowFPS(camera);
        ImGuiWrapper::Render();

        glfwSwapBuffers(window);

        if (initialStart)
            initialStart = false;
    }

    ImGuiWrapper::Deinit();
    glfwTerminate();

    return 0;
}
