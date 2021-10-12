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

void ShowInfo(Camera& camera, int particleCount)
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
        ImGui::Text("SPH fluid sim");
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
        ImGui::Separator();

        ImGui::Text("Particle count: %d", particleCount);
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

std::vector<glm::vec3> QuadModel()
{
    return std::vector<glm::vec3>
    {
        //glm::vec3(-1.f, -1.f,  0.f), glm::vec3( 1.f,  1.f,  0.f), glm::vec3(-1.f,  1.f,  0.f), 
        //glm::vec3( 1.f,  1.f,  0.f), glm::vec3(-1.f, -1.f,  0.f), glm::vec3( 1.f, -1.f,  0.f), 
        glm::vec3(-1.f, -1.f,  0.f), glm::vec3(-1.f,  1.f,  0.f), glm::vec3( 1.f,  1.f,  0.f), 
        glm::vec3( 1.f,  1.f,  0.f), glm::vec3( 1.f, -1.f,  0.f), glm::vec3(-1.f, -1.f,  0.f), 
    };
}

#define PARTICLE_COUNT 1024*10
#define MAX_PARTICLE_COUNT 4096*32
struct ParticleData
{
    glm::vec4 posAndMass;
    glm::vec4 vel;
    glm::vec4 props;
    glm::vec4 force;
}; 

void runSimulationCompute(GLFWwindow* window, Shader& densityCompShader, Shader& forceCompShader, Shader& integrationCompShader, int particleCount,
        std::vector<ParticleData>& particleData, unsigned int particleDataBufId, glm::vec3 gravity, glm::vec3 box,
        glm::vec3 attractorPos, float attractorInnerRadius, float attractorOutterRadius, float attractorStrength)
{
        static const float smoothingRadius = 1.0f;
        static const float poly6Const = 315.f / (64.f * PI * pow(smoothingRadius, 9.f));

        densityCompShader.Use();
        densityCompShader.SetUniform("particleCount", particleCount);
        densityCompShader.SetUniform("smoothingRadius", smoothingRadius);
        densityCompShader.SetUniform("densityReference", 1.f);
        densityCompShader.SetUniform("pressureConst", 250.f);
        densityCompShader.SetUniform("poly6Const", poly6Const);
        glDispatchCompute(particleCount / 256.f, 1, 1);
        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

        static const float spikyConst = -45.f / (PI * pow(smoothingRadius, 6));

        forceCompShader.Use();
        forceCompShader.SetUniform("particleCount", particleCount);
        forceCompShader.SetUniform("smoothingRadius", smoothingRadius);
        forceCompShader.SetUniform("viscosityConst", 0.188f);
        forceCompShader.SetUniform("spikyConst", spikyConst);
        forceCompShader.SetUniform("laplacianConst", -spikyConst);
        forceCompShader.SetUniform("gravity", gravity);
        glDispatchCompute(particleCount / 256.f, 1, 1);
        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

        integrationCompShader.Use();
        integrationCompShader.SetUniform("timestep", 0.016f);
        integrationCompShader.SetUniform("gravity", gravity);
        integrationCompShader.SetUniform("box", box);
        integrationCompShader.SetUniform("attractorPos", attractorPos);
        integrationCompShader.SetUniform("attractorInnerRadius", attractorInnerRadius);
        integrationCompShader.SetUniform("attractorOutterRadius", attractorOutterRadius);
        integrationCompShader.SetUniform("attractorStrength", attractorStrength);
        glDispatchCompute(particleCount / 256.f, 1, 1);
        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
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
    bool skyboxLoaded = true;
    for (int i = 0; i < 6 && skyboxLoaded; i++)
    {
        int width, height, n;
        unsigned char* img = stbi_load(skyboxImages[i], &width, &height, &n, 3);
        if (!img)
        {
            printf("Failed loading image %s", skyboxImages[i]);
            skyboxLoaded = false;
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
    glm::vec3 box(35.f * 1.5, 100.f * 1.5, 15.f * 1.5);
    for (int i = 0; i < MAX_PARTICLE_COUNT; i++)
    {
        if (i < PARTICLE_COUNT)
        {
            particleData[i].posAndMass = glm::vec4(
                    (float)(std::rand() % 1000) / 1000.f * box.x,
                    (float)(std::rand() % 1000) / 1000.f * (box.y / 10.f) + box.y / 5.f,
                    (float)(std::rand() % 1000) / 1000.f * box.z,
                    1.f);
            particleData[i].vel = glm::vec4(0.f);
        }
        else
        {
            particleData[i].posAndMass = glm::vec4(
                    (float)(std::rand() % 1000) / 1000.f * box.x / 10.f + 0.1f,
                    (float)(std::rand() % 1000) / 1000.f * 5.f + (box.y / 8),
                    (float)(std::rand() % 1000) / 1000.f * 5.f + (box.z / 2),
                    1.f);
            particleData[i].vel = glm::vec4(40.f, -15.f, 0.f, 0.f);
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
    Shader simpleInstancedModelShader("../src/instanced_simple_model.vert", "../src/simple_model.frag");
    Shader skyboxShader("../src/skybox.vert", "../src/skybox.frag");
    Shader pointShader("../src/point.vert", "../src/point.frag");
    Shader quadSpriteSphereShader("../src/quad_sprite_sphere.vert", "../src/quad_sprite_sphere.frag");

    breakGlError = true;

    glClearColor(0.0f, 0.0f, 0.0f, 1.f);
    bool showDemoWindow = true;
    bool initialStart = true;

    std::vector<glm::vec3> sphere = SphereModel(16, 16);
    unsigned int sphereVao;
    glGenVertexArrays(1, &sphereVao);
    glBindVertexArray(sphereVao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    unsigned int sphereVbo;
    glGenBuffers(1, &sphereVbo);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * sphere.size(), sphere.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glBindBuffer(GL_ARRAY_BUFFER, particleDataId);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), NULL);
    glVertexAttribDivisor(1, 1);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void*)sizeof(glm::vec4));
    glVertexAttribDivisor(2, 1);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void*)(sizeof(glm::vec4) * 2));
    glVertexAttribDivisor(3, 1);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void*)(sizeof(glm::vec4) * 3));
    glVertexAttribDivisor(4, 1);

    unsigned int pointVao;
    glGenVertexArrays(1, &pointVao);
    glBindVertexArray(pointVao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, particleDataId);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), NULL);
    glVertexAttribDivisor(0, 1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void*)sizeof(glm::vec4));
    glVertexAttribDivisor(1, 1);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void*)(sizeof(glm::vec4) * 2));
    glVertexAttribDivisor(2, 1);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void*)(sizeof(glm::vec4) * 3));
    glVertexAttribDivisor(3, 1);

    std::vector<glm::vec3> quad = QuadModel();
    unsigned int quadVao;
    glGenVertexArrays(1, &quadVao);
    glBindVertexArray(quadVao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    unsigned int quadVbo;
    glGenBuffers(1, &quadVbo);
    glBindBuffer(GL_ARRAY_BUFFER, quadVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * quad.size(), quad.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, particleDataId);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), NULL);
    glVertexAttribDivisor(1, 1);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void*)sizeof(glm::vec4));
    glVertexAttribDivisor(2, 1);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void*)(sizeof(glm::vec4) * 2));
    glVertexAttribDivisor(3, 1);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void*)(sizeof(glm::vec4) * 3));
    glVertexAttribDivisor(4, 1);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_PROGRAM_POINT_SIZE);

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
        static Transform attractorTransform(glm::vec3(box.x / 2.f, box.y / 2.f, box.z / 2.f));
        static float attractorInnerRadius = 10.f;
        static float attractorOutterRadius = 25.f;
        static float attractorStrength = 1000.f;
        static bool attractorEnabled = false;
        if (!simulationPaused)
        {
            runSimulationCompute(window, densityCompShader, forceCompShader, integrationCompShader, particleCount, particleData,
                    particleDataId, glm::vec3(0.f, -1.f, 0.f) * gravityScale, box, attractorTransform.pos, attractorInnerRadius, attractorOutterRadius,
                    attractorStrength * (attractorEnabled ? 1.f : 0.f));
        }

        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        {
            glGetNamedBufferSubData(particleDataId, 0, sizeof(ParticleData) * MAX_PARTICLE_COUNT, particleData.data());

            for (int i = 0; i < particleCount; i++)
            {
                printf("x: %f, y: %f, z: %f, mass: %f\n", particleData[i].posAndMass.x, particleData[i].posAndMass.y, particleData[i].posAndMass.z, particleData[i].posAndMass.w);
                printf("\tvel x: %f, y: %f, z: %f\n", particleData[i].vel.x, particleData[i].vel.y, particleData[i].vel.z);
                printf("\tdensity %f, pressure: %f\n", particleData[i].props.x, particleData[i].props.y);
                printf("\tforce : %f, %f, %f\n", particleData[i].force.x, particleData[i].force.y, particleData[i].force.z);
            }
            printf("\n");
        }

        Transform t;

#define IDENTITY(VALUE) VALUE
#define STRINGIFY(VALUE) #VALUE
#define RENDER_AS_VALUES(F)      \
        F(POINTS),               \
        F(QUADS),                \
        F(QUAD_SPRITE_SPHERES),  \
        F(SPHERES),              \
        F(NONE),                 \
        F(RENDER_AS_COUNT)  

        static const char* renderAsStrings[] = { RENDER_AS_VALUES(STRINGIFY) };
        static enum renderAsEnum
        {
            RENDER_AS_VALUES(IDENTITY)
        } renderAs;
        switch (renderAs)
        {
            case POINTS:
                glBindVertexArray(pointVao);
                t.scale = glm::vec3(1.f);
                pointShader.Use();
                pointShader.SetUniform("projection", camera.projection);
                pointShader.SetUniform("view", camera.View());
                pointShader.SetUniform("cameraPos", camera.transform.pos);
                pointShader.SetUniform("model", t.Model());
                glDrawArraysInstanced(GL_POINTS, 0, 1, particleCount);
                break;
            case QUADS:
                glBindVertexArray(quadVao);
                t.scale = glm::vec3(1.0f);
                t.rot = camera.transform.rot;
                simpleInstancedModelShader.Use();
                simpleInstancedModelShader.SetUniform("projection", camera.projection);
                simpleInstancedModelShader.SetUniform("view", camera.View());
                simpleInstancedModelShader.SetUniform("cameraPos", camera.transform.pos);
                simpleInstancedModelShader.SetUniform("model", t.Model());
                glDrawArraysInstanced(GL_TRIANGLES, 0, quad.size(), particleCount);
                t.rot = glm::quat();
                break;
            case QUAD_SPRITE_SPHERES:
                glBindVertexArray(quadVao);
                t.scale = glm::vec3(1.0f);
                t.rot = camera.transform.rot;
                quadSpriteSphereShader.Use();
                quadSpriteSphereShader.SetUniform("projection", camera.projection);
                quadSpriteSphereShader.SetUniform("view", camera.View());
                quadSpriteSphereShader.SetUniform("cameraPos", camera.transform.pos);
                quadSpriteSphereShader.SetUniform("model", t.Model());
                glDrawArraysInstanced(GL_TRIANGLES, 0, quad.size(), particleCount);
                t.rot = glm::quat();
                break;
            case SPHERES:
                glBindVertexArray(sphereVao);
                t.scale = glm::vec3(0.6f);
                simpleInstancedModelShader.Use();
                simpleInstancedModelShader.SetUniform("projection", camera.projection);
                simpleInstancedModelShader.SetUniform("view", camera.View());
                simpleInstancedModelShader.SetUniform("cameraPos", camera.transform.pos);
                simpleInstancedModelShader.SetUniform("model", t.Model());
                glDrawArraysInstanced(GL_TRIANGLES, 0, sphere.size(), particleCount);
                break;
        }

        glBindVertexArray(cubeVao);
        // Ground
        simpleModelShader.Use();
        simpleModelShader.SetUniform("projection", camera.projection);
        simpleModelShader.SetUniform("view", camera.View());

        simpleModelShader.SetUniform("density", 0.5f);
        static glm::vec3 groundPos(box.x / 2.f, -0.6, box.z / 2.f);
        static glm::vec3 groundScale(40.f * 1.5, 0.2f, 30.f * 1.5);
        t.pos = groundPos;
        t.scale = groundScale;
        simpleModelShader.SetUniform("model", t.Model());
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Attractor
        static glm::vec<2, double> lastMousePos(0.f);
        glm::vec<2, double> mousePos;
        glfwGetCursorPos(window, &mousePos.x, &mousePos.y);
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT))
        {
            glm::vec<2, double> mouseDiff = mousePos - lastMousePos;
            glm::vec3 movement = camera.transform.Right() * (float)mouseDiff.x + camera.transform.Up() * -(float)mouseDiff.y;
            attractorTransform.pos += movement * 0.016f * 8.f;
        }
        simpleModelShader.SetUniform("density", attractorEnabled ? 5.f : 0.f);
        lastMousePos = mousePos;
        simpleModelShader.SetUniform("model", attractorTransform.Model());
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Spawner
        static glm::vec3 spawnerPos;

        // Skybox
        static bool skyboxEnabled = skyboxLoaded;
        if (skyboxEnabled && skyboxLoaded)
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

        const int particlesPer10Frames = 256;
        const int particleIncrease = 256;
        const int cooldownDuration = particleIncrease / particlesPer10Frames * 10;
        static int cooldown = 0;
        cooldown = cooldown-1 < 0 ? 0 : cooldown-1;
        if (cooldown <= 0 && particleCount < MAX_PARTICLE_COUNT && glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
        {
            particleCount += 256; 
            cooldown = cooldownDuration;
        }

        static bool settingsOpen = true;
        if (ImGui::Begin("Settings", &settingsOpen))
        {
            ImGui::Checkbox("Pause simulation", &simulationPaused);
            ImGui::SliderFloat("Gravity", &gravityScale, 0.f, 100.f);
            ImGui::Separator();
            ImGui::Checkbox("Attractor enabled", &attractorEnabled);
            ImGui::SliderFloat("Attractor inner radius", &attractorInnerRadius, 0.f, 100.f);
            ImGui::SliderFloat("Attractor oututer radius", &attractorOutterRadius, 0.f, 100.f);
            ImGui::SliderFloat("Attractor strength", &attractorStrength, 0.f, 2000.f);

            if (skyboxLoaded)
            {
                ImGui::Checkbox("Skybox", &skyboxEnabled);
            }
            else
            {
                bool unused;
                ImGui::Checkbox("Skybox (failed loading images)", &unused);
            }

            if (ImGui::TreeNodeEx("Particle render options", ImGuiTreeNodeFlags_DefaultOpen))
            {
                for (int i = 0; i < RENDER_AS_COUNT; i++)
                {
                    if (ImGui::RadioButton(renderAsStrings[i], renderAs == i))
                        renderAs = (renderAsEnum) i;
                }
                ImGui::TreePop();
            }
        }
        ImGui::End();

        ShowInfo(camera, particleCount);
        ImGuiWrapper::Render();

        glfwSwapBuffers(window);

        if (initialStart)
            initialStart = false;
    }

    ImGuiWrapper::Deinit();
    glfwTerminate();

    return 0;
}
