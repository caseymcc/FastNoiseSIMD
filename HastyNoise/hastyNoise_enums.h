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
template<> HASTY_INLINE_VAR std::vector<NameTypeKey<SIMDType>> EnumKeys<SIMDType>::keys=
{
    {"None", HastyNoise::SIMDType::None},
    {"Neon", HastyNoise::SIMDType::Neon},
    {"SSE2", HastyNoise::SIMDType::SSE2},
    {"SSE41", HastyNoise::SIMDType::SSE4_1},
    {"AVX2", HastyNoise::SIMDType::AVX2},
    {"AVX512", HastyNoise::SIMDType::AVX512}
};

enum class NoiseType { None, Value, ValueFractal, Perlin, PerlinFractal, Simplex, SimplexFractal, OpenSimplex2, OpenSimplex2Fractal, WhiteNoise, Cellular, Cubic, CubicFractal };
template<> HASTY_INLINE_VAR std::vector<NameTypeKey<NoiseType>> EnumKeys<NoiseType>::keys=
{
    {"None", HastyNoise::NoiseType::None},
    {"Value", HastyNoise::NoiseType::Value},
    {"ValueFractal", HastyNoise::NoiseType::ValueFractal},
    {"Perlin", HastyNoise::NoiseType::Perlin},
    {"PerlinFractal", HastyNoise::NoiseType::PerlinFractal},
    {"Simplex", HastyNoise::NoiseType::Simplex},
    {"SimplexFractal", HastyNoise::NoiseType::SimplexFractal},
	{"OpenSimplex2", HastyNoise::NoiseType::OpenSimplex2},
	{"OpenSimplex2Fractal", HastyNoise::NoiseType::OpenSimplex2Fractal},
    {"WhiteNoise", HastyNoise::NoiseType::WhiteNoise},
    {"Cellular", HastyNoise::NoiseType::Cellular},
    {"Cubic", HastyNoise::NoiseType::Cubic},
    {"CubicFractal", HastyNoise::NoiseType::CubicFractal}
};

inline bool isFractal(NoiseType type)
{
    switch(type)
    {
    case HastyNoise::NoiseType::None:
    case HastyNoise::NoiseType::Value:
    case HastyNoise::NoiseType::Perlin:
    case HastyNoise::NoiseType::Simplex:
	case HastyNoise::NoiseType::OpenSimplex2:
    case HastyNoise::NoiseType::WhiteNoise:
    case HastyNoise::NoiseType::Cellular:
    case HastyNoise::NoiseType::Cubic:
        return false;
        break;
    case HastyNoise::NoiseType::ValueFractal:
    case HastyNoise::NoiseType::PerlinFractal:
    case HastyNoise::NoiseType::SimplexFractal:
	case HastyNoise::NoiseType::OpenSimplex2Fractal:
    case HastyNoise::NoiseType::CubicFractal:
        return true;
        break;
    }
    return false;
}

enum class FractalType { None, FBM, Billow, RigidMulti };
template<> HASTY_INLINE_VAR std::vector<NameTypeKey<FractalType>> EnumKeys<FractalType>::keys=
{
    {"None", HastyNoise::FractalType::None},
    {"FBM", HastyNoise::FractalType::FBM},
    {"Billow", HastyNoise::FractalType::Billow},
    {"RigidMulti", HastyNoise::FractalType::RigidMulti}
};

enum class PerturbType { None, Gradient, GradientFractal, Normalise, Gradient_Normalise, GradientFractal_Normalise };
template<> HASTY_INLINE_VAR std::vector<NameTypeKey<PerturbType>> EnumKeys<PerturbType>::keys=
{
    {"None", HastyNoise::PerturbType::None},
    {"Gradient", HastyNoise::PerturbType::Gradient},
    {"GradientFractal", HastyNoise::PerturbType::GradientFractal},
    {"Normalise", HastyNoise::PerturbType::Normalise},
    {"Gradient_Normalise", HastyNoise::PerturbType::Gradient_Normalise},
    {"GradientFractal_Normalise", HastyNoise::PerturbType::GradientFractal_Normalise}
};

enum class CellularDistance { None, Euclidean, Manhattan, Natural };
template<> HASTY_INLINE_VAR std::vector<NameTypeKey<CellularDistance>> EnumKeys<CellularDistance>::keys=
{
    {"None", HastyNoise::CellularDistance::None},
    {"Euclidean", HastyNoise::CellularDistance::Euclidean},
    {"Manhattan", HastyNoise::CellularDistance::Manhattan},
    {"Natural", HastyNoise::CellularDistance::Natural}
};

enum class CellularReturnType { None, Value, Distance, Distance2, ValueDistance2, Distance2Add, Distance2Sub, Distance2Mul, Distance2Div, NoiseLookup, Distance2Cave };
template<> HASTY_INLINE_VAR std::vector<NameTypeKey<CellularReturnType>> EnumKeys<CellularReturnType>::keys=
{
    {"None", HastyNoise::CellularReturnType::None},
    {"Value", HastyNoise::CellularReturnType::Value},
    {"Distance", HastyNoise::CellularReturnType::Distance},
    {"Distance2", HastyNoise::CellularReturnType::Distance2},
    {"ValueDistance2", HastyNoise::CellularReturnType::ValueDistance2},
    {"Distance2Add", HastyNoise::CellularReturnType::Distance2Add},
    {"Distance2Sub", HastyNoise::CellularReturnType::Distance2Sub},
    {"Distance2Mul", HastyNoise::CellularReturnType::Distance2Mul},
    {"Distance2Div", HastyNoise::CellularReturnType::Distance2Div},
    {"NoiseLookup", HastyNoise::CellularReturnType::NoiseLookup},
    {"Distance2Cave", HastyNoise::CellularReturnType::Distance2Cave}
};

enum class NoiseClass { Single, Fractal, Cellular };
template<> HASTY_INLINE_VAR std::vector<NameTypeKey<NoiseClass>> EnumKeys<NoiseClass>::keys=
{
    {"Single", HastyNoise::NoiseClass::Single},
    {"Fractal", HastyNoise::NoiseClass::Fractal},
    {"Cellular", HastyNoise::NoiseClass::Cellular}
};

enum class BuildType { Default, Map, Vector };
template<> HASTY_INLINE_VAR std::vector<NameTypeKey<BuildType>> EnumKeys<BuildType>::keys=
{
    {"Default", HastyNoise::BuildType::Default},
    {"Map", HastyNoise::BuildType::Map},
    {"Vector", HastyNoise::BuildType::Vector}
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