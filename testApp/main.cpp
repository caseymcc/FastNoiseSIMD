#include "HastyNoise/HastyNoise.h"

#ifdef _MSC_VER
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#include <realtimeapiset.h>
#endif

#if HN_USE_FILESYSTEM == 1
#include <filesystem>
namespace fs=std::filesystem;
#elif HN_USE_FILESYSTEM == 2
#include <experimental/filesystem>
#ifdef _MSC_VER
namespace fs=std::experimental::filesystem::v1;
#else
namespace fs=std::experimental::filesystem;
#endif
#endif

#include <iostream>
#include <cassert>
#include <cstring>
#include <cmath>

#include <chrono>
namespace chrono=std::chrono;

struct NoiseInfo
{
    NoiseInfo(std::string name, HastyNoise::NoiseType type):name(name), type(type) {}

    std::string name;
    HastyNoise::NoiseType type;
};

std::vector<NoiseInfo> Noises=
{
    {"Value", HastyNoise::NoiseType::Value},
    {"ValueFractal", HastyNoise::NoiseType::ValueFractal},
    {"Perlin", HastyNoise::NoiseType::Perlin},
    {"PerlinFractal", HastyNoise::NoiseType::PerlinFractal},
    {"Simplex", HastyNoise::NoiseType::Simplex},
    {"SimplexFractal", HastyNoise::NoiseType::SimplexFractal},
    {"WhiteNoise", HastyNoise::NoiseType::WhiteNoise},
    {"Cellular", HastyNoise::NoiseType::Cellular},
    {"Cubic", HastyNoise::NoiseType::Cubic},
    {"CubicFractal", HastyNoise::NoiseType::CubicFractal}
};

struct SIMDInfo
{
    SIMDInfo(std::string name, HastyNoise::SIMDType type):name(name), type(type) {}

    std::string name;
    HastyNoise::SIMDType type;
};

std::vector<SIMDInfo> SIMDNames=
{
    {"NONE", HastyNoise::SIMDType::None},
    {"NEON", HastyNoise::SIMDType::Neon},
    {"SSE2", HastyNoise::SIMDType::SSE2},
    {"SSE41", HastyNoise::SIMDType::SSE4_1},
    {"AVX2", HastyNoise::SIMDType::AVX2},
    {"AVX512", HastyNoise::SIMDType::AVX512}
};

std::string getSimdName(HastyNoise::SIMDType type)
{
    for(SIMDInfo &info:SIMDNames)
        if(info.type==type)
            return info.name;
    return std::string("Unknown");
}

void loadNoise(std::string &fileName, float *data, size_t x, size_t y, size_t z)
{
    FILE *file=fopen(fileName.c_str(), "rb");

    if(!file)
        return;

    size_t size=x*y*z;

    fread(data, sizeof(float), size, file);
    fclose(file);
}

void saveNoise(std::string &fileName, float *data, size_t x, size_t y, size_t z)
{
    FILE *file=fopen(fileName.c_str(), "wb");

    if(!file)
        return;

    size_t size=x*y*z;

    fwrite(data, sizeof(float), size, file);
    fclose(file);
}

bool match(float *data0, float *data1, size_t x, size_t y, size_t z, double &error)
{
    error=0.0;
    size_t size=x*y*z;

    float delta;

    for(size_t i=0; i<size; i++)
    {
        delta=std::abs(data0[i]-data1[i]);

        if(delta>std::numeric_limits<float>::epsilon())
            error+=delta;
    }

    if(error==0.0)
        return true;
    return false;
}

int xSize=64;
int ySize=64;
int zSize=64;

void generate(size_t highestLevel=0)
{
#if HN_USE_FILESYSTEM == 0
    assert(false);
#else
    size_t maxLevel=HastyNoise::GetFastestSIMD();
 
    if(highestLevel>=0)
        maxLevel=std::min(maxLevel, highestLevel);

    fs::path dataDir("./data");

    if(!fs::exists(dataDir))
        fs::create_directory(dataDir);

    std::string fileName;
    
    for(size_t i=maxLevel+1; i>0; --i)
    {
        size_t simdLevel=(size_t)i-1;

        //skip neon
        if(simdLevel==(size_t)HastyNoise::SIMDType::Neon)
            continue;

        std::cout<<"SIMD: "<<getSimdName((HastyNoise::SIMDType)simdLevel)<<" --------------------------------------------------------\n";

        auto noise=HastyNoise::CreateNoise(1337, simdLevel);

        if(noise->GetSIMDLevel()!=simdLevel)
        {
            std::cout<<"Failed to load: "<<getSimdName((HastyNoise::SIMDType)simdLevel)<<"\n";
            continue;
        }
        
        auto noiseSet=noise->GetEmptySet(xSize, ySize, zSize);

        for(auto &info:Noises)
        {
            std::cout<<"    "<<info.name;

            noise->SetNoiseType(info.type);
            noise->FillSet(noiseSet.get(), 0, 0, 0, xSize, ySize, zSize);

            fileName=dataDir.string()+"/"+info.name+"_"+getSimdName((HastyNoise::SIMDType)simdLevel)+".ns";
            saveNoise(fileName, noiseSet.get(), xSize, ySize, zSize);

            std::cout<<"  complete\n";
        }
    }
#endif
}

void checkGenerated()
{
#if HN_USE_FILESYSTEM == 0
    assert(false);
#else
    fs::path dataDir("./data");

    if(!fs::exists(dataDir))
    {
        std::cout<<"No data folder found\n";
        return;
    }

    for(NoiseInfo &noiseInfo:Noises)
    {
        std::vector<HastyNoise::FloatBuffer> noiseSets(SIMDNames.size());
        size_t index=0;

        for(SIMDInfo &simdInfo:SIMDNames)
        {
            std::string fileName=dataDir.string()+"/"+noiseInfo.name+"_"+simdInfo.name+".ns";
            fs::path filePath(fileName);

            if(!fs::exists(filePath))
            {
                index++;
                continue;
            }

            noiseSets[index]=HastyNoise::GetEmptySet(xSize, ySize, zSize, (size_t)simdInfo.type);
            loadNoise(fileName, noiseSets[index].get(), xSize, ySize, zSize);

            index++;
        }

        std::cout<<"Noise: "<<noiseInfo.name<<" --------------------------------------------------------\n";

        for(size_t i=0; i<SIMDNames.size(); i++)
        {
            if(!noiseSets[i])
                continue;

            for(size_t j=i+1; j<SIMDNames.size(); j++)
            {
                if(!noiseSets[j])
                    continue;

                double error;

                if(match(noiseSets[i].get(), noiseSets[j].get(), xSize, ySize, zSize, error))
                    std::cout<<"  "<<SIMDNames[i].name<<" <--> "<<SIMDNames[j].name<<": matched\n";
                else
                    std::cout<<"  "<<SIMDNames[i].name<<" <--> "<<SIMDNames[j].name<<": failed err "<<error<<"\n";
            }
        }
    }
#endif
}


void testPerformance()
{
    size_t maxLevel=HastyNoise::GetFastestSIMD();
    std::vector<HastyNoise::FloatBuffer> noiseSets(HastyNoise::SIMDTypeCount);

    for(auto &info:Noises)
    {
        std::cout<<info.name<<" --------------------------------------------------------\n";

        for(size_t i=maxLevel+1; i>0; --i)
        {
            size_t simdLevel=(size_t)i-1;

            //skip neon
            if(simdLevel==(size_t)HastyNoise::SIMDType::Neon)
                continue;

            auto noise=HastyNoise::CreateNoise(1337, simdLevel);

            if(noise->GetSIMDLevel()!=simdLevel)
            {
                std::cout<<"Failed to load: "<<getSimdName((HastyNoise::SIMDType)simdLevel)<<"\n";
                continue;
            }

            noise->SetNoiseType(info.type);

            if(!noiseSets[simdLevel])
                noiseSets[simdLevel]=HastyNoise::GetEmptySet(xSize, ySize, zSize, simdLevel);

            float *noiseSet=noiseSets[simdLevel].get();

            chrono::high_resolution_clock::time_point startTime;
            chrono::high_resolution_clock::time_point endTime;

            startTime=chrono::high_resolution_clock::now();

            for(int j=0; j<100; ++j)
                noise->FillSet(noiseSet, j*xSize, j*ySize, j*zSize, xSize, ySize, zSize);

            endTime=chrono::high_resolution_clock::now();
            double elapsed=chrono::duration_cast<chrono::milliseconds>(endTime-startTime).count();

            std::cout<<"    "<<getSimdName((HastyNoise::SIMDType)simdLevel)<<" "<<elapsed<<"ms\n";
        }
    }

    std::cout<<"\nPress Enter\n";
    getchar();
}

int main(int argc, char ** argv)
{
    HastyNoise::loadSimd("./");
    
    int maxLevel=-1;
    bool getMaxLevel=false;
    bool runGenerate=false;
    bool runCheckGenerated=false;
    bool runPerformance=false;

    for(int i=1; i<argc; ++i)
    {
        if(strcmp(argv[i], "-g")==0)
            runGenerate=true;
        if(strcmp(argv[i], "-c")==0)
            runCheckGenerated=true;
        else if(strcmp(argv[i], "-p")==0)
            runPerformance=true;
        else if(strcmp(argv[i], "-m")==0)
            getMaxLevel=true;
        else if(getMaxLevel)
            maxLevel=atoi(argv[i]);
        else
        {
            getMaxLevel=false;
        }
    }

    if(runGenerate)
        generate(maxLevel);
    if(runCheckGenerated)
        checkGenerated();
    if(runPerformance)
        testPerformance();
    return 0;
}
