// HastyNoise.h
//
// MIT License
//
// Copyright(c) 2017 Jordan Peck
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// The developer's email is jorzixdan.me2@gzixmail.com (for great email, take
// off every 'zix'.)
//

#ifndef HASTYNOISE_H
#define HASTYNOISE_H

#include "hastyNoise_export.h"
#include "hastyNoise_enums.h"

#include <memory>
#include <vector>
#include <string>
#include <functional>

namespace HastyNoise
{

#if defined(__arm__) || defined(__aarch64__)
#   define HN_ARM
//#define HN_IOS
#   define HN_COMPILE_NEON
#   define HN_COMPILE_NO_SIMD_FALLBACK
#else

#ifdef _MSC_VER
#   if(_M_X64  == 100)
#       define HN_COMPILE_SSE2
#       define HN_COMPILE_SSE41
#       ifdef __AVX2__
#           define HN_COMPILE_AVX2
#       endif
#       define HN_COMPILE_NO_SIMD_FALLBACK
#   endif
#else
#   ifdef __SSE2__
#       define HN_COMPILE_SSE2
#   endif
#   ifdef __SSE4_1__
#       define HN_COMPILE_SSE41
#   endif
#   ifdef __AVX2__
#       define HN_COMPILE_AVX2
#   endif
#   ifdef __AVX512F__
#       define HN_COMPILE_AVX512
#   endif
#endif

// Using FMA instructions with AVX(51)2/NEON provides a small performance increase but can cause 
// minute variations in noise output compared to other SIMD levels due to higher calculation precision
// Intel compiler will always generate FMA instructions, use /Qfma- or -no-fma to disable
#define HN_USE_FMA
#endif

#define HN_CELLULAR_INDEX_MAX 3

// Using aligned sets of memory for float arrays allows faster storing of SIMD data
// Comment out to allow unaligned float arrays to be used as sets
#define HN_ALIGNED_SETS

/*
Tested Compilers:
-MSVC v140/v150
-GCC 7 Linux
-Clang MacOSX

CPU instruction support:

SSE2
Intel Pentium 4 - 2001
AMD Opteron/Athlon - 2003

SEE4.1
Intel Penryn - 2007
AMD Bulldozer - Q4 2011

AVX
Intel Sandy Bridge - Q1 2011
AMD Bulldozer - Q4 2011

AVX2
Intel Haswell - Q2 2013
AMD Carrizo - Q2 2015

FMA3
Intel Haswell - Q2 2013
AMD Piledriver - 2012

AVX-512F
Intel Skylake-X - Q2 2017
*/
//enum class NoiseType { None, Value, ValueFractal, Perlin, PerlinFractal, Simplex, SimplexFractal, WhiteNoise, Cellular, Cubic, CubicFractal };
//
//struct NoiseInfo
//{
//    NoiseInfo(std::string name, HastyNoise::NoiseType type):name(name), type(type) {}
//
//    std::string name;
//    HastyNoise::NoiseType type;
//};
//
//static std::vector<NoiseInfo> NoiseNames=
//{
//    {"Value", HastyNoise::NoiseType::Value},
//    {"ValueFractal", HastyNoise::NoiseType::ValueFractal},
//    {"Perlin", HastyNoise::NoiseType::Perlin},
//    {"PerlinFractal", HastyNoise::NoiseType::PerlinFractal},
//    {"Simplex", HastyNoise::NoiseType::Simplex},
//    {"SimplexFractal", HastyNoise::NoiseType::SimplexFractal},
//    {"WhiteNoise", HastyNoise::NoiseType::WhiteNoise},
//    {"Cellular", HastyNoise::NoiseType::Cellular},
//    {"Cubic", HastyNoise::NoiseType::Cubic},
//    {"CubicFractal", HastyNoise::NoiseType::CubicFractal}
//};
//
//inline std::string getNoiseName(HastyNoise::NoiseType type)
//{
//    for(NoiseInfo &info:NoiseNames)
//        if(info.type==type)
//            return info.name;
//    return std::string("Unknown");
//}

//enum class FractalType { None, FBM, Billow, RigidMulti };
//enum class PerturbType { None, Gradient, GradientFractal, Normalise, Gradient_Normalise, GradientFractal_Normalise };
//enum class CellularDistance { None, Euclidean, Manhattan, Natural };
//enum class CellularReturnType { None, Value, Distance, Distance2, Distance2Add, Distance2Sub, Distance2Mul, Distance2Div, NoiseLookup, Distance2Cave };

//constexpr size_t SIMDTypeCount=6;
//enum class SIMDType { None=0, Neon=1, SSE2=2, SSE4_1=3, AVX2=4, AVX512=5 };
//
//struct SIMDInfo
//{
//    SIMDInfo(std::string name, HastyNoise::SIMDType type):name(name), type(type) {}
//
//    std::string name;
//    HastyNoise::SIMDType type;
//};
//
//static std::vector<SIMDInfo> SIMDNames=
//{
//    {"None", HastyNoise::SIMDType::None},
//    {"Neon", HastyNoise::SIMDType::Neon},
//    {"SSE2", HastyNoise::SIMDType::SSE2},
//    {"SSE41", HastyNoise::SIMDType::SSE4_1},
//    {"AVX2", HastyNoise::SIMDType::AVX2},
//    {"AVX512", HastyNoise::SIMDType::AVX512}
//};
//
//inline std::string getSimdName(HastyNoise::SIMDType type)
//{
//    for(SIMDInfo &info:SIMDNames)
//        if(info.type==type)
//            return info.name;
//    return std::string("Unknown");
//}
//
//enum class NoiseClass {Single, Fractal, Cellular};
//enum class BuildType {Default, Map, Vector };

struct HASTYNOISE_EXPORT NoiseDetails
{
    NoiseDetails():
        seed(1337),
        frequency(0.01f),

        xScale(1.0f),
        yScale(1.0f),
        zScale(1.0f),

        octaves(3),
        lacunarity(2.0f),
        gain(0.5f),

        cellularNoiseLookupFrequency(0.2f),
        cellularDistanceIndex0(0),
        cellularDistanceIndex1(1),
        cellularJitter(0.45f)
     {}

    int  seed;
    float  frequency;

    float  xScale;
    float  yScale;
    float  zScale;

    int  octaves;
    float  lacunarity;
    float  gain;
    float fractalBounding;

    float  cellularNoiseLookupFrequency;
    int  cellularDistanceIndex0;
    int  cellularDistanceIndex1;
    float  cellularJitter;
};

struct HASTYNOISE_EXPORT PerturbDetails
{
    PerturbDetails():
        Amp(1.0f),
        Frequency(0.5f),
        Octaves(3),
        Lacunarity(2.0f),
        Gain(0.5f),
        NormaliseLength(1.0f)
    {}

    float Amp;
    float Frequency;

    int Octaves;
    float Lacunarity;
    float Gain;
    float FractalBounding;
    float NormaliseLength;
};

#if defined(_WIN32)
#define VECTORCALL __vectorcall
#else
#define VECTORCALL
#endif

class NoiseSIMD;
typedef NoiseSIMD *(*NewNoiseSimdFunc)(int);
typedef size_t(*AlignedSizeFunc)(size_t);
typedef float *(*GetEmptySetFunc)(size_t);

struct HASTYNOISE_EXPORT NoiseFuncs
{
    NoiseFuncs():createFunc(nullptr), alignedSizeFunc(nullptr), getEmptySetFunc(nullptr) {}

    NewNoiseSimdFunc createFunc;
    AlignedSizeFunc alignedSizeFunc;
    GetEmptySetFunc getEmptySetFunc;
};


// Loads all available simd libraries from directory
HASTYNOISE_EXPORT bool loadSimd(std::string directory);


namespace details
{
//Creates Noise class at SIMD level
HASTYNOISE_EXPORT NoiseSIMD *CreateNoise(int seed=1337, size_t simdLevel=-1);

// Free a noise set from memory
HASTYNOISE_EXPORT void FreeNoiseSet(float *noiseSet, size_t simdLevel);

// Create an empty (aligned) noise set for use with FillNoiseSet()
HASTYNOISE_EXPORT float *GetEmptySet(size_t size, size_t simdLevel);

// Create an empty (aligned) noise set for use with FillNoiseSet()
inline float *GetEmptySet(size_t xSize, size_t ySize, size_t zSize, size_t simdLevel) { return GetEmptySet(xSize*ySize*zSize, simdLevel); }

}//namespace details

// Returns highest detected level of CPU support
// 5: AVX-512F
// 4: AVX2 & FMA3
// 3: SSE4.1
// 2: SSE2
// 1: ARM NEON
// 0: Fallback, no SIMD support
//size_t GetSIMDLevel(void);
HASTYNOISE_EXPORT size_t GetFastestSIMD();

HASTYNOISE_EXPORT bool SupportedSimd(SIMDType type);

// Creates new NoiseSIMD for the highest supported instuction set of the CPU 
// 5: AVX-512F
// 4: AVX2 & FMA3
// 3: SSE4.1
// 2: SSE2
// 1: ARM NEON
// 0: Fallback, no SIMD support
// -1: Auto-detect fastest supported (Default)
// Caution: Setting this manually can cause crashes on CPUs that do not support that level
// Caution: Changing this after creating NoiseSIMD objects has undefined behaviour
inline std::unique_ptr<NoiseSIMD> CreateNoise(int seed=1337, size_t simdLevel=std::numeric_limits<size_t>::max())
{
    return std::unique_ptr<NoiseSIMD>(details::CreateNoise(seed, simdLevel));
}

struct SetDeleter
{
    SetDeleter():level(std::numeric_limits<size_t>::max()) {}
    SetDeleter(size_t level):level(level) {}

    void operator()(float *ptr) const { if(level>=SIMDTypeCount) return;  details::FreeNoiseSet(ptr, level); }
    size_t level;
};
typedef std::unique_ptr<float, SetDeleter> FloatBuffer;
// Create an empty (aligned) noise set for use with FillNoiseSet()
inline FloatBuffer GetEmptySet(size_t size, size_t simdLevel)
{
    float *ptr=details::GetEmptySet(size, simdLevel);

    return FloatBuffer{ptr, SetDeleter(simdLevel)};
}

// Create an empty (aligned) noise set for use with FillNoiseSet()
inline FloatBuffer GetEmptySet(size_t xSize, size_t ySize, size_t zSize, size_t simdLevel)
{ 
    return GetEmptySet(xSize*ySize*zSize, simdLevel); 
}

// Rounds the size up to the nearest aligned size for the current SIMD level
HASTYNOISE_EXPORT size_t AlignedSize(size_t size, size_t simdLevel);

struct VectorSet
{
public:
    size_t size=-1;
    size_t simdLevel;
    FloatBuffer buffer;
    float *xSet=nullptr;
    float *ySet=nullptr;
    float *zSet=nullptr;

    // Only used for sampled vector sets
    int sampleScale=0;
    int sampleSizeX=-1;
    int sampleSizeY=-1;
    int sampleSizeZ=-1;

    VectorSet(size_t simdLevel):simdLevel(simdLevel), buffer(nullptr, SetDeleter(simdLevel)) {}

    VectorSet(size_t simdLevel, size_t _size):simdLevel(simdLevel), buffer(nullptr, SetDeleter(simdLevel)) { SetSize(_size); }

    ~VectorSet() {}

    void Free()
    {
        size=-1;
        buffer.reset();
        xSet=nullptr;
        ySet=nullptr;
        zSet=nullptr;
    }

    void SetSize(size_t _size)
    {
        Free();
        size=_size;

        size_t alignedSize=AlignedSize(size, simdLevel);

        buffer=GetEmptySet(alignedSize*3, simdLevel);
        xSet=buffer.get();
        ySet=xSet+alignedSize;
        zSet=ySet+alignedSize;
    }

};

//Fills VectorSet with incrementing values
HASTYNOISE_EXPORT void FillVectorSet(VectorSet* vectorSet, int xSize, int ySize, int zSize);

//Fills VectorSet with incrementing values using sampleScale
HASTYNOISE_EXPORT void FillSamplingVectorSet(VectorSet* vectorSet, int sampleScale, int xSize, int ySize, int zSize);

//Gets a VectorSet that works with simdLevel
inline std::unique_ptr<VectorSet> GetVectorSet(int xSize, int ySize, int zSize, size_t simdLevel)
{
    std::unique_ptr<VectorSet> vectorSet(new VectorSet(simdLevel));
    FillVectorSet(vectorSet.get(), xSize, ySize, zSize);
    return vectorSet;
}

//Gets a VectorSet that works with simdLevel using sampleScale
inline std::unique_ptr<VectorSet> GetSamplingVectorSet(int sampleScale, int xSize, int ySize, int zSize, size_t simdLevel)
{
    std::unique_ptr<VectorSet> vectorSet(new VectorSet(simdLevel));
    FillSamplingVectorSet(vectorSet.get(), sampleScale, xSize, ySize, zSize);
    return vectorSet;
}

//Internal, registers NoiseSIMD classes
HASTYNOISE_EXPORT bool registerNoiseSimd(SIMDType type, NewNoiseSimdFunc createFunc, AlignedSizeFunc alignedSizeFunc, GetEmptySetFunc getEmptySetFunc);

HASTYNOISE_EXPORT float CalculateFractalBounding(int octaves, float gain);

class HASTYNOISE_EXPORT NoiseSIMD
{
public:
    virtual ~NoiseSIMD() {}

    // Returns SIMD level that class was created with
    size_t GetSIMDLevel(void) { return m_SIMDLevel; }

    // Returns seed used for all noise types
    int GetSeed(void) const { return m_noiseDetails.seed; }

    // Sets seed used for all noise types
    // Default: 1337
    void SetSeed(int seed) { m_noiseDetails.seed=seed; }

    // Sets frequency for all noise types
    // Default: 0.01
    void SetFrequency(float frequency) { m_noiseDetails.frequency=frequency; }

    //	// Sets noise return type of (Get/Fill)NoiseSet()
    //	// Default: Simplex
    void SetNoiseType(NoiseType noiseType) { m_noiseType = noiseType; }

        // Sets scaling factor for individual axis
        // Defaults: 1.0
    void SetAxisScales(float xScale, float yScale, float zScale) { m_noiseDetails.xScale=xScale; m_noiseDetails.yScale=yScale; m_noiseDetails.zScale=zScale; }


    //	// Sets fractal type for noise that supports it
    //	// Default: FBM
    void SetFractalType(FractalType fractalType) { m_fractalType=fractalType; }

    // Sets octave count for all fractal noise types
    // Default: 3
    void SetFractalOctaves(int octaves) { m_noiseDetails.octaves=octaves; m_noiseDetails.fractalBounding=CalculateFractalBounding(m_noiseDetails.octaves, m_noiseDetails.gain); }

    // Sets octave lacunarity for all fractal noise types
    // Default: 2.0
    void SetFractalLacunarity(float lacunarity) { m_noiseDetails.lacunarity=lacunarity; }

    // Sets octave gain for all fractal noise types
    // Default: 0.5
    void SetFractalGain(float gain) { m_noiseDetails.gain=gain; m_noiseDetails.fractalBounding=CalculateFractalBounding(m_noiseDetails.octaves, m_noiseDetails.gain); }

    // Sets distance function used in cellular noise calculations
    // Default: Euclidean
    void SetCellularDistanceFunction(CellularDistance cellularDistance) { m_cellularDistance=cellularDistance; }

    // Sets return type from cellular noise calculations
    // Default: Distance
    void SetCellularReturnType(CellularReturnType cellularReturnType) { m_cellularReturnType=cellularReturnType; }

    // Sets relative frequency on the cellular noise lookup return type
    // Default: 0.2
    void SetCellularNoiseLookupFrequency(float cellularNoiseLookupFrequency) { m_noiseDetails.cellularNoiseLookupFrequency=cellularNoiseLookupFrequency; }

    // Sets the 2 distance indicies used for distance2 return types
    // Default: 0, 1
    // Note: index0 should be lower than index1
    // Both indicies must be >= 0, index1 must be < 4
    void SetCellularDistance2Indicies(int cellularDistanceIndex0, int cellularDistanceIndex1);

    // Sets the maximum distance a cellular point can move from it's grid position
    // Setting this high will make artifacts more common
    // Default: 0.45
    void SetCellularJitter(float cellularJitter) { m_noiseDetails.cellularJitter=cellularJitter; }

    //	// Enables position perturbing for all noise types
    //	// Default: None
    //	void SetPerturbType(PerturbType perturbType) { m_perturbType = perturbType; }

        // Sets the maximum distance the input position can be perturbed
        // Default: 1.0
    void SetPerturbAmp(float perturbAmp) { m_perturbDetails.Amp=perturbAmp/511.5f; }

    // Set the relative frequency for the perturb gradient
    // Default: 0.5
    void SetPerturbFrequency(float perturbFrequency) { m_perturbDetails.Frequency=perturbFrequency; }


    // Sets octave count for perturb fractal types
    // Default: 3
    void SetPerturbFractalOctaves(int perturbOctaves) { m_perturbDetails.Octaves=perturbOctaves; m_perturbDetails.FractalBounding=CalculateFractalBounding(m_perturbDetails.Octaves, m_perturbDetails.Gain); }

    // Sets octave lacunarity for perturb fractal types 
    // Default: 2.0
    void SetPerturbFractalLacunarity(float perturbLacunarity) { m_perturbDetails.Lacunarity=perturbLacunarity; }

    // Sets octave gain for perturb fractal types 
    // Default: 0.5
    void SetPerturbFractalGain(float perturbGain) { m_perturbDetails.Gain=perturbGain; m_perturbDetails.FractalBounding=CalculateFractalBounding(m_perturbDetails.Octaves, m_perturbDetails.Gain); }

    // Sets the length for vectors after perturb normalising 
    // Default: 1.0
    void SetPerturbNormaliseLength(float perturbNormaliseLength) { m_perturbDetails.NormaliseLength=perturbNormaliseLength; }

    // Gets float buffer compatiable with the classes SIMD level
    FloatBuffer GetEmptySet(size_t xSize, size_t ySize, size_t zSize)
    {
        return HastyNoise::GetEmptySet(xSize*ySize*zSize, m_SIMDLevel);
    }

    // Gets float buffer compatiable with the classes SIMD level, and fills it
    FloatBuffer GetNoiseSet(int xStart, int yStart, int zStart, int xSize, int ySize, int zSize, float scaleModifier=1.0f)
    {
        FloatBuffer noiseSet=HastyNoise::GetEmptySet(xSize*ySize*zSize, m_SIMDLevel);

        FillSet(noiseSet.get(), xStart, yStart, zStart, xSize, ySize, zSize, scaleModifier);

        return noiseSet;
    }

    // Fills float buffer with noise selected in the class starting from nStart, with nSize
    virtual void FillSet(float* noiseSet, int xStart, int yStart, int zStart, int xSize, int ySize, int zSize, float scaleModifier=1.0f);

    // Fills float buffer with noise selected in the class using VectorSet as sample positions
    virtual void FillSet(float* noiseSet, VectorSet* vectorSet, float xOffset=0.0f, float yOffset=0.0f, float zOffset=0.0f);

    // Gets VectorSet compatiable with the classes SIMD level
    std::unique_ptr<VectorSet> GetVectorSet(int xSize, int ySize, int zSize)
    {
        return HastyNoise::GetVectorSet(xSize, ySize, zSize, m_SIMDLevel);
    }

    // Gets VectorSet compatiable with the classes SIMD level, using sampleScale
    std::unique_ptr<VectorSet> GetSamplingVectorSet(int sampleScale, int xSize, int ySize, int zSize)
    {
        return HastyNoise::GetSamplingVectorSet(sampleScale, xSize, ySize, zSize, m_SIMDLevel);
    }

protected:
    NoiseDetails m_noiseDetails;
    NoiseType m_noiseType=NoiseType::SimplexFractal;
    FractalType m_fractalType=FractalType::FBM;

    CellularDistance m_cellularDistance=CellularDistance::Euclidean;
    CellularReturnType m_cellularReturnType=CellularReturnType::Distance;
    NoiseType m_cellularNoiseLookupType=NoiseType::Simplex;

    PerturbType m_perturbType=PerturbType::None;
    PerturbDetails m_perturbDetails;

    size_t m_SIMDLevel;
};

}//namespace HastyNoise

#endif
