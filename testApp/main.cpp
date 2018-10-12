#include "FastNoiseSIMD/FastNoiseSIMD.h"
#include <unordered_map>

#ifdef _MSC_VER
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#   if _MSC_VER > 1900
#include <filesystem>
namespace fs=std::filesystem;
#   elif _MSC_VER == 1900
#include <experimental/filesystem>
namespace fs=std::experimental::filesystem::v1;
#   else
#include <boost/filesystem>
namespace fs=std::filesystem;
#   endif
#else
#include <filesystem>
namespace fs=std::filesystem;
#endif

#include <sstream>
#include <iostream>

struct NoiseInfo
{
    NoiseInfo(std::string name, FastNoiseSIMD::NoiseType type):name(name), type(type){}

    std::string name;
    FastNoiseSIMD::NoiseType type;
};

//enum NoiseType { Value, ValueFractal, Perlin, PerlinFractal, Simplex, SimplexFractal, WhiteNoise, Cellular, Cubic, CubicFractal };
std::vector<NoiseInfo> Noises=
{
    {"Value", FastNoiseSIMD::Value},
    {"ValueFractal", FastNoiseSIMD::ValueFractal},
    {"Perlin", FastNoiseSIMD::Perlin},
    {"PerlinFractal", FastNoiseSIMD::PerlinFractal},
    {"Simplex", FastNoiseSIMD::Simplex},
    {"SimplexFractal", FastNoiseSIMD::SimplexFractal},
    {"WhiteNoise", FastNoiseSIMD::WhiteNoise},
    {"Cellular", FastNoiseSIMD::Cellular},
    {"Cubic", FastNoiseSIMD::Cubic},
    {"CubicFractal", FastNoiseSIMD::CubicFractal}
};

std::vector<std::string> SIMDNames=
{
    {"NONE"},
    {"SSE2"},
    {"SSE41"},
    {"AVX2"},
    {"AVX512"},
    {"NEON"}
};

void loadNoise(std::string &fileName, float *data, size_t x, size_t y, size_t z)
{
    FILE *file=fopen(fileName.c_str(), "r");

    if(!file)
        return;

    size_t size=x*y*z;

    fread(data, sizeof(float), size, file);
    fclose(file);
}

void saveNoise(std::string &fileName, float *data, size_t x, size_t y, size_t z)
{
    FILE *file=fopen(fileName.c_str(), "w");

    if(!file)
        return;

    size_t size=x*y*z;

    fwrite(data, sizeof(float), size, file);
    fclose(file);
}

float calculateRMS(float *data1, float *data2, size_t x, size_t y, size_t z)
{
    double accumulator=0.0;
    float rms;
    size_t size=x*y*z;
    double delta;

    for(size_t i=0; i<size; i++)
    {
        delta=(double)(data1[i]-data2[i]);
        if(isnan(delta)||isinf(delta))
            delta=0.0;
        accumulator+=(delta*delta);
        if(isnan(accumulator) || isinf(accumulator))
            accumulator=0.0;
    }

    accumulator=sqrt(accumulator/(double)size);
    return accumulator;
}

void generate()
{
    int maxLevel=FastNoiseSIMD::GetSIMDLevel();
    int xSize=64;
    int ySize=64;
    int zSize=64;

    fs::path dataDir("./data");

    if(!fs::exists(dataDir))
        fs::create_directory(dataDir);

    std::string fileName;
    maxLevel=1;

    //skip neon
    for(int i=maxLevel; i>=0; --i)
    {
        FastNoiseSIMD::SetSIMDLevel(i);
        float* noiseSet=FastNoiseSIMD::GetEmptySet(xSize, ySize, zSize);
        FastNoiseSIMD *noise=FastNoiseSIMD::NewFastNoiseSIMD();

        for(auto &info:Noises)
        {
            noise->SetNoiseType(info.type);
            noise->FillNoiseSet(noiseSet, 0, 0, 0, xSize, ySize, zSize);

            fileName=dataDir.string()+"/"+info.name+"_"+SIMDNames[i]+".ns";
            saveNoise(fileName, noiseSet, xSize, ySize, zSize);
        }

        delete noise;
    }
}

void testGeneratedFiles()
{
    int xSize=64;
    int ySize=64;
    int zSize=64;

    typedef std::unordered_map<std::string, std::string> SimdMap;
    typedef std::unordered_map<std::string, SimdMap> NoiseMap;

    NoiseMap noiseMap;
    fs::path dataDir("./data");

    if(!fs::exists(dataDir))
    {
        std::cout<<"Directory "<<dataDir<<" does not exist.\n";
        return;
    }

    fs::directory_iterator iter(dataDir);
    fs::directory_iterator endIter;

    while(iter!=endIter)
    {
        fs::path filePath=iter->path();
        std::string fileName=filePath.filename().string();
        std::string extension=filePath.extension().string();

        ++iter;

        if(!fs::is_regular_file(filePath))
            continue;
        
        if(extension!=".ns")
            continue;

        std::istringstream tFileName(fileName);
        std::string noiseType;
        std::string simdType;
        
        if(!std::getline(tFileName, noiseType, '_'))
            continue;
        if(!std::getline(tFileName, simdType, '.'))
            continue;

        noiseMap[noiseType][simdType]=filePath.string();
    }

    std::vector<float *> noiseSets(6, nullptr);
    for(auto noiseIter:noiseMap)
    {
        std::string noiseType=noiseIter.first;
        SimdMap &simdMap=noiseIter.second;

        std::cout<<noiseType<<" --------------------------------------------------------\n";
        bool first=true;
        float *firstSet;
        std::string firstName;
        for(size_t i=0; i<SIMDNames.size();i++)
        {
            auto simdIter=simdMap.find(SIMDNames[i]);

            if(simdIter==simdMap.end())
                continue;

            std::string simdType=simdIter->first;
            std::string &noisePath=simdIter->second;

            FastNoiseSIMD::SetSIMDLevel(i);
            
            if(!noiseSets[i])
                noiseSets[i]=FastNoiseSIMD::GetEmptySet(xSize, ySize, zSize);

            if(first)
            {
                firstSet=noiseSets[i];

                loadNoise(noisePath, firstSet, xSize, ySize, zSize);
                first=false;
                firstName=simdType;
                continue;
            }

            float *noiseSet=noiseSets[i];

            loadNoise(noisePath, noiseSet, xSize, ySize, zSize);

            float rms=calculateRMS(firstSet, noiseSet, xSize, ySize, zSize);

            std::cout<<"    "<<firstName<<" <-> "<<simdType<<" :"<<rms<<"\n";
        }
    }

    std::cout<<"\nPress Enter\n";
    getchar();
}

#ifdef _MSC_VER
void testPerformance()
{
    int maxLevel=FastNoiseSIMD::GetSIMDLevel();
    int xSize=64;
    int ySize=64;
    int zSize=64;

    //    maxLevel=2;
    fs::path dataDir("./data");

    if(!fs::exists(dataDir))
        fs::create_directory(dataDir);

    std::string fileName;
    //skip neon
    std::vector<float *> noiseSets(6, nullptr);


    for(auto &info:Noises)
    {
        std::cout<<info.name<<" --------------------------------------------------------\n";

        for(int i=maxLevel; i>=0; --i)
        {
            FastNoiseSIMD::SetSIMDLevel(i);

            if(!noiseSets[i])
                noiseSets[i]=FastNoiseSIMD::GetEmptySet(xSize, ySize, zSize);

            float *noiseSet=noiseSets[i];
            FastNoiseSIMD *noise=FastNoiseSIMD::NewFastNoiseSIMD();

            noise->SetNoiseType(info.type);

            ULONGLONG start;
            ULONGLONG elapsed;

            QueryInterruptTime(&start);

            for(size_t j=0; j<100; ++j)
                noise->FillNoiseSet(noiseSet, 0, 0, 0, xSize, ySize, zSize);

            QueryInterruptTime(&elapsed);
            elapsed=(elapsed-start)/10000;

            std::cout<<"    "<<SIMDNames[i]<<" "<<elapsed<<"ms\n";
            delete noise;
        }
    }

    std::cout<<"\nPress Enter\n";
    getchar();
}
#endif

int main(int argc, char ** argv)
{
    fs::path testDir="./";

//    generate();
//    testGeneratedFiles();
    testPerformance();
    return 0;
}
