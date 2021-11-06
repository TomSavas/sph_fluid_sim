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

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
    const GLchar* message, const void* userParam)
{
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, \n\tmessage = %s\n",
            ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
}

void ShowInfo(Camera& camera, int particleCount)
{
    static bool open = true;
    const float PAD = 10.0f;

    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | 
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
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

    ImGui::SetNextWindowBgAlpha(0.35f);
    if (ImGui::Begin("Info", &open, window_flags))
    {
        ImGui::Text("SPH fluid sim");
        ImGui::SameLine();
#ifdef DEBUG
        ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "DEBUG");
#else
        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "RELEASE");
#endif
        ImGui::Separator();

        static const unsigned char *renderer = glGetString(GL_RENDERER);
        static const unsigned char *version = glGetString(GL_VERSION);
        ImGui::Text("%s\n%s", renderer, version);

        ImGui::Separator();
        ImGui::Text("Frametime: %.2fms", io.DeltaTime * 1000);
        ImGui::Text("FPS: %.1f", io.Framerate);
        ImGui::Separator();

        ImGui::Text("Particle count: %d", particleCount);
        ImGui::Separator();

        ImGui::Text("Press \"P\"    button to pause/unpause the simulation");
        ImGui::Text("Press \"C\"    button to emit particles");
        ImGui::Text("Press \"R\"    button to reset the simulation");
        ImGui::Text("Hold  \"Ctrl\" button to fly faster");
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
        // pos                       // uv
        glm::vec3(-1.f, -1.f,  0.f), glm::vec3(0.f, 0.f, 0.f),
        glm::vec3(-1.f,  1.f,  0.f), glm::vec3(0.f, 1.f, 0.f),
        glm::vec3( 1.f,  1.f,  0.f), glm::vec3(1.f, 1.f, 0.f),

        glm::vec3( 1.f,  1.f,  0.f), glm::vec3(1.f, 1.f, 0.f),
        glm::vec3( 1.f, -1.f,  0.f), glm::vec3(1.f, 0.f, 0.f),
        glm::vec3(-1.f, -1.f,  0.f), glm::vec3(0.f, 0.f, 0.f),
    };
}

#define INITIAL_PARTICLE_COUNT 1024*4
#define MAX_PARTICLE_COUNT 4096*16
struct ParticleData
{
    glm::vec4 posAndMass;
    glm::vec4 vel;
    glm::vec4 props;
    glm::vec4 force;
}; 

void initParticleData(unsigned int ssboId, std::vector<ParticleData>& particleData, int startIndex, int particleCount, glm::vec3 initialDir, float initialVel, glm::vec3 initialPos, glm::vec3 posSpread, bool pushDataToGpu = true)
{
    glm::vec4 velocity = glm::vec4(glm::normalize(initialDir) * initialVel, 0.f);

    for (int i = startIndex; i < startIndex + particleCount; i++)
    {
        particleData[i].posAndMass = glm::vec4(
                ((float)std::rand() / (float)RAND_MAX - 0.5f) * 2.f * posSpread.x + initialPos.x,
                ((float)std::rand() / (float)RAND_MAX - 0.5f) * 2.f * posSpread.y + initialPos.y,
                ((float)std::rand() / (float)RAND_MAX - 0.5f) * 2.f * posSpread.z + initialPos.z,
                1.f);
        particleData[i].vel = velocity;

        particleData[i].props = glm::vec4(0.f, 0.f, 0.f, 0.f); 
        particleData[i].force = glm::vec4(0.f, 0.f, 0.f, 0.f); 
    }

    if (pushDataToGpu)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboId);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(ParticleData) * startIndex, sizeof(ParticleData) * particleCount, &particleData[startIndex]);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
}

int main(void) 
{
    if (!glfwInit())
    {
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    GLFWwindow *window = glfwCreateWindow(1920, 1080, "SPH fluid sim", NULL, NULL);
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

    ImGuiWrapper::Init(window);
    ImGuiIO& io = ImGui::GetIO();
    // My hardware/os issue -- a phantom controller seems to be always plugged in and holding one of the joysticks down
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


    static glm::vec3 initialParticleDirection = glm::vec3(1.f, -0.9f, 0.f);
    static float initialParticleVelocity = 65.f;
    static glm::vec3 initialParticlePos = glm::vec3(10.f, 30.f, 10.f);
    static glm::vec3 initialParticleSpread = glm::vec3(3.f, 3.f, 3.f);

    int particleCount = INITIAL_PARTICLE_COUNT;
    glm::vec3 box(35.f * 1.5, 100.f * 1.5, 15.f * 1.5);

    std::vector<ParticleData> particleData(MAX_PARTICLE_COUNT);
    unsigned int particleDataId;
    glGenBuffers(1, &particleDataId);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleDataId);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particleDataId);
    initParticleData(particleDataId, particleData, 0, particleCount, initialParticleDirection, 0.f, glm::vec3(26.f, 40.f, 10.f), glm::vec3(25.f, 8.f, 15.f), false);
    initParticleData(particleDataId, particleData, particleCount, MAX_PARTICLE_COUNT - particleCount, initialParticleDirection, initialParticleVelocity, initialParticlePos, initialParticleSpread, false);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ParticleData) * MAX_PARTICLE_COUNT, particleData.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    Shader compShader("../src/fluid_sim.comp");
    Shader densityCompShader("../src/sph_density.comp");
    Shader forceCompShader("../src/sph_force.comp");
    Shader integrationCompShader("../src/sph_integration.comp");
    Shader simpleModelShader("../src/simple_model.vert", "../src/simple_model.frag");
    Shader simpleInstancedModelShader("../src/instanced_simple_model.vert", "../src/simple_model.frag");
    Shader skyboxShader("../src/skybox.vert", "../src/skybox.frag");
    Shader pointShader("../src/point.vert", "../src/point.frag");
    Shader quadSpriteSphereShader("../src/quad_sprite_sphere.vert", "../src/quad_sprite_sphere.frag");
    Shader quadSpriteSphereThicknessShader("../src/quad_sprite_sphere.vert", "../src/quad_sprite_sphere_thickness.frag");
    Shader normalFromDepthShader("../src/screen_quad.vert", "../src/normal_from_depth.frag");
    Shader bilateralDepthBlurShader("../src/screen_quad.vert", "../src/bilateral_depth_blur.frag");
    Shader thicknessGaussianBlurShader("../src/screen_quad.vert", "../src/thickness_gaussian_blur.frag");
    Shader fluidShader("../src/screen_quad.vert", "../src/fluid.frag");

    // Depth FBO
    unsigned int depthFbo;
    glGenFramebuffers(1, &depthFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, depthFbo);

    unsigned int depthTex;
    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 1920, 1080, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);

    unsigned int worldPosTex;
    glGenTextures(1, &worldPosTex);
    glBindTexture(GL_TEXTURE_2D, worldPosTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Blurred depth FBO
    unsigned int blurredDepthFbo;
    glGenFramebuffers(1, &blurredDepthFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, blurredDepthFbo);

    unsigned int horizontallyBlurredDepthTex;
    glGenTextures(1, &horizontallyBlurredDepthTex);
    glBindTexture(GL_TEXTURE_2D, horizontallyBlurredDepthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, horizontallyBlurredDepthTex, 0);

    // Normal FBO
    unsigned int normalFbo;
    glGenFramebuffers(1, &normalFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, normalFbo);

    unsigned int normalTex;
    glGenTextures(1, &normalTex);
    glBindTexture(GL_TEXTURE_2D, normalTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, normalTex, 0);

    // Thickness FBO
    unsigned int thicknessFbo;
    glGenFramebuffers(1, &thicknessFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, thicknessFbo);

    unsigned int thicknessAccumulationTex;
    glGenTextures(1, &thicknessAccumulationTex);
    glBindTexture(GL_TEXTURE_2D, thicknessAccumulationTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    unsigned int thicknessTex;
    glGenTextures(1, &thicknessTex);
    glBindTexture(GL_TEXTURE_2D, thicknessTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.f);

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 2, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, particleDataId);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), NULL);
    glVertexAttribDivisor(1, 1);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void*)sizeof(glm::vec4));
    glVertexAttribDivisor(2, 1);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void*)(sizeof(glm::vec4) * 2));
    glVertexAttribDivisor(3, 1);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void*)(sizeof(glm::vec4) * 3));
    glVertexAttribDivisor(4, 1);

    unsigned int screenQuadVao;
    glGenVertexArrays(1, &screenQuadVao);
    glBindVertexArray(screenQuadVao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    unsigned int screenQuadVbo;
    glGenBuffers(1, &screenQuadVbo);
    glBindBuffer(GL_ARRAY_BUFFER, screenQuadVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * quad.size(), quad.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 2, NULL);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 2, (void*) sizeof(glm::vec3));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glDisable(GL_CULL_FACE);

    glEnable(GL_PROGRAM_POINT_SIZE);

    Camera camera;
    camera.transform.pos = glm::vec3(20, 50, 170);

    while (!glfwWindowShouldClose(window)) 
    {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.Update(window);
        ImGuiWrapper::PreRender();

        static float quadSpriteSphereSize = 0.9f;
        static float gravityScale = 20;
        static bool simulationPaused = false;
        static Transform attractorTransform(glm::vec3(box.x / 2.f, box.y / 2.f, box.z / 2.f));
        static float attractorInnerRadius = 10.f;
        static float attractorOutterRadius = 25.f;
        static float attractorStrength = 1000.f;
        static bool attractorEnabled = false;
        static bool renderInactiveParticles = false;

        static float smoothingRadius = 1.0f;
        static float densityReference = 1.f;
        static float pressureConst = 250.f;
        static float viscosityConst = 1.3f; //0.188f;
        if (!simulationPaused)
        {
            float poly6Const = 315.f / (64.f * PI * pow(smoothingRadius, 9.f));
            float spikyConst = -45.f / (PI * pow(smoothingRadius, 6));

            densityCompShader.Use();
            densityCompShader.SetUniform("particleCount", particleCount);
            densityCompShader.SetUniform("smoothingRadius", smoothingRadius);
            densityCompShader.SetUniform("densityReference", densityReference);
            densityCompShader.SetUniform("pressureConst", pressureConst);
            densityCompShader.SetUniform("poly6Const", poly6Const);
            glDispatchCompute(particleCount / 256.f, 1, 1);
            glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

            forceCompShader.Use();
            forceCompShader.SetUniform("particleCount", particleCount);
            forceCompShader.SetUniform("smoothingRadius", smoothingRadius);
            forceCompShader.SetUniform("viscosityConst", viscosityConst);
            forceCompShader.SetUniform("spikyConst", spikyConst);
            forceCompShader.SetUniform("laplacianConst", -spikyConst);
            forceCompShader.SetUniform("gravity", glm::vec3(0.f, -1.f, 0.f) * gravityScale);
            glDispatchCompute(particleCount / 256.f, 1, 1);
            glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

            integrationCompShader.Use();
            integrationCompShader.SetUniform("timestep", 0.016f);
            integrationCompShader.SetUniform("gravity", glm::vec3(0.f, -1.f, 0.f) * gravityScale);
            integrationCompShader.SetUniform("box", box);
            integrationCompShader.SetUniform("attractorPos", attractorTransform.pos);
            integrationCompShader.SetUniform("attractorInnerRadius", attractorInnerRadius);
            integrationCompShader.SetUniform("attractorOutterRadius", attractorOutterRadius);
            integrationCompShader.SetUniform("attractorStrength", attractorStrength * (attractorEnabled ? 1.f : 0.f));
            glDispatchCompute(particleCount / 256.f, 1, 1);
            glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
        }

        Transform t;
        int renderedParticleCount = renderInactiveParticles ? MAX_PARTICLE_COUNT : particleCount;

        static Transform lightTransform;
        static float lightPosCounter = 0.f;
        lightPosCounter += 0.01f;
        lightTransform.pos = glm::vec3(glm::sin(lightPosCounter) * 50.f + 25, 50.f, glm::cos(lightPosCounter) * 50.f + 10.f);
        // light shines toward the point it orbits
        glm::vec3 normLightDir = -glm::normalize(glm::vec3(25.f, 0.f, 10.f) - lightTransform.pos);

        // Skybox
        static bool skyboxEnabled = skyboxLoaded;
        if (skyboxEnabled && skyboxLoaded)
        {
            glDepthFunc(GL_LEQUAL);
            glDepthMask(GL_FALSE);
            skyboxShader.Use();
            glm::mat4 viewProjection = camera.projection * glm::mat4(glm::mat3(camera.View()));
            skyboxShader.SetUniform("viewProjection", viewProjection);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemapId);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LESS);
        }

#define IDENTITY(VALUE) VALUE
#define STRINGIFY(VALUE) #VALUE
#define RENDER_AS_VALUES(F)      \
        F(NONE),                 \
        F(POINTS),               \
        F(QUADS),                \
        F(QUAD_SPRITE_SPHERES),  \
        F(SPHERES),              \
        F(DEPTH),                \
        F(RECOVERED_NORMALS),    \
        F(THICKNESS),            \
        F(FULLY_SHADED),         \
        F(RENDER_AS_COUNT)  

        static const char* renderAsStrings[] = { RENDER_AS_VALUES(STRINGIFY) };
        static enum renderAsEnum
        {
            RENDER_AS_VALUES(IDENTITY)
        } renderAs = FULLY_SHADED;
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
                glDrawArraysInstanced(GL_POINTS, 0, 1, renderedParticleCount);
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
                simpleInstancedModelShader.SetUniform("useDensityForColor", 1);
                glDrawArraysInstanced(GL_TRIANGLES, 0, quad.size(), renderedParticleCount);
                t.rot = glm::quat();
                break;
            case QUAD_SPRITE_SPHERES:
                glBindVertexArray(quadVao);
                t.scale = glm::vec3(1.0f);
                t.rot = camera.transform.rot;
                quadSpriteSphereShader.Use();
                quadSpriteSphereShader.SetUniform("size", quadSpriteSphereSize);
                quadSpriteSphereShader.SetUniform("projection", camera.projection);
                quadSpriteSphereShader.SetUniform("view", camera.View());
                quadSpriteSphereShader.SetUniform("cameraPos", camera.transform.pos);
                quadSpriteSphereShader.SetUniform("model", t.Model());
                quadSpriteSphereShader.SetUniform("lightDir", normLightDir);
                glDrawArraysInstanced(GL_TRIANGLES, 0, quad.size(), renderedParticleCount);
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
                simpleInstancedModelShader.SetUniform("useDensityForColor", 1);
                glDrawArraysInstanced(GL_TRIANGLES, 0, sphere.size(), renderedParticleCount);
                break;
            case DEPTH:
            case RECOVERED_NORMALS:
            case THICKNESS:
            case FULLY_SHADED:
                // render depth and worldPos textures
                {
                    glBindFramebuffer(GL_FRAMEBUFFER, depthFbo);
                    glClear(GL_DEPTH_BUFFER_BIT);
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, worldPosTex, 0);

                    glBindVertexArray(quadVao);
                    t.scale = glm::vec3(1.f);
                    t.rot = camera.transform.rot;
                    quadSpriteSphereShader.Use();
                    quadSpriteSphereShader.SetUniform("size", quadSpriteSphereSize);
                    quadSpriteSphereShader.SetUniform("projection", camera.projection);
                    quadSpriteSphereShader.SetUniform("view", camera.View());
                    quadSpriteSphereShader.SetUniform("cameraPos", camera.transform.pos);
                    quadSpriteSphereShader.SetUniform("model", t.Model());
                    quadSpriteSphereShader.SetUniform("lightDir", normLightDir);
                    glDrawArraysInstanced(GL_TRIANGLES, 0, quad.size() / 2.f, renderedParticleCount);
                    t.rot = glm::quat();
                }

                // blur depth
                {
                    if (renderAs == DEPTH)
                    {
                        glBindFramebuffer(GL_FRAMEBUFFER, 0);
                    }
                    else
                    {
                        glBindFramebuffer(GL_FRAMEBUFFER, blurredDepthFbo);
                        glClearColor(1.0f, 1.0f, 1.0f, 1.f);
                        glClear(GL_COLOR_BUFFER_BIT);
                        glClearColor(0.0f, 0.0f, 0.0f, 1.f);
                    }

                    bilateralDepthBlurShader.Use();

                    glActiveTexture(GL_TEXTURE0);
                    bilateralDepthBlurShader.SetUniform("tex", 0);
                    glBindTexture(GL_TEXTURE_2D, depthTex);

                    glBindVertexArray(screenQuadVao);
                    glDrawArrays(GL_TRIANGLES, 0, quad.size() / 2.f);

                    if (renderAs == DEPTH)
                    {
                        break;
                    }
                }

                // recalculate normals from depth
                {
                    if (renderAs == RECOVERED_NORMALS)
                    {
                        glBindFramebuffer(GL_FRAMEBUFFER, 0);
                    }
                    else
                    {
                        glBindFramebuffer(GL_FRAMEBUFFER, normalFbo);
                        glClearColor(1.0f, 1.0f, 1.0f, 1.f);
                        glClear(GL_COLOR_BUFFER_BIT);
                        glClearColor(0.0f, 0.0f, 0.0f, 1.f);
                    }

                    normalFromDepthShader.Use();

                    glActiveTexture(GL_TEXTURE0);
                    normalFromDepthShader.SetUniform("tex", 0);
                    glBindTexture(GL_TEXTURE_2D, horizontallyBlurredDepthTex);

                    normalFromDepthShader.SetUniform("invP", glm::inverse(camera.projection));
                    normalFromDepthShader.SetUniform("invV", glm::inverse(camera.View()));

                    glBindVertexArray(screenQuadVao);
                    glDrawArrays(GL_TRIANGLES, 0, quad.size() / 2.f);

                    if (renderAs == RECOVERED_NORMALS)
                    {
                        break;
                    }
                }

                // render thickness
                {
                    // render thickness accumulation
                    glBindFramebuffer(GL_FRAMEBUFFER, thicknessFbo);
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, thicknessAccumulationTex, 0);
                    glClearColor(0.0f, 0.0f, 0.0f, 0.f);
                    glClear(GL_COLOR_BUFFER_BIT);
                    glClearColor(0.0f, 0.0f, 0.0f, 1.f);

                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glDisable(GL_DEPTH_TEST);

                    glBindVertexArray(quadVao);

                    t.scale = glm::vec3(1.0f);
                    t.rot = camera.transform.rot;
                    quadSpriteSphereThicknessShader.Use();
                    quadSpriteSphereThicknessShader.SetUniform("size", quadSpriteSphereSize);
                    quadSpriteSphereThicknessShader.SetUniform("projection", camera.projection);
                    quadSpriteSphereThicknessShader.SetUniform("view", camera.View());
                    quadSpriteSphereThicknessShader.SetUniform("cameraPos", camera.transform.pos);
                    quadSpriteSphereThicknessShader.SetUniform("model", t.Model());
                    glDrawArraysInstanced(GL_TRIANGLES, 0, quad.size(), renderedParticleCount);
                    t.rot = glm::quat();

                    glEnable(GL_DEPTH_TEST);
                    glDisable(GL_BLEND);

                    if (renderAs == THICKNESS)
                    {
                        glBindFramebuffer(GL_FRAMEBUFFER, 0);
                    }
                    else
                    {
                        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, thicknessTex, 0);
                        glClearColor(0.0f, 0.0f, 0.0f, 0.f);
                        glClear(GL_COLOR_BUFFER_BIT);
                        glClearColor(0.0f, 0.0f, 0.0f, 1.f);
                    }

                    thicknessGaussianBlurShader.Use();

                    glActiveTexture(GL_TEXTURE0);
                    thicknessGaussianBlurShader.SetUniform("tex", 0);
                    glBindTexture(GL_TEXTURE_2D, thicknessAccumulationTex);

                    glBindVertexArray(screenQuadVao);
                    glDrawArrays(GL_TRIANGLES, 0, quad.size() / 2.f);

                    if (renderAs == THICKNESS)
                    {
                        break;
                    }
                }

                // final composition and lighting
                {
                    glBindFramebuffer(GL_FRAMEBUFFER, 0);
                    glEnable(GL_BLEND);
                    fluidShader.Use();

                    glActiveTexture(GL_TEXTURE0);
                    fluidShader.SetUniform("thicknessTex", 0);
                    glBindTexture(GL_TEXTURE_2D, thicknessTex);

                    glActiveTexture(GL_TEXTURE1);
                    fluidShader.SetUniform("normalTex", 1);
                    glBindTexture(GL_TEXTURE_2D, normalTex);

                    glActiveTexture(GL_TEXTURE2);
                    fluidShader.SetUniform("worldPosTex", 2);
                    glBindTexture(GL_TEXTURE_2D, worldPosTex);

                    fluidShader.SetUniform("lightDir", normLightDir);
                    fluidShader.SetUniform("cameraPos", camera.transform.pos);

                    glBindVertexArray(screenQuadVao);
                    glDrawArrays(GL_TRIANGLES, 0, quad.size() / 2.f);
                }
                break;
            case NONE:
            default:
                break;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Attractor
        glBindVertexArray(cubeVao);
        simpleModelShader.Use();
        simpleModelShader.SetUniform("projection", camera.projection);
        simpleModelShader.SetUniform("view", camera.View());
        static glm::vec<2, double> lastMousePos(0.f);
        glm::vec<2, double> mousePos;
        glfwGetCursorPos(window, &mousePos.x, &mousePos.y);
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT))
        {
            glm::vec<2, double> mouseDiff = mousePos - lastMousePos;
            glm::vec3 movement = camera.transform.Right() * (float)mouseDiff.x + camera.transform.Up() * -(float)mouseDiff.y;
            attractorTransform.pos += movement * 0.016f * 8.f;
        }
        simpleModelShader.SetUniform("useDensityForColor", 0);
        simpleModelShader.SetUniform("color", attractorEnabled ? glm::vec3(1.f, 0.f, 0.f) : glm::vec3(0.f, 0.f, 0.f));
        lastMousePos = mousePos;
        simpleModelShader.SetUniform("model", attractorTransform.Model());
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Light
        simpleModelShader.SetUniform("useDensityForColor", 0);
        simpleModelShader.SetUniform("color", glm::vec3(1.f, 0.87f, 0.6f));
        simpleModelShader.SetUniform("model", lightTransform.Model());
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Controls
        const int particlesPer10Frames = 256;
        const int particleIncrease = 256;
        const int cooldownDuration = particleIncrease / particlesPer10Frames * 10;
        static int cooldown = 0;
        cooldown = cooldown-1 < 0 ? 0 : cooldown-1;
        if (cooldown <= 0 && particleCount < MAX_PARTICLE_COUNT && glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        {
            particleCount += particleIncrease; 
            cooldown = cooldownDuration;
        }

        static int pauseCooldown = 0;
        pauseCooldown = pauseCooldown-1 < 0 ? 0 : pauseCooldown-1;
        if (pauseCooldown <= 0 && glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        {
            simulationPaused = !simulationPaused;
            pauseCooldown = cooldownDuration / 2;
        }

        static int resetCooldown = 0;
        resetCooldown = resetCooldown-1 < 0 ? 0 : resetCooldown-1;
        if (resetCooldown <= 0 && glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {
            particleCount = INITIAL_PARTICLE_COUNT;
            initParticleData(particleDataId, particleData, 0, particleCount, initialParticleDirection, 0.f, glm::vec3(26.f, 40.f, 10.f), glm::vec3(25.f, 8.f, 15.f));
            initParticleData(particleDataId, particleData, particleCount, MAX_PARTICLE_COUNT - particleCount, initialParticleDirection, initialParticleVelocity, initialParticlePos, initialParticleSpread);
            resetCooldown = cooldownDuration / 2;
        }

        // Settings
        static bool settingsOpen = true;
        if (ImGui::Begin("Settings", &settingsOpen))
        {
            ImGui::Checkbox("Pause simulation", &simulationPaused);

            if (ImGui::TreeNodeEx("Render settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (skyboxLoaded)
                {
                    ImGui::Checkbox("Skybox", &skyboxEnabled);
                }
                else
                {
                    bool unused;
                    ImGui::Checkbox("Skybox (failed loading images)", &unused);
                }

                ImGui::Checkbox("Render inactive particles (disabling increases FPS)", &renderInactiveParticles);
                if (ImGui::TreeNodeEx("Particle render options", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    for (int i = 0; i < RENDER_AS_COUNT; i++)
                    {
                        if (ImGui::RadioButton(renderAsStrings[i], renderAs == i))
                            renderAs = (renderAsEnum) i;
                    }
                    ImGui::TreePop();
                }

                switch (renderAs)
                {
                    case QUAD_SPRITE_SPHERES:
                    case DEPTH:
                    case RECOVERED_NORMALS:
                    case THICKNESS:
                    case FULLY_SHADED:
                        ImGui::SliderFloat("Quad sprite sphere size", &quadSpriteSphereSize, 0.1f, 2.f);
                        break;
                }

                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("Attractor settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Checkbox("Attractor enabled", &attractorEnabled);
                ImGui::SliderFloat("Attractor inner radius", &attractorInnerRadius, 0.f, 100.f);
                ImGui::SliderFloat("Attractor oututer radius", &attractorOutterRadius, 0.f, 100.f);
                ImGui::SliderFloat("Attractor strength", &attractorStrength, 0.f, 2000.f);
                ImGui::TreePop();
            }
            
            if (ImGui::TreeNodeEx("Particle emitter settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
            {
                bool dirChanged = ImGui::SliderFloat3("Initial particle direction", &initialParticleDirection[0], -1.f, 1.f);
                bool velChanged = ImGui::SliderFloat("Initial particle velocity", &initialParticleVelocity, 0.f, 200.f);
                bool posChanged = ImGui::SliderFloat3("Initial particle pos", &initialParticlePos[0], -100.f, 100.f);
                bool spreadChanged = ImGui::SliderFloat3("Initial particle spread", &initialParticleSpread[0], 0.f, 100.f);
                if (dirChanged || velChanged || posChanged || spreadChanged)
                {
                    glGetNamedBufferSubData(particleDataId, 0, sizeof(ParticleData) * MAX_PARTICLE_COUNT, particleData.data());
                    initParticleData(particleDataId, particleData, particleCount, MAX_PARTICLE_COUNT - particleCount, initialParticleDirection, initialParticleVelocity, initialParticlePos, initialParticleSpread);
                }
                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("SPH sim settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::SliderFloat("Gravity", &gravityScale, 0.f, 100.f);
                ImGui::SliderFloat("Smoothing radius", &smoothingRadius, 0.001f, 10.f);
                ImGui::SliderFloat("Minimum density", &densityReference, 1.f, 2.f);
                ImGui::SliderFloat("Pressure const", &pressureConst, 1.f, 1000.f);
                ImGui::SliderFloat("Viscosity const", &viscosityConst, 0.001f, 10.f);
                ImGui::TreePop();
            }
        }
        ImGui::End();

        ShowInfo(camera, particleCount);
        ImGuiWrapper::Render();

        glfwSwapBuffers(window);
    }

    ImGuiWrapper::Deinit();
    glfwTerminate();

    return 0;
}
