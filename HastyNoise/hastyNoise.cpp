// HastyNoise.cpp
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

#include "hastyNoise.h"
#include <assert.h>
#include <stdlib.h>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>

#if defined(_WIN32) || defined(_WIN64)
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

// CPUid
#ifdef _WIN32
#include <intrin.h>
#elif defined(HN_ARM)
#if !defined(__aarch64__) && !defined(HN_IOS)
#include "ARM/cpu-features.h"
#endif
#else
#include <cpuid.h>
#include "inttypes.h"
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


#include "simd_constants.inl"
#include "internal_none.inl"

namespace HastyNoise
{
static std::vector<NoiseFuncs> s_noiseSimds(SIMDTypeCount);

bool registerNoiseSimd(SIMDType type, NewNoiseSimdFunc createFunc, AlignedSizeFunc alignedSizeFunc, GetEmptySetFunc getEmptySetFunc)
{
    s_noiseSimds[(size_t)type].createFunc=createFunc;
    s_noiseSimds[(size_t)type].alignedSizeFunc=alignedSizeFunc;
    s_noiseSimds[(size_t)type].getEmptySetFunc=getEmptySetFunc;

    return true;
}

#ifdef HN_ARM
size_t _GetFastestSIMD()
{
#if defined(__aarch64__) || defined(HN_IOS)
    return (size_t)SIMDType::Neon;
#else
    if(android_getCpuFamily()==ANDROID_CPU_FAMILY_ARM)
    {
        auto cpuFeatures=android_getCpuFeatures();

        if(cpuFeatures & ANDROID_CPU_ARM_FEATURE_NEON)
#ifdef HN_USE_FMA
            if(cpuFeatures & ANDROID_CPU_ARM_FEATURE_NEON_FMA)
#endif
                return (size_t)SIMDType::Neon;
    }

    return (size_t)SIMDType::None;
#endif
}
#else

#ifdef _WIN32
void cpuid(int32_t out[4], int32_t x)
{
    __cpuidex(out, x, 0);
}
uint64_t xgetbv(unsigned int x)
{
    return _xgetbv(x);
}
#else
void cpuid(int32_t out[4], int32_t x)
{
    __cpuid_count(x, 0, out[0], out[1], out[2], out[3]);
}
uint64_t xgetbv(unsigned int index)
{
    uint32_t eax, edx;
    __asm__ __volatile__("xgetbv" : "=a"(eax), "=d"(edx):"c"(index));
    return ((uint64_t)edx<<32)|eax;
}
#define _XCR_XFEATURE_ENABLED_MASK  0
#endif

size_t _GetFastestSIMD()
{
    //https://github.com/Mysticial/FeatureDetector

    int cpuInfo[4];

    cpuid(cpuInfo, 0);
    int nIds=cpuInfo[0];

    if(nIds<0x00000001)
        return (size_t)SIMDType::None;

    cpuid(cpuInfo, 0x00000001);

    // SSE2
    if((cpuInfo[3]&1<<26)==0)
        return (size_t)SIMDType::None;

    // SSE41
    if((cpuInfo[2]&1<<19)==0)
        return (size_t)SIMDType::SSE2;

    // AVX
    bool cpuXSaveSuport=(cpuInfo[2]&1<<26)!=0;
    bool osAVXSuport=(cpuInfo[2]&1<<27)!=0;
    bool cpuAVXSuport=(cpuInfo[2]&1<<28)!=0;

    if(cpuXSaveSuport && osAVXSuport && cpuAVXSuport)
    {
        uint64_t xcrFeatureMask=xgetbv(_XCR_XFEATURE_ENABLED_MASK);
        if((xcrFeatureMask&0x6)!=0x6)
            return (size_t)SIMDType::SSE4_1;
    }
    else
        return (size_t)SIMDType::SSE4_1;

    // AVX2 FMA3
    if(nIds<0x00000007)
        return (size_t)SIMDType::SSE4_1;

#ifdef HN_USE_FMA
    bool cpuFMA3Support=(cpuInfo[2]&1<<12)!=0;
#else
    bool cpuFMA3Support=true;
#endif

    cpuid(cpuInfo, 0x00000007);

    bool cpuAVX2Support=(cpuInfo[1]&1<<5)!=0;

    if(!cpuFMA3Support||!cpuAVX2Support)
        return (size_t)SIMDType::SSE4_1;

    // AVX512
    bool cpuAVX512Support=(cpuInfo[1]&1<<16)!=0;
    bool oxAVX512Support=(xgetbv(_XCR_XFEATURE_ENABLED_MASK)&0xe6)==0xe6;

    if(!cpuAVX512Support||!oxAVX512Support)
        return (size_t)SIMDType::AVX2;

    return (size_t)SIMDType::AVX512;
}

size_t GetFastestSIMD()
{
    //find highest supported simd
    size_t simd=_GetFastestSIMD();

    //see which simd levels are loaded
    while(simd>0)
    {
        if(s_noiseSimds[simd].createFunc)
            return simd;

        simd--;
    }

    return simd; //we always have no simd loaded
}
#endif

bool SupportedSimd(SIMDType type)
{
    size_t level=(size_t)type;

    if((level<0)||(level>=SIMDTypeCount))
        return false;

    if(s_noiseSimds[level].createFunc)
        return true;

    return false;
}

size_t AlignedSize(size_t size, size_t simdLevel)
{
    if(simdLevel>=s_noiseSimds.size())
        return 0;

    if(!s_noiseSimds[simdLevel].alignedSizeFunc)
        return 0;

    return s_noiseSimds[simdLevel].alignedSizeFunc(size);
}



namespace details
{

NoiseSIMD *CreateNoise(int seed, size_t simdLevel)
{
    if(simdLevel>=s_noiseSimds.size())
    {
        simdLevel=GetFastestSIMD();
    }

    if(s_noiseSimds[simdLevel].createFunc)
        return s_noiseSimds[simdLevel].createFunc(seed);

    return s_noiseSimds[(size_t)SIMDType::None].createFunc(seed);
}

float *GetEmptySet(size_t size, size_t simdLevel)
{
    if(simdLevel>=s_noiseSimds.size())
        return nullptr;

    if(!s_noiseSimds[simdLevel].getEmptySetFunc)
        return nullptr;

    return s_noiseSimds[simdLevel].getEmptySetFunc(size);
}

void FreeNoiseSet(float *floatArray, size_t level)
{
#ifdef HN_ALIGNED_SETS

    if(level!=(size_t)SIMDType::None)
#ifdef _WIN32
        _aligned_free(floatArray);
#else
        free(floatArray);
#endif
    else
#endif
        delete[] floatArray;
}

}//namespace details

bool loadSimd(std::string libPath)
{
#if HN_USE_FILESYSTEM == 0
    assert(false);
    return false;
#else
    fs::path directory(libPath);

    if(!fs::exists(directory)||!fs::is_directory(directory))
        return false;

    std::string libExtension;

#if defined(_WIN32) || defined(_WIN64)
    libExtension=".dll";
#else
    libExtension=".so";
#endif
    fs::directory_iterator iter(directory);
    fs::directory_iterator endIter;

    while(iter!=endIter)
    {
        fs::path filePath=iter->path();
        std::string fileName=filePath.filename().string();
        std::string extension=filePath.extension().string();

        if(fs::is_regular_file(*iter) && extension==libExtension && fileName.compare(0, 11, "hastyNoise_")==0)
        {
#if defined(_WIN32) || defined(_WIN64)
            HINSTANCE instance=NULL;

            try
            { instance=LoadLibrary(filePath.string().c_str()); } //Plugins should autoregister
            catch(...)
            { instance=NULL; }

            if(instance==NULL)
            {
                LPVOID lpMsgBuf;
                DWORD error=GetLastError();

                FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER|
                    FORMAT_MESSAGE_FROM_SYSTEM|
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    error,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                    (LPTSTR)&lpMsgBuf,
                    0,
                    NULL
                );

                std::string message((char *)lpMsgBuf);
                std::cout<<"Failed to load "<<fileName<<" error: "<<message;
                LocalFree(lpMsgBuf);
            }
#else
            void *handle;
            
            dlerror();
            try
            { handle=dlopen(filePath.string().c_str(), RTLD_NOW); }
            catch(std::exception &except)
            { handle=NULL; }
            catch(...)
            { handle=NULL; }

            if(handle==NULL)
            {
                char *error=dlerror();

                std::cout<<"Failed to load "<<fileName<<" error: "<<error;
            }
#endif
        }

        ++iter;
    }

    return true;
#endif
}

void FillVectorSet(VectorSet* vectorSet, int xSize, int ySize, int zSize)
{
    assert(vectorSet);

    vectorSet->SetSize(xSize*ySize*zSize);
    vectorSet->sampleScale=0;

    int index=0;

    for(int ix=0; ix<xSize; ix++)
    {
        for(int iy=0; iy<ySize; iy++)
        {
            for(int iz=0; iz<zSize; iz++)
            {
                vectorSet->xSet[index]=float(ix);
                vectorSet->ySet[index]=float(iy);
                vectorSet->zSet[index]=float(iz);
                index++;
            }
        }
    }
}

void FillSamplingVectorSet(VectorSet* vectorSet, int sampleScale, int xSize, int ySize, int zSize)
{
    assert(vectorSet);

    if(sampleScale<=0)
    {
        FillVectorSet(vectorSet, xSize, ySize, zSize);
        return;
    }

    vectorSet->sampleSizeX=xSize;
    vectorSet->sampleSizeY=ySize;
    vectorSet->sampleSizeZ=zSize;

    int sampleSize=1<<sampleScale;
    int sampleMask=sampleSize-1;

    int xSizeSample=xSize;
    int ySizeSample=ySize;
    int zSizeSample=zSize;

    if(xSizeSample & sampleMask)
        xSizeSample=(xSizeSample & ~sampleMask)+sampleSize;

    if(ySizeSample & sampleMask)
        ySizeSample=(ySizeSample & ~sampleMask)+sampleSize;

    if(zSizeSample & sampleMask)
        zSizeSample=(zSizeSample & ~sampleMask)+sampleSize;

    xSizeSample=(xSizeSample>>sampleScale)+1;
    ySizeSample=(ySizeSample>>sampleScale)+1;
    zSizeSample=(zSizeSample>>sampleScale)+1;

    vectorSet->SetSize(xSizeSample*ySizeSample*zSizeSample);
    vectorSet->sampleScale=sampleScale;

    int index=0;

    for(int ix=0; ix<xSizeSample; ix++)
    {
        for(int iy=0; iy<ySizeSample; iy++)
        {
            for(int iz=0; iz<zSizeSample; iz++)
            {
                vectorSet->xSet[index]=float(ix*sampleSize);
                vectorSet->ySet[index]=float(iy*sampleSize);
                vectorSet->zSet[index]=float(iz*sampleSize);
                index++;
            }
        }
    }
}

void NoiseSIMD::FillSet(float* noiseSet, int xStart, int yStart, int zStart, int xSize, int ySize, int zSize, float scaleModifier)
{
    assert(false);
}

void NoiseSIMD::FillSet(float* noiseSet, VectorSet* vectorSet, float xOffset, float yOffset, float zOffset)
{
    assert(false);
}

float CalculateFractalBounding(int octaves, float gain)
{
    float amp=gain;
    float ampFractal=1.0f;
    for(int i=1; i<octaves; i++)
    {
        ampFractal+=amp;
        amp*=gain;
    }
    return 1.0f/ampFractal;
}

void NoiseSIMD::SetCellularDistance2Indicies(int cellularDistanceIndex0, int cellularDistanceIndex1)
{
    m_noiseDetails.cellularDistanceIndex0=std::min(cellularDistanceIndex0, cellularDistanceIndex1);
    m_noiseDetails.cellularDistanceIndex1=std::max(cellularDistanceIndex0, cellularDistanceIndex1);

    m_noiseDetails.cellularDistanceIndex0=std::min(std::max(m_noiseDetails.cellularDistanceIndex0, 0), HN_CELLULAR_INDEX_MAX);
    m_noiseDetails.cellularDistanceIndex1=std::min(std::max(m_noiseDetails.cellularDistanceIndex1, 0), HN_CELLULAR_INDEX_MAX);
}

}//namespace HastyNoise