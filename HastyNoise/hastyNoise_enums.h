// HastyNoise.h
//
// MIT License
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
//

#ifndef HASTYNOISE_ENUMS_H
#define HASTYNOISE_ENUMS_H

#include <string>
#include <vector>

namespace HastyNoise
{

#ifdef _MSC_VER

#if _MSC_VER < 1912 //msvc 2015 linker doesnt see this as a problem (and inline not supported)
#define HASTY_INLINE_VAR
#else
#define HASTY_INLINE_VAR inline
#endif

#else//_MSC_VER

#define HASTY_INLINE_VAR inline

#endif//_MSC_VER

template<typename _Type>
struct NameTypeKey
{
    std::string name;
    _Type type;
};

template<typename _Type>
struct EnumKeys
{
    static std::vector<NameTypeKey<_Type>> keys;
};

constexpr size_t SIMDTypeCount=6;
enum class SIMDType { None=0, Neon=1, SSE2=2, SSE4_1=3, AVX2=4, AVX512=5 };
template<>
std::vector<NameTypeKey<SIMDType>> EnumKeys<SIMDType>::keys
{
    {"None", HastyNoise::SIMDType::None},
    {"Neon", HastyNoise::SIMDType::Neon},
    {"SSE2", HastyNoise::SIMDType::SSE2},
    {"SSE41", HastyNoise::SIMDType::SSE4_1},
    {"AVX2", HastyNoise::SIMDType::AVX2},
    {"AVX512", HastyNoise::SIMDType::AVX512}
};

enum class NoiseType { None, Value, ValueFractal, Perlin, PerlinFractal, Simplex, SimplexFractal, WhiteNoise, Cellular, Cubic, CubicFractal };
template<>
struct EnumKeys<NoiseType>
{
    static std::vector<NameTypeKey<NoiseType>> keys;
};

inline bool isFractal(NoiseType type)
{
    switch(type)
    {
    case HastyNoise::NoiseType::None:
    case HastyNoise::NoiseType::Value:
    case HastyNoise::NoiseType::Perlin:
    case HastyNoise::NoiseType::Simplex:
    case HastyNoise::NoiseType::WhiteNoise:
    case HastyNoise::NoiseType::Cellular:
    case HastyNoise::NoiseType::Cubic:
        return false;
        break;
    case HastyNoise::NoiseType::ValueFractal:
    case HastyNoise::NoiseType::PerlinFractal:
    case HastyNoise::NoiseType::SimplexFractal:
    case HastyNoise::NoiseType::CubicFractal:
        return true;
        break;
    }
    return false;
}

enum class FractalType { None, FBM, Billow, RigidMulti };
template<>
struct EnumKeys<FractalType>
{
    static std::vector<NameTypeKey<FractalType>> keys;
};

enum class PerturbType { None, Gradient, GradientFractal, Normalise, Gradient_Normalise, GradientFractal_Normalise };
template<>
struct EnumKeys<PerturbType>
{
    static std::vector<NameTypeKey<PerturbType>> keys;
};

enum class CellularDistance { None, Euclidean, Manhattan, Natural };
template<>
struct EnumKeys<CellularDistance>
{
    static std::vector<NameTypeKey<CellularDistance>> keys;
};

enum class CellularReturnType { None, Value, Distance, Distance2, ValueDistance2, Distance2Add, Distance2Sub, Distance2Mul, Distance2Div, NoiseLookup, Distance2Cave };
template<>
struct EnumKeys<CellularReturnType>
{
    static std::vector<NameTypeKey<CellularReturnType>> keys;
};

enum class NoiseClass { Single, Fractal, Cellular };
template<>
struct EnumKeys<NoiseClass>
{
    static std::vector<NameTypeKey<NoiseClass>> keys;
};

enum class BuildType { Default, Map, Vector };
template<>
struct EnumKeys<BuildType>
{
    static std::vector<NameTypeKey<BuildType>> keys;
};

template<typename _Type>
std::string getName(_Type type)
{
    auto &keys=EnumKeys<_Type>::keys;

    for(auto &key:keys)
    {
        if(key.type==type)
            return key.name;
    }
    return "Unknown";
}

template<typename _Type>
_Type getType(std::string name)
{
    auto &keys=EnumKeys<_Type>::keys;

    for(auto &key:keys)
    {
        if(key.name==name)
            return key.type;
    }
    return keys[0].type;
}

}//namespace HastyNoise

#endif//HASTYNOISE_ENUMS_H