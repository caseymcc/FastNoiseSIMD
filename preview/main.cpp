#include "HastyNoise/hastyNoise.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <chrono>
namespace chrono=std::chrono;

struct WindowInfo
{
    WindowInfo():width(1280), height(930), resized(true){}

    bool resized;
    int width;
    int height;
};

void error_callback(int error, const char* description);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

WindowInfo g_windowInfo;

std::vector<char> packVectorString(std::vector<std::string> &values)
{
    std::vector<char> packed;

    size_t size=0;
    for(std::string &value:values)
        size+=value.size()+1;//null terminator

    packed.resize(size+1);

    size=0;
    for(std::string &value:values)
    {
        memcpy(&packed[size], value.c_str(), value.size());
        size+=value.size();
        packed[size]=0;
        size++;
    }
    packed[size]=0;

    return packed;
}

template<typename _Type>
std::vector<char> getTypeNamesPacked(std::vector<std::string> &names)
{
    std::vector<std::string> temp;

    for(auto &key:HastyNoise::EnumKeys<_Type>::keys)
        names.push_back(key.name);

    return packVectorString(names);
}

template<typename _Type>
std::vector<char> getTypeNamesPacked()
{
    std::vector<std::string> temp;

    return getTypeNamesPacked<_Type>(temp);
}

template<typename _Type>
_Type getTypeFromIndex(std::vector<std::string> &items, int index) 
{
    std::string &itemName=items[index];

    for(auto &key:HastyNoise::EnumKeys<_Type>::keys)
    {
        if(key.name == itemName)
            return key.type;
    }
    return HastyNoise::EnumKeys<_Type>::keys[0].type;
}

template<typename _Type>
int getIndexFromType(std::vector<std::string> &items, _Type type)
{
    std::string itemName=HastyNoise::getName(type);

    for(int i=0; i<items.size(); ++i)
    {
        if(items[i] == itemName)
            return i;
    }
    return 0;
}

int main(int argc, char ** argv)
{
    HastyNoise::loadSimd();

    // Setup window
    glfwSetErrorCallback(error_callback);
    if(!glfwInit())
        return 1;

    const char* glsl_version="#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac

    // Create window with graphics context
    GLFWwindow* window=glfwCreateWindow(g_windowInfo.width, g_windowInfo.height, "HastyNoise Preview", NULL, NULL);

    if(window==NULL)
        return 1;
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialize OpenGL loader
    bool err=glewInit()!=GLEW_OK;
    
    if(err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io=ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImVec4 clear_color=ImVec4(0.0f, 0.0f, 0.0f, 1.00f);


//Hasty noise/Status
    std::unique_ptr<HastyNoise::NoiseSIMD> noise;
    size_t fastestSimd=HastyNoise::GetFastestSIMD();
    
    int currentNoiseSizeX=512;
    int currentNoiseSizeY=512;
    HastyNoise::SIMDType currentSimdType=(HastyNoise::SIMDType)fastestSimd;
    std::vector<char> simdTypesPacked=getTypeNamesPacked<HastyNoise::SIMDType>();
    std::vector<std::string> simdTypes;

    for(auto &key:HastyNoise::EnumKeys<HastyNoise::SIMDType>::keys)
    {
        if(HastyNoise::SupportedSimd(key.type))
            simdTypes.push_back(key.name);
    }
    simdTypesPacked=packVectorString(simdTypes);

    HastyNoise::NoiseType currentNoiseType=HastyNoise::NoiseType::PerlinFractal;
    std::vector<std::string> noiseTypes;
    std::vector<char> noiseTypesPacked=getTypeNamesPacked<HastyNoise::NoiseType>(noiseTypes);

    HastyNoise::FractalType currentFractalType=HastyNoise::FractalType::FBM;
    std::vector<std::string> fractalTypes;
    std::vector<char> fractalTypesPacked=getTypeNamesPacked<HastyNoise::FractalType>(fractalTypes);

    HastyNoise::CellularDistance currentCellularDistance=HastyNoise::CellularDistance::Euclidean;
    std::vector<std::string> cellularDistanceTypes;
    std::vector<char> cellularDistanceTypesPacked=getTypeNamesPacked<HastyNoise::CellularDistance>(cellularDistanceTypes);

    HastyNoise::CellularReturnType currentCellularReturnType=HastyNoise::CellularReturnType::Distance;
    std::vector<std::string> cellularReturnTypes;
    std::vector<char> cellularReturnTypesPacked=getTypeNamesPacked<HastyNoise::CellularReturnType>(cellularReturnTypes);
    
    bool noiseTextureDirty=false;
    bool noiseTextureValid=false;
    
    bool noiseBufferDirty=true;
    HastyNoise::FloatBuffer noiseBuffer;
    GLuint noiseTextureId;
    float noiseBufferMin=0.0;
    float noiseBufferMax=0.0;
    
    glGenTextures(1, &noiseTextureId);

    bool show=true;
    HastyNoise::SIMDType simdType=currentSimdType;

    int noiseSizeX=currentNoiseSizeX;
    int noiseSizeY=currentNoiseSizeY;
    bool noiseInvert=false;
    int noiseSeed=12345;
    float noiseFrequency=0.2f;
    bool fractalDisable=!HastyNoise::isFractal(currentNoiseType);
    bool cellularDisable=!(currentNoiseType==HastyNoise::NoiseType::Cellular);

    int noiseFractalOctaves=3;
    float noiseFractalLacunarity=2.0;
    float noiseFractalGain=0.5;

    int noiseCellularDistance=0;
    int noiseCellularReturnType=0;

    double elapsedTime=0.0;

    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    noise=HastyNoise::CreateNoise(noiseSeed, (size_t)currentSimdType);
    
    // Main loop
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        if(g_windowInfo.resized)
        {
            g_windowInfo.resized=false;
            glViewport(0, 0, g_windowInfo.width, g_windowInfo.height);
        }

        if(noiseTextureDirty)
        {
            glBindTexture(GL_TEXTURE_2D, noiseTextureId);

            float *buffer=noiseBuffer.get();
            size_t sizeBuffer=currentNoiseSizeX*currentNoiseSizeY;

            noiseBufferMin=0.0f;
            noiseBufferMax=1.0f;

            for(size_t i=0; i<sizeBuffer; ++i)
            {
                noiseBufferMin=std::min(noiseBufferMin, buffer[i]);
                noiseBufferMax=std::max(noiseBufferMax, buffer[i]);
            }

            std::vector<GLubyte> textureBuffer;
            size_t index=0;
            float delta=255/(noiseBufferMax-noiseBufferMin);

            textureBuffer.resize(currentNoiseSizeX*noiseSizeY*3);
            for(size_t i=0; i<sizeBuffer; ++i)
            {
                GLubyte value=std::max(0, std::min(255, (int)((buffer[i]-noiseBufferMin)*delta)));
                
                textureBuffer[index++]=value;
                textureBuffer[index++]=value;
                textureBuffer[index++]=value;
            }

            if(noiseInvert)
            {
                size_t size=sizeBuffer*3;
                for(size_t i=0; i<size; ++i)
                    textureBuffer[i]=255-textureBuffer[i];
            }

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, currentNoiseSizeX, currentNoiseSizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, &textureBuffer[0]);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

            noiseTextureDirty=false;
            noiseTextureValid=true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos({0, 0});
        ImGui::SetNextWindowSize({350, (float)g_windowInfo.height});

        ImGui::Begin("Controls", &show, 
            ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize);
        
        if(ImGui::Button("Generate"))
        {
            if(simdType!=currentSimdType)
            {
                noise=HastyNoise::CreateNoise(noiseSeed, (size_t)simdType);
                currentSimdType=simdType;

                noiseBufferDirty=true;
            }

            noise->SetNoiseType(currentNoiseType);
            noise->SetSeed(noiseSeed);
            noise->SetFrequency(noiseFrequency);

            if(HastyNoise::isFractal(currentNoiseType))
            {
                noise->SetFractalType(currentFractalType);
                noise->SetFractalOctaves(noiseFractalOctaves);
                noise->SetFractalLacunarity(noiseFractalLacunarity);
                noise->SetFractalGain(noiseFractalGain);
            }

            if(currentNoiseType==HastyNoise::NoiseType::Cellular)
            {
                noise->SetCellularDistanceFunction(currentCellularDistance);
                noise->SetCellularReturnType(currentCellularReturnType);
            }

            if(noiseSizeX != currentNoiseSizeX)
            {
                currentNoiseSizeX=noiseSizeX;
                noiseBufferDirty=true;
            }

            if(noiseSizeY!=currentNoiseSizeY)
            {
                currentNoiseSizeY=noiseSizeY;
                noiseBufferDirty=true;
            }

            if(noiseBufferDirty)
            {
                noiseBuffer=HastyNoise::GetEmptySet(currentNoiseSizeX, currentNoiseSizeY, 1, (size_t)currentSimdType);
                noiseBufferDirty=false;
            }

            chrono::high_resolution_clock::time_point startTime;
            chrono::high_resolution_clock::time_point endTime;

            startTime=chrono::high_resolution_clock::now();

            noise->FillSet(noiseBuffer.get(), 0, 0, 0, currentNoiseSizeX, currentNoiseSizeY, 1);

            endTime=chrono::high_resolution_clock::now();
            elapsedTime=chrono::duration_cast<chrono::milliseconds>(endTime-startTime).count();

            noiseTextureDirty=true;
        }
        ImGui::SameLine();
        ImGui::BeginGroup();
        ImGui::Text("Time(ms): %f", elapsedTime);
        ImGui::Text("Min: %f", noiseBufferMin);
        ImGui::Text("Max: %f", noiseBufferMax);
        ImGui::EndGroup();

        ImGui::Separator();

        int simdIndex=getIndexFromType(simdTypes, simdType);
        if(ImGui::Combo("SIMD Level", &simdIndex, &simdTypesPacked[0]))
            simdType=getTypeFromIndex<HastyNoise::SIMDType>(simdTypes, simdIndex);

        if(ImGui::InputInt("Width", &noiseSizeX, 2, 16))
            noiseSizeX=noiseSizeX%2?noiseSizeX-1:noiseSizeX;

        if(ImGui::InputInt("Height", &noiseSizeY, 2, 16))
            noiseSizeY=noiseSizeY%2?noiseSizeY-1:noiseSizeY;

        ImGui::Text("");
        ImGui::Text("General Settings");
        if(ImGui::Checkbox("Invert", &noiseInvert))
            noiseTextureDirty=true;
        
        int noiseIndex=getIndexFromType(noiseTypes, currentNoiseType);
        if(ImGui::Combo("Noise Type", &noiseIndex, &noiseTypesPacked[0]))
        {
            currentNoiseType=getTypeFromIndex<HastyNoise::NoiseType>(noiseTypes, noiseIndex);

            fractalDisable=!HastyNoise::isFractal((HastyNoise::NoiseType)currentNoiseType);
            cellularDisable=!(currentNoiseType==HastyNoise::NoiseType::Cellular);
        }

        ImGui::InputInt("Seed", &noiseSeed);
        ImGui::SliderFloat("Frequency", &noiseFrequency, 0.01f, 1.0f, "%.3f");

        ImGui::Text("");
        ImGui::Text("Fractal Settings");

        if(fractalDisable)
        {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }

        int fractalIndex=getIndexFromType(fractalTypes, currentFractalType);
        if(ImGui::Combo("Fractal Type", &fractalIndex, &fractalTypesPacked[0]))
            currentFractalType=getTypeFromIndex<HastyNoise::FractalType>(fractalTypes, fractalIndex);
        
        ImGui::SliderInt("Octaves", &noiseFractalOctaves, 1, 15);
        ImGui::SliderFloat("Lacunarity", &noiseFractalLacunarity, 0.01f, 2.0f, "%.3f");
        ImGui::SliderFloat("Gain", &noiseFractalGain, 0.01f, 10.0f, "%.3f");
        
        if(fractalDisable)
        {
            ImGui::PopItemFlag();
            ImGui::PopStyleVar();
        }

        ImGui::Text("");
        ImGui::Text("Cellular Settings");

        if(cellularDisable)
        {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }

        int cellularReturnIndex=getIndexFromType(cellularReturnTypes, currentCellularReturnType);
        if(ImGui::Combo("Return Type", &cellularReturnIndex, &cellularReturnTypesPacked[0]))
            currentCellularReturnType=getTypeFromIndex<HastyNoise::CellularReturnType>(cellularReturnTypes, cellularReturnIndex);

        int cellularDistanceIndex=getIndexFromType(cellularDistanceTypes, currentCellularDistance);
        if(ImGui::Combo("Distance Function", &cellularDistanceIndex, &cellularDistanceTypesPacked[0]))
            currentCellularDistance=getTypeFromIndex<HastyNoise::CellularDistance>(cellularDistanceTypes, cellularDistanceIndex);
        
        if(cellularDisable)
        {
            ImGui::PopItemFlag();
            ImGui::PopStyleVar();
        }

        ImGui::End();

        int imageWidth=g_windowInfo.width-350-20;
        int imageHeight=g_windowInfo.height-20;

        ImGui::SetNextWindowPos({350, 0});
        ImGui::SetNextWindowSize({(float)(g_windowInfo.width-350), (float)g_windowInfo.height});
        ImGui::Begin("Image", &show,
            ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize);
        
        if(noiseTextureValid)
            ImGui::Image((ImTextureID)noiseTextureId, {(float)imageWidth, (float)imageHeight}, {0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 0.5f});

        ImGui::End();

        // Rendering
        ImGui::Render();
        
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    g_windowInfo.resized=true;
    g_windowInfo.width=width;
    g_windowInfo.height=height;
}
