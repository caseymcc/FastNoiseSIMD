#include "FastNoiseSIMD/FastNoiseSIMD.h"

#ifdef _MSC_VER
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

std::vector<std::string> SIMDName=
{
    {"NONE"},
    {"SSE2"},
    {"SSE41"},
    {"AVX2"},
    {"AVX512"},
    {"NEON"},
};

void saveNoise(std::string &fileName, float *data, size_t x, size_t y, size_t z)
{
    FILE *file=fopen(fileName.c_str(), "w");

    if(!file)
        return;

    size_t size=x*y*z;

    fwrite(data, sizeof(float), size, file);
    fclose(file);
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

            fileName=dataDir.string()+"/"+info.name+"_"+SIMDName[i]+".ns";
            saveNoise(fileName, noiseSet, xSize, ySize, zSize);
        }

        delete noise;
    }
}

int main(int argc, char ** argv)
{
    fs::path testDir="./";

    generate();
    return 0;
}
