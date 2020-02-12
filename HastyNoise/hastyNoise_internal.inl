// hastyNoise_internal.inl
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

//#include "hastyNoise.h"
#include <assert.h> 

#ifndef _WIN32
#include <cstdlib>
#endif

// Memory Allocation
#if SIMD_LEVEL > HN_NO_SIMD_FALLBACK && defined(HN_ALIGNED_SETS)
#ifdef _WIN32
#define SIMD_ALLOCATE_SET(floatP, floatCount) floatP = (float*)_aligned_malloc((floatCount)* sizeof(float), MEMORY_ALIGNMENT)
#else
#include <cstdlib>
#define SIMD_ALLOCATE_SET(floatP, floatCount) posix_memalign((void**)&floatP, MEMORY_ALIGNMENT, (floatCount)* sizeof(float))
#endif
#else
#define SIMD_ALLOCATE_SET(floatP, floatCount) floatP = new float[floatCount]
#endif

#define SIMD_HELPERS \
typedef SIMD<_SIMDType> simd;\
typedef typename simd::Float Float;\
typedef typename simd::Int Int;\
typedef typename simd::Mask Mask;\
typedef Constants<Float, Int, _SIMDType> Constant;

namespace HastyNoise
{
namespace details
{

template<SIMDType _SIMDType>
const bool NoiseSIMD<_SIMDType>::m_registered=HastyNoise::registerNoiseSimd(_SIMDType, NoiseSIMD<_SIMDType>::create, NoiseSIMD<_SIMDType>::AlignedSize, NoiseSIMD<_SIMDType>::GetEmptySet);

template<SIMDType _SIMDType>
struct simdAlloc
{
    static float *_(size_t count)
    {
		SIMD_HELPERS

#ifdef HN_ALIGNED_SETS
#   ifdef _WIN32
        return (float*)_aligned_malloc((count)*sizeof(float), simd::alignment());
#   else
        float *ptr;

        posix_memalign((void**)&ptr, simd::alignment(), (count)*sizeof(float));
        return ptr;
#   endif
#else
        return new float[count];
#endif
    }
};

template<>
struct simdAlloc<SIMDType::None>
{
    static float *_(size_t count) { return new float[count]; }
};

template<SIMDType _SIMDType>
static typename SIMD<_SIMDType>::Float VECTORCALL Lerp(typename SIMD<_SIMDType>::Float a, typename SIMD<_SIMDType>::Float b, typename SIMD<_SIMDType>::Float t)
{
	SIMD_HELPERS

    Float r;
    r=simd::sub(b, a);
    r=simd::mulAdd(r, t, a);
    return r;
}

template<SIMDType _SIMDType>
static typename SIMD<_SIMDType>::Float VECTORCALL InterpQuintic(typename SIMD<_SIMDType>::Float t)
{
	SIMD_HELPERS

    Float r;
    r=simd::mulSub(t, Constant::numf_6, Constant::numf_15);
    r=simd::mulAdd(r, t, Constant::numf_10);
    r=simd::mulf(r, t);
    r=simd::mulf(r, t);
    r=simd::mulf(r, t);

    return r;
}

template<SIMDType _SIMDType>
static typename SIMD<_SIMDType>::Float VECTORCALL CubicLerp(typename SIMD<_SIMDType>::Float a, typename SIMD<_SIMDType>::Float b, typename SIMD<_SIMDType>::Float c, typename SIMD<_SIMDType>::Float d, typename SIMD<_SIMDType>::Float t)
{
	SIMD_HELPERS

    Float p=simd::sub(simd::sub(d, c), simd::sub(a, b));
    return simd::mulAdd(t, simd::mulf(t, simd::mulf(t, p)), simd::mulAdd(t, simd::mulf(t, simd::sub(simd::sub(a, b), p)), simd::mulAdd(t, simd::sub(c, a), b)));
}

template<SIMDType _SIMDType>
static typename SIMD<_SIMDType>::Int VECTORCALL Hash(typename SIMD<_SIMDType>::Int seed, typename SIMD<_SIMDType>::Int x, typename SIMD<_SIMDType>::Int y, typename SIMD<_SIMDType>::Int z)
{
	SIMD_HELPERS

    Int hash=seed;

    hash=simd::_xor(x, hash);
    hash=simd::_xor(y, hash);
    hash=simd::_xor(z, hash);

    hash=simd::mul(simd::mul(simd::mul(hash, hash), Constant::numi_60493), hash);
    hash=simd::_xor(simd::shiftR(hash, 13), hash);

    return hash;
}

template<SIMDType _SIMDType>
static typename SIMD<_SIMDType>::Int VECTORCALL HashHB(typename SIMD<_SIMDType>::Int seed, typename SIMD<_SIMDType>::Int x, typename SIMD<_SIMDType>::Int y, typename SIMD<_SIMDType>::Int z)
{
	SIMD_HELPERS

    Int hash=seed;

    hash=simd::_xor(x, hash);
    hash=simd::_xor(y, hash);
    hash=simd::_xor(z, hash);

    hash=simd::mul(simd::mul(simd::mul(hash, hash), Constant::numi_60493), hash);

    return hash;
}

template<SIMDType _SIMDType>
static typename SIMD<_SIMDType>::Float VECTORCALL ValCoord(typename SIMD<_SIMDType>::Int seed, typename SIMD<_SIMDType>::Int x, typename SIMD<_SIMDType>::Int y, typename SIMD<_SIMDType>::Int z)
{
	SIMD_HELPERS

    // High bit hash
    Int hash=seed;

    hash=simd::_xor(x, hash);
    hash=simd::_xor(y, hash);
    hash=simd::_xor(z, hash);

    hash=simd::mul(simd::mul(simd::mul(hash, hash), Constant::numi_60493), hash);

    return simd::mulf(Constant::numf_hash2Float, simd::convert(hash));
}

template<SIMDType _SIMDType>
struct GradCoord
{
    static typename SIMD<_SIMDType>::Float VECTORCALL _(typename SIMD<_SIMDType>::Int seed, typename SIMD<_SIMDType>::Int xi, typename SIMD<_SIMDType>::Int yi, typename SIMD<_SIMDType>::Int zi, typename SIMD<_SIMDType>::Float x, typename SIMD<_SIMDType>::Float y, typename SIMD<_SIMDType>::Float z)
    {
		SIMD_HELPERS

        Int hash=Hash<_SIMDType>(seed, xi, yi, zi);
        Int hasha13=simd::_and(hash, Constant::numi_13);

        //if h < 8 then x, else y
        Mask l8=simd::lessThan(hasha13, Constant::numi_8);
        Float u=simd::blend(y, x, l8);

        //if h < 4 then y else if h is 12 or 14 then x else z
        Mask l4=simd::lessThan(hasha13, Constant::numi_2);
        Mask h12o14=simd::equal(Constant::numi_12, hasha13);
        Float v=simd::blend(simd::blend(z, x, h12o14), y, l4);

        //if h1 then -u else u
        //if h2 then -v else v
        Float h1=simd::cast(simd::shiftL(hash, 31));
        Float h2=simd::cast(simd::shiftL(simd::_and(hash, Constant::numi_2), 30));
        //then add them
        return simd::add(simd::_xor(u, h1), simd::_xor(v, h2));
    }
};

template<SIMDType _SIMDType, NoiseType _NoiseType>
struct Single
{
    static typename SIMD<_SIMDType>::Float VECTORCALL _(typename SIMD<_SIMDType>::Int seed, typename SIMD<_SIMDType>::Float x, typename SIMD<_SIMDType>::Float y, typename SIMD<_SIMDType>::Float z)
    {
		SIMD_HELPERS

        assert(false);
        return simd::zeroFloat();
    }
};

template<SIMDType _SIMDType>
struct Single<_SIMDType, NoiseType::WhiteNoise>
{
    static typename SIMD<_SIMDType>::Float VECTORCALL _(typename SIMD<_SIMDType>::Int seed, typename SIMD<_SIMDType>::Float x, typename SIMD<_SIMDType>::Float y, typename SIMD<_SIMDType>::Float z)
    {
		SIMD_HELPERS

        return ValCoord<_SIMDType>(seed,
            simd::mul(simd::_xor(simd::cast(x), simd::shiftR(simd::cast(x), 16)), Constant::numi_xPrime),
            simd::mul(simd::_xor(simd::cast(y), simd::shiftR(simd::cast(y), 16)), Constant::numi_yPrime),
            simd::mul(simd::_xor(simd::cast(z), simd::shiftR(simd::cast(z), 16)), Constant::numi_zPrime));
    }
};

template<SIMDType _SIMDType>
struct Single<_SIMDType, NoiseType::Value>
{
    static typename SIMD<_SIMDType>::Float VECTORCALL _(typename SIMD<_SIMDType>::Int seed, typename SIMD<_SIMDType>::Float x, typename SIMD<_SIMDType>::Float y, typename SIMD<_SIMDType>::Float z)
    {
		SIMD_HELPERS

        Float xs=simd::floor(x);
        Float ys=simd::floor(y);
        Float zs=simd::floor(z);

        Int x0=simd::mul(simd::convert(xs), Constant::numi_xPrime);
        Int y0=simd::mul(simd::convert(ys), Constant::numi_yPrime);
        Int z0=simd::mul(simd::convert(zs), Constant::numi_zPrime);
        Int x1=simd::add(x0, Constant::numi_xPrime);
        Int y1=simd::add(y0, Constant::numi_yPrime);
        Int z1=simd::add(z0, Constant::numi_zPrime);

        xs=InterpQuintic<_SIMDType>(simd::sub(x, xs));
        ys=InterpQuintic<_SIMDType>(simd::sub(y, ys));
        zs=InterpQuintic<_SIMDType>(simd::sub(z, zs));

        return Lerp<_SIMDType>(
            Lerp<_SIMDType>(
                Lerp<_SIMDType>(ValCoord<_SIMDType>(seed, x0, y0, z0), ValCoord<_SIMDType>(seed, x1, y0, z0), xs),
                Lerp<_SIMDType>(ValCoord<_SIMDType>(seed, x0, y1, z0), ValCoord<_SIMDType>(seed, x1, y1, z0), xs), ys),
            Lerp<_SIMDType>(
                Lerp<_SIMDType>(ValCoord<_SIMDType>(seed, x0, y0, z1), ValCoord<_SIMDType>(seed, x1, y0, z1), xs),
                Lerp<_SIMDType>(ValCoord<_SIMDType>(seed, x0, y1, z1), ValCoord<_SIMDType>(seed, x1, y1, z1), xs), ys), zs);
    }
};

template<SIMDType _SIMDType>
struct Single<_SIMDType, NoiseType::Perlin>
{
    static typename SIMD<_SIMDType>::Float VECTORCALL _(typename SIMD<_SIMDType>::Int seed, typename SIMD<_SIMDType>::Float x, typename SIMD<_SIMDType>::Float y, typename SIMD<_SIMDType>::Float z)
    {
		SIMD_HELPERS

        Float xs=simd::floor(x);
        Float ys=simd::floor(y);
        Float zs=simd::floor(z);

        Int x0=simd::mul(simd::convert(xs), Constant::numi_xPrime);
        Int y0=simd::mul(simd::convert(ys), Constant::numi_yPrime);
        Int z0=simd::mul(simd::convert(zs), Constant::numi_zPrime);
        Int x1=simd::add(x0, Constant::numi_xPrime);
        Int y1=simd::add(y0, Constant::numi_yPrime);
        Int z1=simd::add(z0, Constant::numi_zPrime);

        Float xf0=xs=simd::sub(x, xs);
        Float yf0=ys=simd::sub(y, ys);
        Float zf0=zs=simd::sub(z, zs);
        Float xf1=simd::sub(xf0, Constant::numf_1);
        Float yf1=simd::sub(yf0, Constant::numf_1);
        Float zf1=simd::sub(zf0, Constant::numf_1);

        xs=InterpQuintic<_SIMDType>(xs);
        ys=InterpQuintic<_SIMDType>(ys);
        zs=InterpQuintic<_SIMDType>(zs);

        return Lerp<_SIMDType>(
            Lerp<_SIMDType>(
                Lerp<_SIMDType>(GradCoord<_SIMDType>::_(seed, x0, y0, z0, xf0, yf0, zf0), GradCoord<_SIMDType>::_(seed, x1, y0, z0, xf1, yf0, zf0), xs),
                Lerp<_SIMDType>(GradCoord<_SIMDType>::_(seed, x0, y1, z0, xf0, yf1, zf0), GradCoord<_SIMDType>::_(seed, x1, y1, z0, xf1, yf1, zf0), xs), ys),
            Lerp<_SIMDType>(
                Lerp<_SIMDType>(GradCoord<_SIMDType>::_(seed, x0, y0, z1, xf0, yf0, zf1), GradCoord<_SIMDType>::_(seed, x1, y0, z1, xf1, yf0, zf1), xs),
                Lerp<_SIMDType>(GradCoord<_SIMDType>::_(seed, x0, y1, z1, xf0, yf1, zf1), GradCoord<_SIMDType>::_(seed, x1, y1, z1, xf1, yf1, zf1), xs), ys), zs);
    }
};

template<SIMDType _SIMDType>
struct Single<_SIMDType, NoiseType::Simplex>
{
    static typename SIMD<_SIMDType>::Float VECTORCALL _(typename SIMD<_SIMDType>::Int seed, typename SIMD<_SIMDType>::Float x, typename SIMD<_SIMDType>::Float y, typename SIMD<_SIMDType>::Float z)
    {
		SIMD_HELPERS
		
        Float f=simd::mulf(Constant::numf_F3, simd::add(simd::add(x, y), z));
        Float x0=simd::floor(simd::add(x, f));
        Float y0=simd::floor(simd::add(y, f));
        Float z0=simd::floor(simd::add(z, f));

        Int i=simd::mul(simd::convert(x0), Constant::numi_xPrime);
        Int j=simd::mul(simd::convert(y0), Constant::numi_yPrime);
        Int k=simd::mul(simd::convert(z0), Constant::numi_zPrime);

        Float g=simd::mulf(Constant::numf_G3, simd::add(simd::add(x0, y0), z0));
        x0=simd::sub(x, simd::sub(x0, g));
        y0=simd::sub(y, simd::sub(y0, g));
        z0=simd::sub(z, simd::sub(z0, g));

        Mask x0_ge_y0=simd::greaterEqual(x0, y0);
        Mask y0_ge_z0=simd::greaterEqual(y0, z0);
        Mask x0_ge_z0=simd::greaterEqual(x0, z0);

        Mask i1=simd::maskAnd(x0_ge_y0, x0_ge_z0);
        Mask j1=simd::maskAndNot(x0_ge_y0, y0_ge_z0);
        Mask k1=simd::maskAndNot(x0_ge_z0, simd::maskNot(y0_ge_z0));

        Mask i2=simd::maskOr(x0_ge_y0, x0_ge_z0);
        Mask j2=simd::maskOr(simd::maskNot(x0_ge_y0), y0_ge_z0);
        Mask k2=simd::maskNot(simd::maskAnd(x0_ge_z0, y0_ge_z0));

        Float x1=simd::add(simd::maskSub(i1, x0, Constant::numf_1), Constant::numf_G3);
        Float y1=simd::add(simd::maskSub(j1, y0, Constant::numf_1), Constant::numf_G3);
        Float z1=simd::add(simd::maskSub(k1, z0, Constant::numf_1), Constant::numf_G3);
        Float x2=simd::add(simd::maskSub(i2, x0, Constant::numf_1), Constant::numf_F3);
        Float y2=simd::add(simd::maskSub(j2, y0, Constant::numf_1), Constant::numf_F3);
        Float z2=simd::add(simd::maskSub(k2, z0, Constant::numf_1), Constant::numf_F3);
        Float x3=simd::add(x0, Constant::numf_G33);
        Float y3=simd::add(y0, Constant::numf_G33);
        Float z3=simd::add(z0, Constant::numf_G33);

        Float t0=simd::nmulAdd(z0, z0, simd::nmulAdd(y0, y0, simd::nmulAdd(x0, x0, Constant::numf_0_6)));
        Float t1=simd::nmulAdd(z1, z1, simd::nmulAdd(y1, y1, simd::nmulAdd(x1, x1, Constant::numf_0_6)));
        Float t2=simd::nmulAdd(z2, z2, simd::nmulAdd(y2, y2, simd::nmulAdd(x2, x2, Constant::numf_0_6)));
        Float t3=simd::nmulAdd(z3, z3, simd::nmulAdd(y3, y3, simd::nmulAdd(x3, x3, Constant::numf_0_6)));

        t0=simd::max(t0, Constant::numf_0);
        t1=simd::max(t1, Constant::numf_0);
        t2=simd::max(t2, Constant::numf_0);
        t3=simd::max(t3, Constant::numf_0);

        t0=simd::mulf(t0, t0);
        t1=simd::mulf(t1, t1);
        t2=simd::mulf(t2, t2);
        t3=simd::mulf(t3, t3);

        Float v0=simd::mulf(simd::mulf(t0, t0), GradCoord<_SIMDType>::_(seed, i, j, k, x0, y0, z0));
        Float v1=simd::mulf(simd::mulf(t1, t1), GradCoord<_SIMDType>::_(seed, simd::maskAdd(i1, i, Constant::numi_xPrime), simd::maskAdd(j1, j, Constant::numi_yPrime), simd::maskAdd(k1, k, Constant::numi_zPrime), x1, y1, z1));
        Float v2=simd::mulf(simd::mulf(t2, t2), GradCoord<_SIMDType>::_(seed, simd::maskAdd(i2, i, Constant::numi_xPrime), simd::maskAdd(j2, j, Constant::numi_yPrime), simd::maskAdd(k2, k, Constant::numi_zPrime), x2, y2, z2));
        Float v3=simd::mulf(simd::mulf(t3, t3), GradCoord<_SIMDType>::_(seed, simd::add(i, Constant::numi_xPrime), simd::add(j, Constant::numi_yPrime), simd::add(k, Constant::numi_zPrime), x3, y3, z3));

        return simd::mulf(Constant::numf_32, simd::add(simd::add(simd::add(v3, v2), v1), v0));
    }
};

template<SIMDType _SIMDType>
struct Single<_SIMDType, NoiseType::OpenSimplex2>
{
	static typename SIMD<_SIMDType>::Float VECTORCALL _(typename SIMD<_SIMDType>::Int seed, typename SIMD<_SIMDType>::Float x, typename SIMD<_SIMDType>::Float y, typename SIMD<_SIMDType>::Float z)
	{
		SIMD_HELPERS

		Float f=simd::mulf(Constant::numf_R3, simd::add(simd::add(x, y), z));
		Float xr=simd::sub(f, x);
		Float yr=simd::sub(f, y);
		Float zr=simd::sub(f, z);

		Float val=Constant::numf_0;
		for(size_t i=0; i<2; i++)
		{
			Float v0xr=simd::floor(simd::add(xr, Constant::numf_0_5));
			Float v0yr=simd::floor(simd::add(yr, Constant::numf_0_5));
			Float v0zr=simd::floor(simd::add(zr, Constant::numf_0_5));
			Float d0xr=simd::sub(xr, v0xr);
			Float d0yr=simd::sub(yr, v0yr);
			Float d0zr=simd::sub(zr, v0zr);

			Float score0xr=simd::abs(d0xr);
			Float score0yr=simd::abs(d0yr);
			Float score0zr=simd::abs(d0zr);
			Mask dir0xr=simd::lessEqual(simd::max(score0yr, score0zr), score0xr);
			Mask dir0yr=simd::maskAndNot(dir0xr, simd::lessEqual(simd::max(score0zr, score0xr), score0yr));
			Mask dir0zr=simd::maskNot(simd::maskOr(dir0xr, dir0yr));
			Float v1xr=simd::maskAdd(dir0xr, v0xr, simd::_or(Constant::numf_1, simd::_and(d0xr, Constant::numf_neg1)));
			Float v1yr=simd::maskAdd(dir0yr, v0yr, simd::_or(Constant::numf_1, simd::_and(d0yr, Constant::numf_neg1)));
			Float v1zr=simd::maskAdd(dir0zr, v0zr, simd::_or(Constant::numf_1, simd::_and(d0zr, Constant::numf_neg1)));
			Float d1xr=simd::sub(xr, v1xr);
			Float d1yr=simd::sub(yr, v1yr);
			Float d1zr=simd::sub(zr, v1zr);

			Int hv0xr=simd::mul(simd::convert(v0xr), Constant::numi_xPrime);
			Int hv0yr=simd::mul(simd::convert(v0yr), Constant::numi_yPrime);
			Int hv0zr=simd::mul(simd::convert(v0zr), Constant::numi_zPrime);
			Int hv1xr=simd::mul(simd::convert(v1xr), Constant::numi_xPrime);
			Int hv1yr=simd::mul(simd::convert(v1yr), Constant::numi_yPrime);
			Int hv1zr=simd::mul(simd::convert(v1zr), Constant::numi_zPrime);

			Float t0=simd::nmulAdd(d0zr, d0zr, simd::nmulAdd(d0yr, d0yr, simd::nmulAdd(d0xr, d0xr, Constant::numf_0_6)));
			Float t1=simd::nmulAdd(d1zr, d1zr, simd::nmulAdd(d1yr, d1yr, simd::nmulAdd(d1xr, d1xr, Constant::numf_0_6)));
			t0=simd::max(t0, Constant::numf_0);
			t1=simd::max(t1, Constant::numf_0);
			t0=simd::mulf(t0, t0);
			t1=simd::mulf(t1, t1);

			Float v0=simd::mulf(simd::mulf(t0, t0), GradCoord<_SIMDType>::_(seed, hv0xr, hv0yr, hv0zr, d0xr, d0yr, d0zr));
			Float v1=simd::mulf(simd::mulf(t1, t1), GradCoord<_SIMDType>::_(seed, hv1xr, hv1yr, hv1zr, d1xr, d1yr, d1zr));

			val=simd::add(simd::add(val, v1), v0);

			if(i==0)
			{
				xr=simd::add(xr, Constant::numf_32768_5);
				yr=simd::add(yr, Constant::numf_32768_5);
				zr=simd::add(zr, Constant::numf_32768_5);
			}
		}
		return simd::mulf(Constant::numf_32, val);
	}
};

template<SIMDType _SIMDType>
struct Single<_SIMDType, NoiseType::Cubic>
{
    static typename SIMD<_SIMDType>::Float VECTORCALL _(typename SIMD<_SIMDType>::Int seed, typename SIMD<_SIMDType>::Float x, typename SIMD<_SIMDType>::Float y, typename SIMD<_SIMDType>::Float z)
    {
		SIMD_HELPERS

        Float xf1=simd::floor(x);
        Float yf1=simd::floor(y);
        Float zf1=simd::floor(z);

        Int x1=simd::mul(simd::convert(xf1), Constant::numi_xPrime);
        Int y1=simd::mul(simd::convert(yf1), Constant::numi_yPrime);
        Int z1=simd::mul(simd::convert(zf1), Constant::numi_zPrime);

        Int x0=simd::sub(x1, Constant::numi_xPrime);
        Int y0=simd::sub(y1, Constant::numi_yPrime);
        Int z0=simd::sub(z1, Constant::numi_zPrime);
        Int x2=simd::add(x1, Constant::numi_xPrime);
        Int y2=simd::add(y1, Constant::numi_yPrime);
        Int z2=simd::add(z1, Constant::numi_zPrime);
        Int x3=simd::add(x2, Constant::numi_xPrime);
        Int y3=simd::add(y2, Constant::numi_yPrime);
        Int z3=simd::add(z2, Constant::numi_zPrime);

        Float xs=simd::sub(x, xf1);
        Float ys=simd::sub(y, yf1);
        Float zs=simd::sub(z, zf1);

        return simd::mulf(CubicLerp<_SIMDType>(
            CubicLerp<_SIMDType>(
                CubicLerp<_SIMDType>(ValCoord<_SIMDType>(seed, x0, y0, z0), ValCoord<_SIMDType>(seed, x1, y0, z0), ValCoord<_SIMDType>(seed, x2, y0, z0), ValCoord<_SIMDType>(seed, x3, y0, z0), xs),
                CubicLerp<_SIMDType>(ValCoord<_SIMDType>(seed, x0, y1, z0), ValCoord<_SIMDType>(seed, x1, y1, z0), ValCoord<_SIMDType>(seed, x2, y1, z0), ValCoord<_SIMDType>(seed, x3, y1, z0), xs),
                CubicLerp<_SIMDType>(ValCoord<_SIMDType>(seed, x0, y2, z0), ValCoord<_SIMDType>(seed, x1, y2, z0), ValCoord<_SIMDType>(seed, x2, y2, z0), ValCoord<_SIMDType>(seed, x3, y2, z0), xs),
                CubicLerp<_SIMDType>(ValCoord<_SIMDType>(seed, x0, y3, z0), ValCoord<_SIMDType>(seed, x1, y3, z0), ValCoord<_SIMDType>(seed, x2, y3, z0), ValCoord<_SIMDType>(seed, x3, y3, z0), xs),
                ys),
            CubicLerp<_SIMDType>(
                CubicLerp<_SIMDType>(ValCoord<_SIMDType>(seed, x0, y0, z1), ValCoord<_SIMDType>(seed, x1, y0, z1), ValCoord<_SIMDType>(seed, x2, y0, z1), ValCoord<_SIMDType>(seed, x3, y0, z1), xs),
                CubicLerp<_SIMDType>(ValCoord<_SIMDType>(seed, x0, y1, z1), ValCoord<_SIMDType>(seed, x1, y1, z1), ValCoord<_SIMDType>(seed, x2, y1, z1), ValCoord<_SIMDType>(seed, x3, y1, z1), xs),
                CubicLerp<_SIMDType>(ValCoord<_SIMDType>(seed, x0, y2, z1), ValCoord<_SIMDType>(seed, x1, y2, z1), ValCoord<_SIMDType>(seed, x2, y2, z1), ValCoord<_SIMDType>(seed, x3, y2, z1), xs),
                CubicLerp<_SIMDType>(ValCoord<_SIMDType>(seed, x0, y3, z1), ValCoord<_SIMDType>(seed, x1, y3, z1), ValCoord<_SIMDType>(seed, x2, y3, z1), ValCoord<_SIMDType>(seed, x3, y3, z1), xs),
                ys),
            CubicLerp<_SIMDType>(
                CubicLerp<_SIMDType>(ValCoord<_SIMDType>(seed, x0, y0, z2), ValCoord<_SIMDType>(seed, x1, y0, z2), ValCoord<_SIMDType>(seed, x2, y0, z2), ValCoord<_SIMDType>(seed, x3, y0, z2), xs),
                CubicLerp<_SIMDType>(ValCoord<_SIMDType>(seed, x0, y1, z2), ValCoord<_SIMDType>(seed, x1, y1, z2), ValCoord<_SIMDType>(seed, x2, y1, z2), ValCoord<_SIMDType>(seed, x3, y1, z2), xs),
                CubicLerp<_SIMDType>(ValCoord<_SIMDType>(seed, x0, y2, z2), ValCoord<_SIMDType>(seed, x1, y2, z2), ValCoord<_SIMDType>(seed, x2, y2, z2), ValCoord<_SIMDType>(seed, x3, y2, z2), xs),
                CubicLerp<_SIMDType>(ValCoord<_SIMDType>(seed, x0, y3, z2), ValCoord<_SIMDType>(seed, x1, y3, z2), ValCoord<_SIMDType>(seed, x2, y3, z2), ValCoord<_SIMDType>(seed, x3, y3, z2), xs),
                ys),
            CubicLerp<_SIMDType>(
                CubicLerp<_SIMDType>(ValCoord<_SIMDType>(seed, x0, y0, z3), ValCoord<_SIMDType>(seed, x1, y0, z3), ValCoord<_SIMDType>(seed, x2, y0, z3), ValCoord<_SIMDType>(seed, x3, y0, z3), xs),
                CubicLerp<_SIMDType>(ValCoord<_SIMDType>(seed, x0, y1, z3), ValCoord<_SIMDType>(seed, x1, y1, z3), ValCoord<_SIMDType>(seed, x2, y1, z3), ValCoord<_SIMDType>(seed, x3, y1, z3), xs),
                CubicLerp<_SIMDType>(ValCoord<_SIMDType>(seed, x0, y2, z3), ValCoord<_SIMDType>(seed, x1, y2, z3), ValCoord<_SIMDType>(seed, x2, y2, z3), ValCoord<_SIMDType>(seed, x3, y2, z3), xs),
                CubicLerp<_SIMDType>(ValCoord<_SIMDType>(seed, x0, y3, z3), ValCoord<_SIMDType>(seed, x1, y3, z3), ValCoord<_SIMDType>(seed, x2, y3, z3), ValCoord<_SIMDType>(seed, x3, y3, z3), xs),
                ys),
            zs), Constant::numf_cubicBounding);
    }
};

template<SIMDType _SIMDType>
void gradientCoord(typename SIMD<_SIMDType>::Int seed, const typename SIMD<_SIMDType>::Int &x, const typename SIMD<_SIMDType>::Int &y, const typename SIMD<_SIMDType>::Int &z,
    typename SIMD<_SIMDType>::Float &xGrad, typename SIMD<_SIMDType>::Float &yGrad, typename SIMD<_SIMDType>::Float &zGrad)
{
	SIMD_HELPERS

    Int hash=HashHB<_SIMDType>(seed, x, y, z);

    xGrad=simd::sub(simd::convert(simd::_and(hash, Constant::numi_bit10Mask)), Constant::numf_511_5);
    yGrad=simd::sub(simd::convert(simd::_and(simd::shiftR(hash, 10), Constant::numi_bit10Mask)), Constant::numf_511_5);
    zGrad=simd::sub(simd::convert(simd::_and(simd::shiftR(hash, 20), Constant::numi_bit10Mask)), Constant::numf_511_5);
}

template<SIMDType _SIMDType>
static void VECTORCALL GradientPerturbSingle(typename SIMD<_SIMDType>::Int seed, typename SIMD<_SIMDType>::Float perturbAmp, typename SIMD<_SIMDType>::Float perturbFrequency, typename SIMD<_SIMDType>::Float& x, typename SIMD<_SIMDType>::Float& y, typename SIMD<_SIMDType>::Float& z)
{
	SIMD_HELPERS

    Float xf=simd::mulf(x, perturbFrequency);
    Float yf=simd::mulf(y, perturbFrequency);
    Float zf=simd::mulf(z, perturbFrequency);

    Float xs=simd::floor(xf);
    Float ys=simd::floor(yf);
    Float zs=simd::floor(zf);

    Int x0=simd::mul(simd::convert(xs), Constant::numi_xPrime);
    Int y0=simd::mul(simd::convert(ys), Constant::numi_yPrime);
    Int z0=simd::mul(simd::convert(zs), Constant::numi_zPrime);
    Int x1=simd::add(x0, Constant::numi_xPrime);
    Int y1=simd::add(y0, Constant::numi_yPrime);
    Int z1=simd::add(z0, Constant::numi_zPrime);

    xs=InterpQuintic<_SIMDType>(simd::sub(xf, xs));
    ys=InterpQuintic<_SIMDType>(simd::sub(yf, ys));
    zs=InterpQuintic<_SIMDType>(simd::sub(zf, zs));

    Float x000, y000, z000;
    Float x001, y001, z001;
    Float x010, y010, z010;
    Float x011, y011, z011;
    Float x100, y100, z100;
    Float x101, y101, z101;
    Float x110, y110, z110;
    Float x111, y111, z111;

    gradientCoord<_SIMDType>(seed, x0, y0, z0, x000, y000, z000);
    gradientCoord<_SIMDType>(seed, x0, y0, z1, x001, y001, z001);
    gradientCoord<_SIMDType>(seed, x0, y1, z0, x010, y010, z010);
    gradientCoord<_SIMDType>(seed, x0, y1, z1, x011, y011, z011);
    gradientCoord<_SIMDType>(seed, x1, y0, z0, x100, y100, z100);
    gradientCoord<_SIMDType>(seed, x1, y0, z1, x101, y101, z101);
    gradientCoord<_SIMDType>(seed, x1, y1, z0, x110, y110, z110);
    gradientCoord<_SIMDType>(seed, x1, y1, z1, x111, y111, z111);

    Float x0y=Lerp<_SIMDType>(Lerp<_SIMDType>(x000, x100, xs), Lerp<_SIMDType>(x010, x110, xs), ys);
    Float y0y=Lerp<_SIMDType>(Lerp<_SIMDType>(y000, y100, xs), Lerp<_SIMDType>(y010, y110, xs), ys);
    Float z0y=Lerp<_SIMDType>(Lerp<_SIMDType>(z000, z100, xs), Lerp<_SIMDType>(z010, z110, xs), ys);

    Float x1y=Lerp<_SIMDType>(Lerp<_SIMDType>(x001, x101, xs), Lerp<_SIMDType>(x011, x111, xs), ys);
    Float y1y=Lerp<_SIMDType>(Lerp<_SIMDType>(y001, y101, xs), Lerp<_SIMDType>(y011, y111, xs), ys);
    Float z1y=Lerp<_SIMDType>(Lerp<_SIMDType>(z001, z101, xs), Lerp<_SIMDType>(z011, z111, xs), ys);

    x=simd::mulAdd(Lerp<_SIMDType>(x0y, x1y, zs), perturbAmp, x);
    y=simd::mulAdd(Lerp<_SIMDType>(y0y, y1y, zs), perturbAmp, y);
    z=simd::mulAdd(Lerp<_SIMDType>(z0y, z1y, zs), perturbAmp, z);
}

template<SIMDType _SIMDType>
NoiseSIMD<_SIMDType>::NoiseSIMD(int seed)
{
    InitSIMDValues<_SIMDType>::_();

    m_noiseDetails.seed=seed;
    m_noiseDetails.fractalBounding=CalculateFractalBounding(m_noiseDetails.octaves, m_noiseDetails.gain);
    m_perturbDetails.FractalBounding=CalculateFractalBounding(m_perturbDetails.Octaves, m_perturbDetails.Gain);

    m_SIMDLevel=SIMD<_SIMDType>::level();
}

template<SIMDType _SIMDType>
HastyNoise::NoiseSIMD *NoiseSIMD<_SIMDType>::create(int seed)
{
    return new NoiseSIMD<_SIMDType>(seed);
}

template<SIMDType _SIMDType>
size_t NoiseSIMD<_SIMDType>::AlignedSize(size_t size)
{
#ifdef HN_ALIGNED_SETS
    // size must be a multiple of SIMD<_SIMDType>::vectorSize() (8)
    if((size & (SIMD<_SIMDType>::vectorSize()-1))!=0)
    {
        size&=~(SIMD<_SIMDType>::vectorSize()-1);
        size+=SIMD<_SIMDType>::vectorSize();
    }
#endif
    return size;
}

template<SIMDType _SIMDType>
float* NoiseSIMD<_SIMDType>::GetEmptySet(size_t size)
{
    size=AlignedSize(size);

    float* noiseSet;
    noiseSet=simdAlloc<_SIMDType>::_(size);

    return noiseSet;
}

template<SIMDType _SIMDType>
void axisReset(typename SIMD<_SIMDType>::Int &x, typename SIMD<_SIMDType>::Int &y, typename SIMD<_SIMDType>::Int &z,
    const typename SIMD<_SIMDType>::Int &ySizeV, const typename SIMD<_SIMDType>::Int &yEndV, const typename SIMD<_SIMDType>::Int &zSizeV, const typename SIMD<_SIMDType>::Int &zEndV, int &_zSize, int _start)
{
	SIMD_HELPERS

    for(size_t _i=(_zSize) * (_start); _i<simd::vectorSize(); _i+=(_zSize))
    {
        Mask _zReset=simd::greaterThan(z, zEndV);

        y=simd::maskAdd(_zReset, y, Constant::numi_1);
        z=simd::maskSub(_zReset, z, zSizeV);

        Mask _yReset=simd::greaterThan(y, yEndV);

        x=simd::maskAdd(_yReset, x, Constant::numi_1);
        y=simd::maskSub(_yReset, y, ySizeV);
    }
}

#ifdef HN_ALIGNED_SETS
#define STORE_LAST_RESULT(_dest, _source) SIMD<_SIMDType>::store(_dest, _source)
#else
#include <cstring>
#define STORE_LAST_RESULT(_dest, _source) std::memcpy(_dest, &_source, (maxIndex - index) * 4)
#endif

template<SIMDType _SIMDType>
struct NoiseValues
{
    typename SIMD<_SIMDType>::Int seedV;
    typename SIMD<_SIMDType>::Float lacunarityV;
    typename SIMD<_SIMDType>::Float gainV;
    typename SIMD<_SIMDType>::Float fractalBoundingV;

    typename SIMD<_SIMDType>::Float xFreqV;
    typename SIMD<_SIMDType>::Float yFreqV;
    typename SIMD<_SIMDType>::Float zFreqV;

    typename SIMD<_SIMDType>::Float cellJitterV;
    int index0;
    int index1;
    typename SIMD<_SIMDType>::Float cellularLookupFrequencyV;

    size_t octaves;
};

template<SIMDType _SIMDType>
NoiseValues<_SIMDType> initNoise(const NoiseDetails &noiseDetails, float scaleModifier=1.0f)
{
	SIMD_HELPERS

    NoiseValues<_SIMDType> noise;

    noise.seedV=simd::set(noiseDetails.seed);
    noise.lacunarityV=simd::set(noiseDetails.lacunarity);
    noise.gainV=simd::set(noiseDetails.gain);
    noise.fractalBoundingV=simd::set(noiseDetails.fractalBounding);

    scaleModifier*=noiseDetails.frequency;
    noise.xFreqV=simd::set(scaleModifier * noiseDetails.xScale);
    noise.yFreqV=simd::set(scaleModifier * noiseDetails.yScale);
    noise.zFreqV=simd::set(scaleModifier * noiseDetails.zScale);

    noise.cellJitterV=simd::set(noiseDetails.cellularJitter);
    noise.index0=noiseDetails.cellularDistanceIndex0;
    noise.index1=noiseDetails.cellularDistanceIndex1;

    noise.cellularLookupFrequencyV=simd::set(noiseDetails.cellularNoiseLookupFrequency);

    noise.octaves=noiseDetails.octaves;

    return noise;
};

template<SIMDType _SIMDType, PerturbType _PerturbType>
struct init_perturb
{
    static PerturbValues<_SIMDType> _(const NoiseDetails &noiseDetails, const PerturbDetails &perturbDetails)
    {
        PerturbValues<_SIMDType> perturb;

        perturb.Octaves=perturbDetails.Octaves;
        return perturb;
    }
};

template<SIMDType _SIMDType>
struct init_perturb<_SIMDType, PerturbType::Gradient_Normalise>
{
    static PerturbValues<_SIMDType> _(const NoiseDetails &noiseDetails, const PerturbDetails &perturbDetails)
    {
		SIMD_HELPERS

        PerturbValues<_SIMDType> perturb;

        perturb.Octaves=perturbDetails.Octaves;
        perturb.NormaliseLengthV=simd::set(perturbDetails.NormaliseLength*noiseDetails.frequency);
        return perturb;
    }
};

template<SIMDType _SIMDType>
struct init_perturb<_SIMDType, PerturbType::Gradient>
{
    static PerturbValues<_SIMDType> _(const NoiseDetails &noiseDetails, const PerturbDetails &perturbDetails)
    {
		SIMD_HELPERS

        PerturbValues<_SIMDType> perturb;

        perturb.Octaves=perturbDetails.Octaves;
        perturb.AmpV=simd::set(perturbDetails.Amp);
        perturb.FreqV=simd::set(perturbDetails.Frequency);
        return perturb;
    }
};

template<SIMDType _SIMDType>
struct init_perturb<_SIMDType, PerturbType::GradientFractal_Normalise>
{
    static PerturbValues<_SIMDType> _(const NoiseDetails &noiseDetails, const PerturbDetails &perturbDetails)
    {
		SIMD_HELPERS

        PerturbValues<_SIMDType> perturb;

        perturb.Octaves=perturbDetails.Octaves;
        perturb.NormaliseLengthV=simd::set(perturbDetails.NormaliseLength*noiseDetails.frequency);
        return perturb;
    }
};

template<SIMDType _SIMDType>
struct init_perturb<_SIMDType, PerturbType::GradientFractal>
{
    static PerturbValues<_SIMDType> _(const NoiseDetails &noiseDetails, const PerturbDetails &perturbDetails)
    {
		SIMD_HELPERS

        PerturbValues<_SIMDType> perturb;

        perturb.Octaves=perturbDetails.Octaves;
        perturb.AmpV=simd::set(perturbDetails.Amp*noiseDetails.fractalBounding);
        perturb.FreqV=simd::set(perturbDetails.Frequency);
        perturb.LacunarityV=simd::set(perturbDetails.Lacunarity);
        perturb.GainV=simd::set(perturbDetails.Gain);
        return perturb;
    }
};

template<SIMDType _SIMDType>
struct init_perturb<_SIMDType, PerturbType::Normalise>
{
    static PerturbValues<_SIMDType> _(const NoiseDetails &noiseDetails, const PerturbDetails &perturbDetails)
    {
		SIMD_HELPERS

        PerturbValues<_SIMDType> perturb;

        perturb.Octaves=perturbDetails.Octaves;
        perturb.NormaliseLengthV=simd::set(perturbDetails.NormaliseLength*noiseDetails.frequency);
        return perturb;
    }
};

template<SIMDType _SIMDType>
PerturbValues<_SIMDType> initPerturb(PerturbType perturbType, const NoiseDetails &noiseDetails, const PerturbDetails &perturbDetails)
{
    switch(perturbType)
    {
    case PerturbType::None:
        return init_perturb<_SIMDType, PerturbType::None>::_(noiseDetails, perturbDetails);
        break;
    case PerturbType::Gradient:
        return init_perturb<_SIMDType, PerturbType::Gradient>::_(noiseDetails, perturbDetails);
        break;
    case PerturbType::GradientFractal:
        return init_perturb<_SIMDType, PerturbType::GradientFractal>::_(noiseDetails, perturbDetails);
        break;
    case PerturbType::Normalise:
        return init_perturb<_SIMDType, PerturbType::Normalise>::_(noiseDetails, perturbDetails);
        break;
    case PerturbType::Gradient_Normalise:
        return init_perturb<_SIMDType, PerturbType::Gradient_Normalise>::_(noiseDetails, perturbDetails);
        break;
    case PerturbType::GradientFractal_Normalise:
        return init_perturb<_SIMDType, PerturbType::GradientFractal_Normalise>::_(noiseDetails, perturbDetails);
        break;
    }

    return init_perturb<_SIMDType, PerturbType::None>::_(noiseDetails, perturbDetails);
};

template<SIMDType _SIMDType, PerturbType _PerturbType>
struct Perturb
{
    static void _(const typename SIMD<_SIMDType>::Int &seedV, const PerturbValues<_SIMDType> &perturb, const typename SIMD<_SIMDType>::Float &xF, const typename SIMD<_SIMDType>::Float &yF, const typename SIMD<_SIMDType>::Float &zF)
    {}
};

template<SIMDType _SIMDType>
struct Perturb<_SIMDType, PerturbType::Gradient>
{
    static void _(const typename SIMD<_SIMDType>::Int &seedV, const PerturbValues<_SIMDType> &perturb, typename SIMD<_SIMDType>::Float &xF, typename SIMD<_SIMDType>::Float &yF, typename SIMD<_SIMDType>::Float &zF)
    {
		SIMD_HELPERS

        GradientPerturbSingle<_SIMDType>(simd::sub(seedV, Constant::numi_1), perturb.AmpV, perturb.FreqV, xF, yF, zF);
    }
};

template<SIMDType _SIMDType>
struct Perturb<_SIMDType, PerturbType::GradientFractal>
{
    static void _(const typename SIMD<_SIMDType>::Int &seedV, const PerturbValues<_SIMDType> &perturb, typename SIMD<_SIMDType>::Float &xF, typename SIMD<_SIMDType>::Float &yF, typename SIMD<_SIMDType>::Float &zF)
    {
		SIMD_HELPERS

        Int seedF=simd::sub(seedV, Constant::numi_1);
        Float freqF=perturb.FreqV;
        Float ampF=perturb.AmpV;

        GradientPerturbSingle<_SIMDType>(seedF, ampF, freqF, xF, yF, zF);

        int octaveIndex=0;

        while(++octaveIndex < perturb.Octaves)
        {
            freqF=simd::mulf(freqF, perturb.LacunarityV);
            seedF=simd::sub(seedF, Constant::numi_1);
            ampF=simd::mulf(ampF, perturb.GainV);

            GradientPerturbSingle<_SIMDType>(seedF, ampF, freqF, xF, yF, zF);
        }
    }
};

template<SIMDType _SIMDType>
struct Perturb<_SIMDType, PerturbType::Gradient_Normalise>
{
    static void _(const typename SIMD<_SIMDType>::Int &seedV, const PerturbValues<_SIMDType> &perturb, typename SIMD<_SIMDType>::Float &xF, typename SIMD<_SIMDType>::Float &yF, typename SIMD<_SIMDType>::Float &zF)
    {
		SIMD_HELPERS

        GradientPerturbSingle<_SIMDType>(simd::sub(seedV, Constant::numi_1), perturb.AmpV, perturb.FreqV, xF, yF, zF);
    }
};

template<SIMDType _SIMDType>
struct Perturb<_SIMDType, PerturbType::Normalise>
{
    static void _(const typename SIMD<_SIMDType>::Int &seedV, const PerturbValues<_SIMDType> &perturb, typename SIMD<_SIMDType>::Float &xF, typename SIMD<_SIMDType>::Float &yF, typename SIMD<_SIMDType>::Float &zF)
    {
		SIMD_HELPERS

        Float invMag=simd::mulf(perturb.NormaliseLengthV, simd::invSqrt(simd::mulAdd(xF, xF, simd::mulAdd(yF, yF, simd::mulf(zF, zF)))));

        xF=simd::mulf(xF, invMag);
        yF=simd::mulf(yF, invMag);
        zF=simd::mulf(zF, invMag);
    }
};

template<SIMDType _SIMDType>
struct Perturb<_SIMDType, PerturbType::GradientFractal_Normalise>
{
    static void _(const typename SIMD<_SIMDType>::Int &seedV, const PerturbValues<_SIMDType> &perturb, typename SIMD<_SIMDType>::Float &xF, typename SIMD<_SIMDType>::Float &yF, typename SIMD<_SIMDType>::Float &zF)
    {
		SIMD_HELPERS

        Int seedF=simd::sub(seedV, Constant::numi_1);
        Float freqF=perturb.FreqV;
        Float ampF=perturb.AmpV;

        GradientPerturbSingle<_SIMDType>(seedF, ampF, freqF, xF, yF, zF);

        int octaveIndex=0;

        while(++octaveIndex < perturb.Octaves)
        {
            freqF=simd::mulf(freqF, perturb.LacunarityV);
            seedF=simd::sub(seedF, Constant::numi_1);
            ampF=simd::mulf(ampF, perturb.GainV);

            GradientPerturbSingle<_SIMDType>(seedF, ampF, freqF, xF, yF, zF);
        }
        Float invMag=simd::mulf(perturb.NormaliseLengthV, simd::invSqrt(simd::mulAdd(xF, xF, simd::mulAdd(yF, yF, simd::mulf(zF, zF)))));
        xF=simd::mulf(xF, invMag);
        yF=simd::mulf(yF, invMag);
        zF=simd::mulf(zF, invMag);
    }
};

// FBM SINGLE
template<SIMDType _SIMDType, NoiseType _NoiseType>
typename SIMD<_SIMDType>::Float FBMSingle(const NoiseValues<_SIMDType> &noise, typename SIMD<_SIMDType>::Float &xF, typename SIMD<_SIMDType>::Float &yF, typename SIMD<_SIMDType>::Float &zF)
{
	SIMD_HELPERS

    Int seedF=noise.seedV;
    Float result=Single<_SIMDType, _NoiseType>::_(seedF, xF, yF, zF);
    Float ampF=Constant::numf_1;
    size_t octaveIndex=0;

    while(++octaveIndex<noise.octaves)
    {
        xF=simd::mulf(xF, noise.lacunarityV);
        yF=simd::mulf(yF, noise.lacunarityV);
        zF=simd::mulf(zF, noise.lacunarityV);
        seedF=simd::add(seedF, Constant::numi_1);

        ampF=simd::mulf(ampF, noise.gainV);
        result=simd::mulAdd(Single<_SIMDType, _NoiseType>::_(seedF, xF, yF, zF), ampF, result);
    }
    result=simd::mulf(result, noise.fractalBoundingV);

    return result;
}

// BILLOW SINGLE
template<SIMDType _SIMDType, NoiseType _NoiseType>
typename SIMD<_SIMDType>::Float BillowSingle(const NoiseValues<_SIMDType> &noise, typename SIMD<_SIMDType>::Float &xF, typename SIMD<_SIMDType>::Float &yF, typename SIMD<_SIMDType>::Float &zF)
{
	SIMD_HELPERS

    Int seedF=noise.seedV;
    Float result=simd::mulSub(simd::abs(Single<_SIMDType, _NoiseType>::_(seedF, xF, yF, zF)), Constant::numf_2, Constant::numf_1);
    Float ampF=Constant::numf_1;
    size_t octaveIndex=0;

    while(++octaveIndex<noise.octaves)
    {
        xF=simd::mulf(xF, noise.lacunarityV);
        yF=simd::mulf(yF, noise.lacunarityV);
        zF=simd::mulf(zF, noise.lacunarityV);
        seedF=simd::add(seedF, Constant::numi_1);

        ampF=simd::mulf(ampF, noise.gainV);
        result=simd::mulAdd(simd::mulSub(simd::abs(Single<_SIMDType, _NoiseType>::_(seedF, xF, yF, zF)), Constant::numf_2, Constant::numf_1), ampF, result);
    }
    result=simd::mulf(result, noise.fractalBoundingV);

    return result;
}

// RIGIDMULTI SINGLE
template<SIMDType _SIMDType, NoiseType _NoiseType>
typename SIMD<_SIMDType>::Float RigidMultiSingle(const NoiseValues<_SIMDType> &noise, typename SIMD<_SIMDType>::Float &xF, typename SIMD<_SIMDType>::Float &yF, typename SIMD<_SIMDType>::Float &zF)
{
	SIMD_HELPERS

    Int seedF=noise.seedV;
    Float result=simd::sub(Constant::numf_1, simd::abs(Single<_SIMDType, _NoiseType>::_(seedF, xF, yF, zF)));
    Float ampF=Constant::numf_1;
    size_t octaveIndex=0;

    while(++octaveIndex < noise.octaves)
    {
        xF=simd::mulf(xF, noise.lacunarityV);
        yF=simd::mulf(yF, noise.lacunarityV);
        zF=simd::mulf(zF, noise.lacunarityV);
        seedF=simd::add(seedF, Constant::numi_1);

        ampF=simd::mulf(ampF, noise.gainV);
        result=simd::mulAdd(simd::sub(Constant::numf_1, simd::abs(Single<_SIMDType, _NoiseType>::_(seedF, xF, yF, zF))), ampF, result);
    }
    return result;
}

#ifdef HN_ALIGNED_SETS
#define SIZE_MASK
#else
#define SIZE_MASK & ~(SIMD<_SIMDType>::vectorSize() - 1)
#endif

template<SIMDType _SIMDType, CellularDistance _CellularDistance>
struct Distance
{
    static typename SIMD<_SIMDType>::Float _(const typename SIMD<_SIMDType>::Float &_x, const typename SIMD<_SIMDType>::Float &_y, const typename SIMD<_SIMDType>::Float &_z)
    {//Euclidean
		SIMD_HELPERS

        return simd::mulAdd(_x, _x, simd::mulAdd(_y, _y, simd::mulf(_z, _z)));
    }
};

template<SIMDType _SIMDType>
struct Distance<_SIMDType, CellularDistance::Manhattan>
{
    static typename SIMD<_SIMDType>::Float _(const typename SIMD<_SIMDType>::Float &_x, const typename SIMD<_SIMDType>::Float &_y, const typename SIMD<_SIMDType>::Float &_z)
    {
		SIMD_HELPERS

        return simd::add(simd::add(simd::abs(_x), simd::abs(_y)), simd::abs(_z));
    }
};

template<SIMDType _SIMDType>
struct Distance<_SIMDType, CellularDistance::Natural>
{
    static typename SIMD<_SIMDType>::Float _(const typename SIMD<_SIMDType>::Float &_x, const typename SIMD<_SIMDType>::Float &_y, const typename SIMD<_SIMDType>::Float &_z)
    {
		SIMD_HELPERS

        return simd::add(Distance<_SIMDType, CellularDistance::Euclidean>::_(_x, _y, _z), Distance<_SIMDType, CellularDistance::Manhattan>::_(_x, _y, _z));
    }
};

template<SIMDType _SIMDType, CellularReturnType _CellularReturnType>
struct ReturnDistance
{
    static typename SIMD<_SIMDType>::Float _(const typename SIMD<_SIMDType>::Float &distance, const typename SIMD<_SIMDType>::Float &distance2)
    {
        return distance;
    }
};

template<SIMDType _SIMDType>
struct ReturnDistance<_SIMDType, CellularReturnType::Distance2>
{
    static typename SIMD<_SIMDType>::Float _(const typename SIMD<_SIMDType>::Float &distance, const typename SIMD<_SIMDType>::Float &distance2)
    {
        return distance2;
    }
};

template<SIMDType _SIMDType>
struct ReturnDistance<_SIMDType, CellularReturnType::Distance2Add>
{
    static typename SIMD<_SIMDType>::Float _(const typename SIMD<_SIMDType>::Float &distance, const typename SIMD<_SIMDType>::Float &distance2)
    {
		SIMD_HELPERS

        return simd::add(distance, distance2);
    }
};

template<SIMDType _SIMDType>
struct ReturnDistance<_SIMDType, CellularReturnType::Distance2Sub>
{
    static typename SIMD<_SIMDType>::Float _(const typename SIMD<_SIMDType>::Float &distance, const typename SIMD<_SIMDType>::Float &distance2)
    {
		SIMD_HELPERS

        return simd::sub(distance2, distance);
    }
};

template<SIMDType _SIMDType>
struct ReturnDistance<_SIMDType, CellularReturnType::Distance2Mul>
{
    static typename SIMD<_SIMDType>::Float _(const typename SIMD<_SIMDType>::Float &distance, const typename SIMD<_SIMDType>::Float &distance2)
    {
		SIMD_HELPERS

        return simd::mulf(distance, distance2);
    }
};

template<SIMDType _SIMDType>
struct ReturnDistance<_SIMDType, CellularReturnType::Distance2Div>
{
    static typename SIMD<_SIMDType>::Float _(const typename SIMD<_SIMDType>::Float &distance, const typename SIMD<_SIMDType>::Float &distance2)
    {
		SIMD_HELPERS

        return simd::div(distance, distance2);
    }
};

template<SIMDType _SIMDType>
struct NoiseLookupSettings
{
    NoiseType type;
    typename SIMD<_SIMDType>::Float frequency;
    FractalType fractalType;
    int fractalOctaves;
    typename SIMD<_SIMDType>::Float fractalLacunarity;
    typename SIMD<_SIMDType>::Float fractalGain;
    typename SIMD<_SIMDType>::Float fractalBounding;
};

//template<SIMDType _SIMDType, NoiseType _NoiseType, FractalType _FractalType>
//typename SIMD<_SIMDType>::Float FractalCelluarLookup(const typename SIMD<_SIMDType>::Int &seedV, const typename SIMD<_SIMDType>::Float &xF, const typename SIMD<_SIMDType>::Float &yF, const typename SIMD<_SIMDType>::Float &zF, const NoiseLookupSettings<_SIMDType>& noiseLookupSettings)
//{
//    typename SIMD<_SIMDType>::Float lacunarityV = noiseLookupSettings.fractalLacunarity;
//    typename SIMD<_SIMDType>::Float gainV = noiseLookupSettings.fractalGain;
//    typename SIMD<_SIMDType>::Float fractalBoundingV = noiseLookupSettings.fractalBounding;
//    int m_octaves = noiseLookupSettings.fractalOctaves;
//
//    return GetValue<_SIMDType, _NoiseType, _FractalType>::_(seedV, xF, yF, zF);
//}

//CellularReturnType::Value
//template<SIMDType _SIMDType, CellularReturnType _Type, typename ..._Types>
//struct ReturnValue
//{
//
//};
//
//template<SIMDType _SIMDType, CellularReturnType _Type>
//struct ReturnValue
//{
//    using Type=typename SIMD<_SIMDType>::Float;
//};

template<SIMDType _SIMDType, CellularDistance _CellularDistance, CellularReturnType _CellularReturnType>
static typename SIMD<_SIMDType>::Float VECTORCALL CellularDistanceSingle(typename SIMD<_SIMDType>::Int seed, typename SIMD<_SIMDType>::Float x, typename SIMD<_SIMDType>::Float y, typename SIMD<_SIMDType>::Float z, typename SIMD<_SIMDType>::Float cellJitter, int index0, int index1)
{
	SIMD_HELPERS

    Float distance;
    Float cellValue;
    Float valueArray[HN_CELLULAR_INDEX_MAX+1];
    Float distanceArray[HN_CELLULAR_INDEX_MAX+1];

//    ReturnValue<_SIMDType, args...> value;

    if(_CellularReturnType==CellularReturnType::Value)
    {
        cellValue=Constant::numf_0;
        distance=Constant::numf_999999;
    }
    if(_CellularReturnType==CellularReturnType::Distance)
        distance=Constant::numf_999999;
    else if(_CellularReturnType==CellularReturnType::ValueDistance2)
    {
        valueArray[0]=Constant::numf_0;
        valueArray[1]=Constant::numf_0;
        valueArray[2]=Constant::numf_0;
        valueArray[3]=Constant::numf_0;

        distanceArray[0]=Constant::numf_999999;
        distanceArray[1]=Constant::numf_999999;
        distanceArray[2]=Constant::numf_999999;
        distanceArray[3]=Constant::numf_999999;
    }
    else
    {
        distanceArray[0]=Constant::numf_999999;
        distanceArray[1]=Constant::numf_999999;
        distanceArray[2]=Constant::numf_999999;
        distanceArray[3]=Constant::numf_999999;
    }

    Int xc=simd::sub(simd::convert(x), Constant::numi_1);
    Int ycBase=simd::sub(simd::convert(y), Constant::numi_1);
    Int zcBase=simd::sub(simd::convert(z), Constant::numi_1);

    Float xcf=simd::sub(simd::convert(xc), x);
    Float ycfBase=simd::sub(simd::convert(ycBase), y);
    Float zcfBase=simd::sub(simd::convert(zcBase), z);

    xc=simd::mul(xc, Constant::numi_xPrime);
    ycBase=simd::mul(ycBase, Constant::numi_yPrime);
    zcBase=simd::mul(zcBase, Constant::numi_zPrime);

    for(int xi=0; xi < 3; xi++)
    {
        Float ycf=ycfBase;
        Int yc=ycBase;
        for(int yi=0; yi < 3; yi++)
        {
            Float zcf=zcfBase;
            Int zc=zcBase;
            for(int zi=0; zi < 3; zi++)
            {
                Int hash=HashHB<_SIMDType>(seed, xc, yc, zc);
                Float xd=simd::sub(simd::convert(simd::_and(hash, Constant::numi_bit10Mask)), Constant::numf_511_5);
                Float yd=simd::sub(simd::convert(simd::_and(simd::shiftR(hash, 10), Constant::numi_bit10Mask)), Constant::numf_511_5);
                Float zd=simd::sub(simd::convert(simd::_and(simd::shiftR(hash, 20), Constant::numi_bit10Mask)), Constant::numf_511_5);

                Float invMag=simd::mulf(cellJitter, simd::invSqrt(simd::mulAdd(xd, xd, simd::mulAdd(yd, yd, simd::mulf(zd, zd)))));

                xd=simd::mulAdd(xd, invMag, xcf);
                yd=simd::mulAdd(yd, invMag, ycf);
                zd=simd::mulAdd(zd, invMag, zcf);

                Float newDistance=Distance<_SIMDType, _CellularDistance>::_(xd, yd, zd);

                if(_CellularReturnType == CellularReturnType::Value)
                {
                    Float newCellValue=simd::mulf(Constant::numf_hash2Float, simd::convert(hash));
                    Mask closer=simd::lessThan(newDistance, distance);
                    cellValue=simd::blend(cellValue, newCellValue, closer);

                    distance=simd::min(distance, newDistance);
                }
                else if(_CellularReturnType == CellularReturnType::Distance)
                {
                    distance=simd::min(distance, newDistance);
                }
                else if(_CellularReturnType==CellularReturnType::ValueDistance2)
                {
                    Float newCellValue=simd::mulf(Constant::numf_hash2Float, simd::convert(hash));
                    Mask mask;

                    for(int i=index1; i>0; i--)
                    {
                        mask=simd::lessThan(newDistance, distanceArray[i]);
                        
                        Float currentValue=simd::blend(valueArray[i], newCellValue, mask);
                        Float minDistance=simd::min(distanceArray[i], newDistance);

                        mask=simd::greaterThan(minDistance, distanceArray[i-1]);
                        valueArray[i]=simd::blend(valueArray[i-1], currentValue, mask);
                        distanceArray[i]=simd::max(minDistance, distanceArray[i-1]);
                    }
                    mask=simd::lessThan(newDistance, distanceArray[0]);
                    valueArray[0]=simd::blend(valueArray[0], newCellValue, mask);
                    distanceArray[0]=simd::min(distanceArray[0], newDistance);
                }
                else
                {
                    for(int i=index1; i>0; i--)
                        distanceArray[i]=simd::max(simd::min(distanceArray[i], newDistance), distanceArray[i-1]);
                    distanceArray[0]=simd::min(distanceArray[0], newDistance);
                }

                zcf=simd::add(zcf, Constant::numf_1);
                zc=simd::add(zc, Constant::numi_zPrime);
            }
            ycf=simd::add(ycf, Constant::numf_1);
            yc=simd::add(yc, Constant::numi_yPrime);
        }
        xcf=simd::add(xcf, Constant::numf_1);
        xc=simd::add(xc, Constant::numi_xPrime);
    }

    if(_CellularReturnType == CellularReturnType::Value)
        return cellValue;
    else if(_CellularReturnType == CellularReturnType::Distance)
        return distance;
    else if(_CellularReturnType == CellularReturnType::ValueDistance2)
        return valueArray[index1];
    else
        return ReturnDistance<_SIMDType, _CellularReturnType>::_(distanceArray[index0], distanceArray[index1]);
    return Constant::numf_0;
}

////////////////////////////////////////////////////////////////////////////////
//getValue functions
////////////////////////////////////////////////////////////////////////////////
template<SIMDType _SIMDType, NoiseType _NoiseType, FractalType _FractalType, CellularDistance _CellularDistance, CellularReturnType _CellularReturnType, NoiseType _LookupNoiseType>
struct GetValue
{
    template<typename ..._Types>
    static typename SIMD<_SIMDType>::Float _(const NoiseValues<_SIMDType> &noise, typename SIMD<_SIMDType>::Float &xF, typename SIMD<_SIMDType>::Float &yF, typename SIMD<_SIMDType>::Float &zF, _Types ...args)
    { return CellularDistanceSingle<_SIMDType, _CellularDistance, _CellularReturnType>(noise.seedV, xF, yF, zF, noise.cellJitterV, noise.index0, noise.index1); }
};

template<SIMDType _SIMDType, NoiseType _NoiseType>
struct GetValue<_SIMDType, _NoiseType, FractalType::None, CellularDistance::None, CellularReturnType::None, NoiseType::None>
{
    template<typename ..._Types>
    static typename SIMD<_SIMDType>::Float _(const NoiseValues<_SIMDType> &noise, typename SIMD<_SIMDType>::Float &xF, typename SIMD<_SIMDType>::Float &yF, typename SIMD<_SIMDType>::Float &zF)
    { return Single<_SIMDType, _NoiseType>::_(noise.seedV, xF, yF, zF); }
};

template<SIMDType _SIMDType, NoiseType _NoiseType>
struct GetValue<_SIMDType, _NoiseType, FractalType::FBM, CellularDistance::None, CellularReturnType::None, NoiseType::None>
{
    template<typename ..._Types>
    static typename SIMD<_SIMDType>::Float _(const NoiseValues<_SIMDType> &noise, typename SIMD<_SIMDType>::Float &xF, typename SIMD<_SIMDType>::Float &yF, typename SIMD<_SIMDType>::Float &zF)
    { return FBMSingle<_SIMDType, _NoiseType>(noise, xF, yF, zF); }
};

template<SIMDType _SIMDType, NoiseType _NoiseType>
struct GetValue<_SIMDType, _NoiseType, FractalType::Billow, CellularDistance::None, CellularReturnType::None, NoiseType::None>
{
    template<typename ..._Types>
    static typename SIMD<_SIMDType>::Float _(const NoiseValues<_SIMDType> &noise, typename SIMD<_SIMDType>::Float &xF, typename SIMD<_SIMDType>::Float &yF, typename SIMD<_SIMDType>::Float &zF)
    { return BillowSingle<_SIMDType, _NoiseType>(noise, xF, yF, zF); }
};

template<SIMDType _SIMDType, NoiseType _NoiseType>
struct GetValue<_SIMDType, _NoiseType, FractalType::RigidMulti, CellularDistance::None, CellularReturnType::None, NoiseType::None>
{
    template<typename ..._Types>
    static typename SIMD<_SIMDType>::Float _(const NoiseValues<_SIMDType> &noise, typename SIMD<_SIMDType>::Float &xF, typename SIMD<_SIMDType>::Float &yF, typename SIMD<_SIMDType>::Float &zF)
    { return RigidMultiSingle<_SIMDType, _NoiseType>(noise, xF, yF, zF); }
};

template<SIMDType _SIMDType, NoiseType _NoiseType, FractalType _FractalType, CellularDistance _CellularDistance>
static typename SIMD<_SIMDType>::Float VECTORCALL CellularLookupSingle(const NoiseValues<_SIMDType> &noise, typename SIMD<_SIMDType>::Float x, typename SIMD<_SIMDType>::Float y, typename SIMD<_SIMDType>::Float z, typename SIMD<_SIMDType>::Float cellJitter)
{
	SIMD_HELPERS

    Float distance=Constant::numf_999999;
    Float xCell=simd::undefinedFloat();
    Float yCell=simd::undefinedFloat();
    Float zCell=simd::undefinedFloat();

    Int xc=simd::sub(simd::convert(x), Constant::numi_1);
    Int ycBase=simd::sub(simd::convert(y), Constant::numi_1);
    Int zcBase=simd::sub(simd::convert(z), Constant::numi_1);

    Float xcf=simd::convert(xc);
    Float ycfBase=simd::convert(ycBase);
    Float zcfBase=simd::convert(zcBase);

    xc=simd::mul(xc, Constant::numi_xPrime);
    ycBase=simd::mul(ycBase, Constant::numi_yPrime);
    zcBase=simd::mul(zcBase, Constant::numi_zPrime);

    for(int xi=0; xi < 3; xi++)
    {
        Float ycf=ycfBase;
        Int yc=ycBase;
        Float xLocal=simd::sub(xcf, x);
        for(int yi=0; yi < 3; yi++)
        {
            Float zcf=zcfBase;
            Int zc=zcBase;
            Float yLocal=simd::sub(ycf, y);
            for(int zi=0; zi < 3; zi++)
            {
                Float zLocal=simd::sub(zcf, z);

                Int hash=HashHB<_SIMDType>(noise.seedV, xc, yc, zc);
                Float xd=simd::sub(simd::convert(simd::_and(hash, Constant::numi_bit10Mask)), Constant::numf_511_5);
                Float yd=simd::sub(simd::convert(simd::_and(simd::shiftR(hash, 10), Constant::numi_bit10Mask)), Constant::numf_511_5);
                Float zd=simd::sub(simd::convert(simd::_and(simd::shiftR(hash, 20), Constant::numi_bit10Mask)), Constant::numf_511_5);

                Float invMag=simd::mulf(noise.cellJitterV, simd::invSqrt(simd::mulAdd(xd, xd, simd::mulAdd(yd, yd, simd::mulf(zd, zd)))));

                Float xCellNew=simd::mulf(xd, invMag);
                Float yCellNew=simd::mulf(yd, invMag);
                Float zCellNew=simd::mulf(zd, invMag);

                xd=simd::add(xCellNew, xLocal);
                yd=simd::add(yCellNew, yLocal);
                zd=simd::add(zCellNew, zLocal);

                xCellNew=simd::add(xCellNew, xcf);
                yCellNew=simd::add(yCellNew, ycf);
                zCellNew=simd::add(zCellNew, zcf);

                Float newDistance=Distance<_SIMDType, _CellularDistance>::_(xd, yd, zd);

                Mask closer=simd::lessThan(newDistance, distance);

                distance=simd::min(newDistance, distance);
                xCell=simd::blend(xCell, xCellNew, closer);
                yCell=simd::blend(yCell, yCellNew, closer);
                zCell=simd::blend(zCell, zCellNew, closer);

                zcf=simd::add(zcf, Constant::numf_1);
                zc=simd::add(zc, Constant::numi_zPrime);
            }
            ycf=simd::add(ycf, Constant::numf_1);
            yc=simd::add(yc, Constant::numi_yPrime);
        }
        xcf=simd::add(xcf, Constant::numf_1);
        xc=simd::add(xc, Constant::numi_xPrime);
    }

    Float xF=simd::mulf(xCell, noise.cellularLookupFrequencyV);
    Float yF=simd::mulf(yCell, noise.cellularLookupFrequencyV);
    Float zF=simd::mulf(zCell, noise.cellularLookupFrequencyV);

    Float result=GetValue<_SIMDType, _NoiseType, _FractalType, CellularDistance::None, CellularReturnType::None, NoiseType::None>::_(noise, xF, yF, zF);
    return result;
}

template<SIMDType _SIMDType, NoiseType _NoiseType, FractalType _FractalType, CellularDistance _CellularDistance, NoiseType _LookupNoiseType>
struct GetValue<_SIMDType, _NoiseType, _FractalType, _CellularDistance, CellularReturnType::NoiseLookup, _LookupNoiseType>
{
    template<typename ..._Types>
    static typename SIMD<_SIMDType>::Float _(const NoiseValues<_SIMDType> &noise, typename SIMD<_SIMDType>::Float &xF, typename SIMD<_SIMDType>::Float &yF, typename SIMD<_SIMDType>::Float &zF)
    {
		SIMD_HELPERS

        CellularLookupSingle<_SIMDType, _LookupNoiseType, _FractalType, _CellularDistance>(noise, xF, yF, zF, noise.cellJitterV);
        return simd::zeroFloat();
    }
};

////////////////////////////////////////////////////////////////////////////////
//build functions
////////////////////////////////////////////////////////////////////////////////
template<SIMDType _SIMDType, NoiseType _NoiseType, PerturbType _PerturbType, FractalType _FractalType, CellularDistance _CellularDistance, CellularReturnType _CellularReturnType, NoiseType _LookupNoiseType, BuildType buildType>
struct Build
{
    template<typename... _Types>
    static void _(const NoiseValues<_SIMDType> &noise, const PerturbValues<_SIMDType> &perturb,
        float *noiseSet, int xStart, int yStart, int zStart, int xSize, int ySize, int zSize, _Types... args)
    {
		SIMD_HELPERS

        if((zSize & (simd::vectorSize()-1))==0)
        {
            Int yBase=simd::set(yStart);
            Int zBase=simd::add(Constant::numi_incremental, simd::set(zStart));
            Int x=simd::set(xStart);

            size_t index=0;

            for(size_t ix=0; ix<xSize; ix++)
            {
                Float xf=simd::mulf(simd::convert(x), noise.xFreqV);
                Int y=yBase;

                for(size_t iy=0; iy<ySize; iy++)
                {
                    Float yf=simd::mulf(simd::convert(y), noise.yFreqV);
                    Int z=zBase;
                    Float xF=xf;
                    Float yF=yf;
                    Float zF=simd::mulf(simd::convert(z), noise.zFreqV);

                    Perturb<_SIMDType, _PerturbType>::_(noise.seedV, perturb, xF, yF, zF);
                    Float result=GetValue<_SIMDType, _NoiseType, _FractalType, _CellularDistance, _CellularReturnType, _LookupNoiseType>::_(noise, xF, yF, zF, args...);
                    simd::store(&noiseSet[index], result);

                    size_t iz=simd::vectorSize();
                    while(iz<zSize)
                    {
                        z=simd::add(z, Constant::numi_vectorSize);
                        index+=simd::vectorSize();
                        iz+=simd::vectorSize();
                        xF=xf;
                        yF=yf;
                        zF=simd::mulf(simd::convert(z), noise.zFreqV);

                        Perturb<_SIMDType, _PerturbType>::_(noise.seedV, perturb, xF, yF, zF);
                        Float result=GetValue<_SIMDType, _NoiseType, _FractalType, _CellularDistance, _CellularReturnType, _LookupNoiseType>::_(noise, xF, yF, zF, args...);
                        simd::store(&noiseSet[index], result);
                    }
                    index+=simd::vectorSize();
                    y=simd::add(y, Constant::numi_1);
                }
                x=simd::add(x, Constant::numi_1);
            }
        }
        else
        {
            Int ySizeV=simd::set(ySize);
            Int zSizeV=simd::set(zSize);

            Int yEndV=simd::set(yStart+ySize-1);
            Int zEndV=simd::set(zStart+zSize-1);

            Int x=simd::set(xStart);
            Int y=simd::set(yStart);
            Int z=simd::add(simd::set(zStart), Constant::numi_incremental);

            axisReset<_SIMDType>(x, y, z, ySizeV, yEndV, zSizeV, zEndV, zSize, 1);

            size_t index=0;
            int maxIndex=xSize * ySize * zSize;

            for(; index<maxIndex-simd::vectorSize(); index+=simd::vectorSize())
            {
                Float xF=simd::mulf(simd::convert(x), noise.xFreqV);
                Float yF=simd::mulf(simd::convert(y), noise.yFreqV);
                Float zF=simd::mulf(simd::convert(z), noise.zFreqV);

                Perturb<_SIMDType, _PerturbType>::_(noise.seedV, perturb, xF, yF, zF);
                Float result=GetValue<_SIMDType, _NoiseType, _FractalType, _CellularDistance, _CellularReturnType, _LookupNoiseType>::_(noise, xF, yF, zF, args...);

                simd::store(&noiseSet[index], result);

                z=simd::add(z, Constant::numi_vectorSize);

                axisReset<_SIMDType>(x, y, z, ySizeV, yEndV, zSizeV, zEndV, zSize, 0);
            }

            Float xF=simd::mulf(simd::convert(x), noise.xFreqV);
            Float yF=simd::mulf(simd::convert(y), noise.yFreqV);
            Float zF=simd::mulf(simd::convert(z), noise.zFreqV);

            Perturb<_SIMDType, _PerturbType>::_(noise.seedV, perturb, xF, yF, zF);
            Float result=GetValue<_SIMDType, _NoiseType, _FractalType, _CellularDistance, _CellularReturnType, _LookupNoiseType>::_(noise, xF, yF, zF, args...);
            STORE_LAST_RESULT(&noiseSet[index], result);
        }
    }
};

template<SIMDType _SIMDType, NoiseType _NoiseType, PerturbType _PerturbType, FractalType _FractalType, CellularDistance _CellularDistance, CellularReturnType _CellularReturnType, NoiseType _LookupNoiseType>
struct Build<_SIMDType, _NoiseType, _PerturbType, _FractalType, _CellularDistance, _CellularReturnType, _LookupNoiseType, BuildType::Vector>
{
    template<typename... _Types>
    static void _(const NoiseValues<_SIMDType> &noise, const PerturbValues<_SIMDType> &perturb, float* noiseSet, VectorSet* vectorSet,
        const typename SIMD<_SIMDType>::Float &xOffsetV, const typename SIMD<_SIMDType>::Float &yOffsetV, const typename SIMD<_SIMDType>::Float &zOffsetV, _Types... args)
    {
		SIMD_HELPERS

        size_t index=0;
        size_t loopMax=vectorSet->size SIZE_MASK;

        while(index<loopMax)
        {
            Float xF=simd::mulAdd(simd::load(&vectorSet->xSet[index]), noise.xFreqV, xOffsetV);
            Float yF=simd::mulAdd(simd::load(&vectorSet->ySet[index]), noise.yFreqV, yOffsetV);
            Float zF=simd::mulAdd(simd::load(&vectorSet->zSet[index]), noise.zFreqV, zOffsetV);

            Perturb<_SIMDType, _PerturbType>::_(noise.seedV, perturb, xF, yF, zF);
            Float result=GetValue<_SIMDType, _NoiseType, _FractalType, _CellularDistance, _CellularReturnType, _LookupNoiseType>::_(noise, xF, yF, zF);
            simd::store(&noiseSet[index], result);
            index+=simd::vectorSize();
        }

#ifndef HN_ALIGNED_SETS
        if(loopMax!=vectorSet->size)
        {
            std::size_t remaining=(vectorSet->size-loopMax)*4;

            Float xF=simd::load(&vectorSet->xSet[loopMax]);
            Float yF=simd::load(&vectorSet->ySet[loopMax]);
            Float zF=simd::load(&vectorSet->zSet[loopMax]);

            xF=simd::mulAdd(xF, noise.xFreqV, xOffsetV);
            yF=simd::mulAdd(yF, noise.yFreqV, yOffsetV);
            zF=simd::mulAdd(zF, noise.zFreqV, zOffsetV);

            Float result=GetValue<_SIMDType, _NoiseType, _FractalType, _LookupNoiseType>::_(noise, xF, yF, zF);
            std::memcpy(&noiseSet[index], &result, remaining);
        }
#endif
    }
};

template<SIMDType _SIMDType, BuildType _BuildType, PerturbType _PerturbType, FractalType _FractalType, CellularDistance _CellularDistance, CellularReturnType _CellularReturnType, NoiseType _LookupNoiseType, typename... _Types>
static void CallBuild(NoiseType noiseType, _Types... args)
{
    switch(noiseType)
    {
    case NoiseType::None:
        break;
    case NoiseType::Value:
    case NoiseType::ValueFractal:
        Build<_SIMDType, NoiseType::Value, _PerturbType, _FractalType, _CellularDistance, _CellularReturnType, _LookupNoiseType, _BuildType>::_(args...);
        break;
    case NoiseType::Perlin:
    case NoiseType::PerlinFractal:
        Build<_SIMDType, NoiseType::Perlin, _PerturbType, _FractalType, _CellularDistance, _CellularReturnType, _LookupNoiseType, _BuildType>::_(args...);
        break;
    case NoiseType::Simplex:
    case NoiseType::SimplexFractal:
        Build<_SIMDType, NoiseType::Simplex, _PerturbType, _FractalType, _CellularDistance, _CellularReturnType, _LookupNoiseType, _BuildType>::_(args...);
        break;
	case NoiseType::OpenSimplex2:
	case NoiseType::OpenSimplex2Fractal:
		Build<_SIMDType, NoiseType::OpenSimplex2, _PerturbType, _FractalType, _CellularDistance, _CellularReturnType, _LookupNoiseType, _BuildType>::_(args...);
		break;
    case NoiseType::WhiteNoise:
        Build<_SIMDType, NoiseType::WhiteNoise, _PerturbType, _FractalType, _CellularDistance, _CellularReturnType, _LookupNoiseType, _BuildType>::_(args...);
        break;
    case NoiseType::Cellular:
        Build<_SIMDType, NoiseType::Cellular, _PerturbType, _FractalType, _CellularDistance, _CellularReturnType, _LookupNoiseType, _BuildType>::_(args...);
        break;
    case NoiseType::Cubic:
    case NoiseType::CubicFractal:
        Build<_SIMDType, NoiseType::Cubic, _PerturbType, _FractalType, _CellularDistance, _CellularReturnType, _LookupNoiseType, _BuildType>::_(args...);
        break;
    }
}

template<SIMDType _SIMDType, BuildType _BuildType, FractalType _FractalType, CellularDistance _CellularDistance, CellularReturnType _CellularReturnType, NoiseType _LookupNoiseType, typename... _Types>
static void CallBuild(NoiseType noiseType, PerturbType perturbType, _Types... args)
{
    switch(perturbType)
    {
    case PerturbType::None:
        CallBuild<_SIMDType, _BuildType, PerturbType::None, _FractalType, _CellularDistance, _CellularReturnType, _LookupNoiseType>(noiseType, args...);
        break;
    case PerturbType::Gradient:
        CallBuild<_SIMDType, _BuildType, PerturbType::Gradient, _FractalType, _CellularDistance, _CellularReturnType, _LookupNoiseType>(noiseType, args...);
        break;
    case PerturbType::GradientFractal:
        CallBuild<_SIMDType, _BuildType, PerturbType::GradientFractal, _FractalType, _CellularDistance, _CellularReturnType, _LookupNoiseType>(noiseType, args...);
        break;
    case PerturbType::Normalise:
        CallBuild<_SIMDType, _BuildType, PerturbType::Normalise, _FractalType, _CellularDistance, _CellularReturnType, _LookupNoiseType>(noiseType, args...);
        break;
    case PerturbType::Gradient_Normalise:
        CallBuild<_SIMDType, _BuildType, PerturbType::Gradient_Normalise, _FractalType, _CellularDistance, _CellularReturnType, _LookupNoiseType>(noiseType, args...);
        break;
    case PerturbType::GradientFractal_Normalise:
        CallBuild<_SIMDType, _BuildType, PerturbType::GradientFractal_Normalise, _FractalType, _CellularDistance, _CellularReturnType, _LookupNoiseType>(noiseType, args...);
        break;
    }
}

template<SIMDType _SIMDType, BuildType _BuildType, CellularDistance _cellularDistance, CellularReturnType _cellularReturnType, NoiseType _LookupNoiseType, typename... _Types>
static void CallBuildFractal(NoiseType noiseType, PerturbType perturbType, FractalType fractalType, _Types... args)
{
    switch(fractalType)
    {
    case FractalType::None:
        //        CallBuild<_SIMDType, _BuildType, FractalType::None, _cellularDistance, _cellularReturnType, _LookupNoiseType>(noiseType, perturbType, args...);
        break;
    case FractalType::FBM:
        CallBuild<_SIMDType, _BuildType, FractalType::FBM, _cellularDistance, _cellularReturnType, _LookupNoiseType>(noiseType, perturbType, args...);
        break;
    case FractalType::Billow:
        CallBuild<_SIMDType, _BuildType, FractalType::Billow, _cellularDistance, _cellularReturnType, _LookupNoiseType>(noiseType, perturbType, args...);
        break;
    case FractalType::RigidMulti:
        CallBuild<_SIMDType, _BuildType, FractalType::RigidMulti, _cellularDistance, _cellularReturnType, _LookupNoiseType>(noiseType, perturbType, args...);
        break;
    }
}

template<SIMDType _SIMDType, BuildType _BuildType, PerturbType _PerturbType, CellularReturnType _CellularReturnType, typename... _Types>
static void CallBuildCellular(CellularDistance cellularDistance, _Types... args)
{
    switch(cellularDistance)
    {
    case CellularDistance::None:
        Build<_SIMDType, NoiseType::Cellular, _PerturbType, FractalType::None, CellularDistance::None, _CellularReturnType, NoiseType::None, _BuildType>::_(args...);
        break;
    case CellularDistance::Euclidean:
        Build<_SIMDType, NoiseType::Cellular, _PerturbType, FractalType::None, CellularDistance::Euclidean, _CellularReturnType, NoiseType::None, _BuildType>::_(args...);
        break;
    case CellularDistance::Manhattan:
        Build<_SIMDType, NoiseType::Cellular, _PerturbType, FractalType::None, CellularDistance::Manhattan, _CellularReturnType, NoiseType::None, _BuildType>::_(args...);
        break;
    case CellularDistance::Natural:
        Build<_SIMDType, NoiseType::Cellular, _PerturbType, FractalType::None, CellularDistance::Natural, _CellularReturnType, NoiseType::None, _BuildType>::_(args...);
        break;
    }
}

template<SIMDType _SIMDType, BuildType _BuildType, CellularReturnType _CellularReturnType, NoiseType _lookupNoiseType, typename... _Types>
static void CallBuildCellularLookup(NoiseType noiseType, PerturbType perturbType, FractalType fractalType, CellularDistance cellularDistance, _Types... args)
{
    switch(cellularDistance)
    {
    case CellularDistance::None:
        CallBuildFractal<_SIMDType, _BuildType, CellularDistance::None, _CellularReturnType, _lookupNoiseType>(noiseType, perturbType, fractalType, args...);
        break;
    case CellularDistance::Euclidean:
        CallBuildFractal<_SIMDType, _BuildType, CellularDistance::Euclidean, _CellularReturnType, _lookupNoiseType>(noiseType, perturbType, fractalType, args...);
        break;
    case CellularDistance::Manhattan:
        CallBuildFractal<_SIMDType, _BuildType, CellularDistance::Manhattan, _CellularReturnType, _lookupNoiseType>(noiseType, perturbType, fractalType, args...);
        break;
    case CellularDistance::Natural:
        CallBuildFractal<_SIMDType, _BuildType, CellularDistance::Natural, _CellularReturnType, _lookupNoiseType>(noiseType, perturbType, fractalType, args...);
        break;
    }
}

template<SIMDType _SIMDType, BuildType _BuildType, CellularReturnType _cellularReturnType, typename... _Types>
static void CallBuildCellularLookup(NoiseType noiseType, PerturbType perturbType, FractalType fractalType, CellularDistance cellularDistance, NoiseType lookupNoiseType, _Types... args)
{
    switch(lookupNoiseType)
    {
    case NoiseType::None:
        CallBuildCellularLookup<_SIMDType, _BuildType, _cellularReturnType, NoiseType::None>(noiseType, perturbType, fractalType, cellularDistance, args...);
        break;
    case NoiseType::Value:
        CallBuildCellularLookup<_SIMDType, _BuildType, _cellularReturnType, NoiseType::Value>(noiseType, perturbType, fractalType, cellularDistance, args...);
        break;
    case NoiseType::ValueFractal:
        CallBuildCellularLookup<_SIMDType, _BuildType, _cellularReturnType, NoiseType::ValueFractal>(noiseType, perturbType, fractalType, cellularDistance, args...);
        break;
    case NoiseType::Perlin:
        CallBuildCellularLookup<_SIMDType, _BuildType, _cellularReturnType, NoiseType::Perlin>(noiseType, perturbType, fractalType, cellularDistance, args...);
        break;
    case NoiseType::PerlinFractal:
        CallBuildCellularLookup<_SIMDType, _BuildType, _cellularReturnType, NoiseType::PerlinFractal>(noiseType, perturbType, fractalType, cellularDistance, args...);
        break;
    case NoiseType::Simplex:
        CallBuildCellularLookup<_SIMDType, _BuildType, _cellularReturnType, NoiseType::Simplex>(noiseType, perturbType, fractalType, cellularDistance, args...);
        break;
    case NoiseType::SimplexFractal:
        CallBuildCellularLookup<_SIMDType, _BuildType, _cellularReturnType, NoiseType::SimplexFractal>(noiseType, perturbType, fractalType, cellularDistance, args...);
        break;
    case NoiseType::OpenSimplex2:
        CallBuildCellularLookup<_SIMDType, _BuildType, _cellularReturnType, NoiseType::OpenSimplex2>(noiseType, perturbType, fractalType, cellularDistance, args...);
        break;
    case NoiseType::OpenSimplex2Fractal:
        CallBuildCellularLookup<_SIMDType, _BuildType, _cellularReturnType, NoiseType::OpenSimplex2Fractal>(noiseType, perturbType, fractalType, cellularDistance, args...);
        break;
    case NoiseType::WhiteNoise:
        CallBuildCellularLookup<_SIMDType, _BuildType, _cellularReturnType, NoiseType::WhiteNoise>(noiseType, perturbType, fractalType, cellularDistance, args...);
        break;
    case NoiseType::Cellular:
        CallBuildCellularLookup<_SIMDType, _BuildType, _cellularReturnType, NoiseType::Cellular>(noiseType, perturbType, fractalType, cellularDistance, args...);
        break;
    case NoiseType::Cubic:
        CallBuildCellularLookup<_SIMDType, _BuildType, _cellularReturnType, NoiseType::Cubic>(noiseType, perturbType, fractalType, cellularDistance, args...);
        break;
    case NoiseType::CubicFractal:
        CallBuildCellularLookup<_SIMDType, _BuildType, _cellularReturnType, NoiseType::CubicFractal>(noiseType, perturbType, fractalType, cellularDistance, args...);
        break;
    }
}

template<SIMDType _SIMDType, BuildType _BuildType, PerturbType _PerturbType, typename... _Types>
static void CallBuildCellular(CellularDistance cellularDistance, CellularReturnType cellularReturnType, NoiseType lookupNoiseType, _Types... args)
{
    switch(cellularReturnType)
    {
    case CellularReturnType::None:
        CallBuildCellular<_SIMDType, _BuildType, _PerturbType, CellularReturnType::None>(cellularDistance, args...);
        break;
    case CellularReturnType::Value:
        CallBuildCellular<_SIMDType, _BuildType, _PerturbType, CellularReturnType::Value>(cellularDistance, args...);
        break;
    case CellularReturnType::Distance:
        CallBuildCellular<_SIMDType, _BuildType, _PerturbType, CellularReturnType::Distance>(cellularDistance, args...);
        break;
    case CellularReturnType::ValueDistance2:
        CallBuildCellular<_SIMDType, _BuildType, _PerturbType, CellularReturnType::ValueDistance2>(cellularDistance, args...);
        break;
    case CellularReturnType::Distance2:
        CallBuildCellular<_SIMDType, _BuildType, _PerturbType, CellularReturnType::Distance2>(cellularDistance, args...);
        break;
    case CellularReturnType::Distance2Add:
        CallBuildCellular<_SIMDType, _BuildType, _PerturbType, CellularReturnType::Distance2Add>(cellularDistance, args...);
        break;
    case CellularReturnType::Distance2Sub:
        CallBuildCellular<_SIMDType, _BuildType, _PerturbType, CellularReturnType::Distance2Sub>(cellularDistance, args...);
        break;
    case CellularReturnType::Distance2Mul:
        CallBuildCellular<_SIMDType, _BuildType, _PerturbType, CellularReturnType::Distance2Mul>(cellularDistance, args...);
        break;
    case CellularReturnType::Distance2Div:
        CallBuildCellular<_SIMDType, _BuildType, _PerturbType, CellularReturnType::Distance2Div>(cellularDistance, args...);
        break;
    case CellularReturnType::NoiseLookup:
        //        CallBuildCellularLookup<_SIMDType, _BuildType, CellularReturnType::NoiseLookup>(noiseType, perturbType, fractalType, cellularDistance, lookupNoiseType, args...);
        break;
    case CellularReturnType::Distance2Cave:
        CallBuildCellular<_SIMDType, _BuildType, _PerturbType, CellularReturnType::Distance2Cave>(cellularDistance, args...);
        break;
    }
}

template<SIMDType _SIMDType, BuildType _BuildType, typename... _Types>
static void CallBuildCellular(PerturbType perturbType, CellularDistance cellularDistance, CellularReturnType cellularReturnType, NoiseType lookupNoiseType, _Types... args)
{
    switch(perturbType)
    {
    case PerturbType::None:
        CallBuildCellular<_SIMDType, _BuildType, PerturbType::None>(cellularDistance, cellularReturnType, lookupNoiseType, args...);
        break;
    case PerturbType::Gradient:
        CallBuildCellular<_SIMDType, _BuildType, PerturbType::Gradient>(cellularDistance, cellularReturnType, lookupNoiseType, args...);
        break;
    case PerturbType::GradientFractal:
        CallBuildCellular<_SIMDType, _BuildType, PerturbType::GradientFractal>(cellularDistance, cellularReturnType, lookupNoiseType, args...);
        break;
    case PerturbType::Normalise:
        CallBuildCellular<_SIMDType, _BuildType, PerturbType::Normalise>(cellularDistance, cellularReturnType, lookupNoiseType, args...);
        break;
    case PerturbType::Gradient_Normalise:
        CallBuildCellular<_SIMDType, _BuildType, PerturbType::Gradient_Normalise>(cellularDistance, cellularReturnType, lookupNoiseType, args...);
        break;
    case PerturbType::GradientFractal_Normalise:
        CallBuildCellular<_SIMDType, _BuildType, PerturbType::GradientFractal_Normalise>(cellularDistance, cellularReturnType, lookupNoiseType, args...);
        break;
    }
}

template<SIMDType _SIMDType, BuildType _BuildType, typename... _Types>
static void CallBuild(NoiseType noiseType, PerturbType perturbType, FractalType fractalType, CellularDistance cellularDistance, CellularReturnType cellularReturnType, NoiseType lookupNoiseType, _Types... args)
{
    if(noiseType==NoiseType::Cellular)
    {
        if(cellularReturnType==CellularReturnType::NoiseLookup)
            CallBuildCellularLookup<_SIMDType, _BuildType, CellularReturnType::NoiseLookup>(noiseType, perturbType, fractalType, cellularDistance, lookupNoiseType, args...);
        else
            CallBuildCellular<_SIMDType, _BuildType>(perturbType, cellularDistance, cellularReturnType, lookupNoiseType, args...);
    }
	else if(isFractal(noiseType))
        CallBuildFractal<_SIMDType, _BuildType, CellularDistance::None, CellularReturnType::None, NoiseType::None>(noiseType, perturbType, fractalType, args...);
    else
        CallBuild<_SIMDType, _BuildType, FractalType::None, CellularDistance::None, CellularReturnType::None, NoiseType::None>(noiseType, perturbType, args...);
}



////////////////////////////////////////////////////////////////////////////////
//fill functions
////////////////////////////////////////////////////////////////////////////////

template<SIMDType _SIMDType>
void NoiseSIMD<_SIMDType>::FillSet(float* noiseSet, int xStart, int yStart, int zStart, int xSize, int ySize, int zSize, float scaleModifier)
{
	SIMD_HELPERS

    assert(noiseSet);
    simd::zeroAll();

    if(m_noiseType==NoiseType::WhiteNoise)
    {
        FillWhiteNoiseSet(noiseSet, xStart, yStart, zStart, xSize, ySize, zSize, scaleModifier);
        return;
    }

    NoiseValues<_SIMDType> noise=initNoise<_SIMDType>(m_noiseDetails, scaleModifier);
    PerturbValues<_SIMDType> perturb=initPerturb<_SIMDType>(m_perturbType, m_noiseDetails, m_perturbDetails);

    CallBuild<_SIMDType, BuildType::Default>(m_noiseType, m_perturbType, m_fractalType, m_cellularDistance, m_cellularReturnType, m_cellularNoiseLookupType,
        noise, perturb, noiseSet, xStart, yStart, zStart, xSize, ySize, zSize);

    simd::zeroAll();
}

template<SIMDType _SIMDType>
void NoiseSIMD<_SIMDType>::FillSet(float* noiseSet, VectorSet* vectorSet, float xOffset, float yOffset, float zOffset)
{
	SIMD_HELPERS

    assert(noiseSet);
    assert(vectorSet);
    assert(vectorSet->size>=0);
    simd::zeroAll();

    NoiseValues<_SIMDType> noise=initNoise<_SIMDType>(m_noiseDetails);
    Float xOffsetV=simd::mulf(simd::set(xOffset), noise.xFreqV);
    Float yOffsetV=simd::mulf(simd::set(yOffset), noise.yFreqV);
    Float zOffsetV=simd::mulf(simd::set(zOffset), noise.zFreqV);
    PerturbValues<_SIMDType> perturb=initPerturb<_SIMDType>(m_perturbType, m_noiseDetails, m_perturbDetails);

    CallBuild<_SIMDType, BuildType::Vector>(m_noiseType, m_perturbType, m_fractalType, m_cellularDistance, m_cellularReturnType, m_cellularNoiseLookupType,
        noise, perturb, noiseSet, vectorSet, xOffsetV, yOffsetV, zOffsetV);

    simd::zeroAll();
}

template<SIMDType _SIMDType>
void NoiseSIMD<_SIMDType>::FillWhiteNoiseSet(float* noiseSet, int xStart, int yStart, int zStart, int xSize, int ySize, int zSize, float scaleModifier)
{
	SIMD_HELPERS

    assert(noiseSet);
    simd::zeroAll();
    Int seedV=simd::set(m_noiseDetails.seed);

    if((zSize & (simd::vectorSize()-1))==0)
    {
        Int x=simd::mul(simd::set(xStart), Constant::numi_xPrime);
        Int yBase=simd::mul(simd::set(yStart), Constant::numi_yPrime);
        Int zBase=simd::mul(simd::add(Constant::numi_incremental, simd::set(zStart)), Constant::numi_zPrime);

        Int zStep=simd::mul(Constant::numi_vectorSize, Constant::numi_zPrime);

        size_t index=0;

        for(size_t ix=0; ix < xSize; ix++)
        {
            Int y=yBase;

            for(size_t iy=0; iy < ySize; iy++)
            {
                Int z=zBase;

                simd::store(&noiseSet[index], ValCoord<_SIMDType>(seedV, x, y, z));

                size_t iz=simd::vectorSize();
                while(iz < zSize)
                {
                    z=simd::add(z, zStep);
                    index+=simd::vectorSize();
                    iz+=simd::vectorSize();

                    simd::store(&noiseSet[index], ValCoord<_SIMDType>(seedV, x, y, z));
                }
                index+=simd::vectorSize();
                y=simd::add(y, Constant::numi_yPrime);
            }
            x=simd::add(x, Constant::numi_xPrime);
        }
    }
    else
    {
        Int ySizeV=simd::set(ySize);
        Int zSizeV=simd::set(zSize);

        Int yEndV=simd::set(yStart+ySize-1);
        Int zEndV=simd::set(zStart+zSize-1);

        Int x=simd::set(xStart);
        Int y=simd::set(yStart);
        Int z=simd::add(simd::set(zStart), Constant::numi_incremental);

        axisReset<_SIMDType>(x, y, z, ySizeV, yEndV, zSizeV, zEndV, zSize, 1);

        size_t index=0;
        int maxIndex=xSize * ySize * zSize;

        for(; index < maxIndex-simd::vectorSize(); index+=simd::vectorSize())
        {
            simd::store(&noiseSet[index], ValCoord<_SIMDType>(seedV, simd::mul(x, Constant::numi_xPrime), simd::mul(y, Constant::numi_yPrime), simd::mul(z, Constant::numi_zPrime)));

            z=simd::add(z, Constant::numi_vectorSize);

            axisReset<_SIMDType>(x, y, z, ySizeV, yEndV, zSizeV, zEndV, zSize, 0);
        }
        Float result=ValCoord<_SIMDType>(seedV, simd::mul(x, Constant::numi_xPrime), simd::mul(y, Constant::numi_yPrime), simd::mul(z, Constant::numi_zPrime));
        STORE_LAST_RESULT(&noiseSet[index], result);
    }
    simd::zeroAll();
}


//#define SAMPLE_INDEX(_x,_y,_z) ((_x) * yzSizeSample + (_y) * zSizeSample + (_z))
//#define SET_INDEX(_x,_y,_z) ((_x) * yzSize + (_y) * zSize + (_z))
//
//template<SIMDType _SIMDType>
//void NoiseSIMD<_SIMDType>::FillSampledNoiseSet(float* noiseSet, int xStart, int yStart, int zStart, int xSize, int ySize, int zSize, int sampleScale)
//{
//    SIMD_HELPERS
//
//	assert(noiseSet);
//	simd::zeroAll();
//
//	if (sampleScale <= 0)
//	{
//		FillSet(noiseSet, xStart, yStart, zStart, xSize, ySize, zSize, 1.0);
//		return;
//	}
//
//	int sampleSize = 1 << sampleScale;
//	int sampleMask = sampleSize - 1;
//	float scaleModifier = float(sampleSize);
//
//	int xOffset = (sampleSize - (xStart & sampleMask)) & sampleMask;
//	int yOffset = (sampleSize - (yStart & sampleMask)) & sampleMask;
//	int zOffset = (sampleSize - (zStart & sampleMask)) & sampleMask;
//
//	int xSizeSample = xSize + xOffset;
//	int ySizeSample = ySize + yOffset;
//	int zSizeSample = zSize + zOffset;
//
//	if (xSizeSample & sampleMask)
//		xSizeSample = (xSizeSample & ~sampleMask) + sampleSize;
//
//	if (ySizeSample & sampleMask)
//		ySizeSample = (ySizeSample & ~sampleMask) + sampleSize;
//
//	if (zSizeSample & sampleMask)
//		zSizeSample = (zSizeSample & ~sampleMask) + sampleSize;
//
//	xSizeSample = (xSizeSample >> sampleScale) + 1;
//	ySizeSample = (ySizeSample >> sampleScale) + 1;
//	zSizeSample = (zSizeSample >> sampleScale) + 1;
//
//	float* noiseSetSample = GetEmptySet(xSizeSample * ySizeSample * zSizeSample);
//	FillSet(noiseSetSample, xStart >> sampleScale, yStart >> sampleScale, zStart >> sampleScale, xSizeSample, ySizeSample, zSizeSample, scaleModifier);
//
//	int yzSizeSample = ySizeSample * zSizeSample;
//	int yzSize = ySize * zSize;
//
//	Int axisMask = simd::set(sampleMask);
//	Float axisScale = simd::set(1.f / scaleModifier);
//	Float axisOffset = simd::mulf(axisScale, Constant::numf_0_5);
//
//	Int sampleSizeSIMD = simd::set(sampleSize);
//	Int xSIMD = simd::set(-xOffset);
//	Int yBase = simd::set(-yOffset);
//	Int zBase = simd::set(-zOffset);
//
//	int localCountMax = (1 << (sampleScale * 3));
//	int vMax = simd::vectorSize();
//
//#if SIMD_LEVEL == HN_NEON
//	Int sampleScaleV = simd::set(-sampleScale);
//	Int sampleScale2V = simd::mul(sampleScaleV, Constant::numi_2);
//#endif
//
//	for (int x = 0; x < xSizeSample - 1; x++)
//	{
//		Int ySIMD = yBase;
//		for (int y = 0; y < ySizeSample - 1; y++)
//		{
//			Int zSIMD = zBase;
//
//			Float c001 = simd::set(noiseSetSample[SAMPLE_INDEX(x, y, 0)]);
//			Float c101 = simd::set(noiseSetSample[SAMPLE_INDEX(x + 1, y, 0)]);
//			Float c011 = simd::set(noiseSetSample[SAMPLE_INDEX(x, y + 1, 0)]);
//			Float c111 = simd::set(noiseSetSample[SAMPLE_INDEX(x + 1, y + 1, 0)]);
//			for (int z = 0; z < zSizeSample - 1; z++)
//			{
//				Float c000 = c001;
//				Float c100 = c101;
//				Float c010 = c011;
//				Float c110 = c111;
//				c001 = simd::set(noiseSetSample[SAMPLE_INDEX(x, y, z + 1)]);
//				c101 = simd::set(noiseSetSample[SAMPLE_INDEX(x + 1, y, z + 1)]);
//				c011 = simd::set(noiseSetSample[SAMPLE_INDEX(x, y + 1, z + 1)]);
//				c111 = simd::set(noiseSetSample[SAMPLE_INDEX(x + 1, y + 1, z + 1)]);
//
//				Int localCountSIMD = Constant::numi_incremental;
//
//				int localCount = 0;
//				while (localCount < localCountMax)
//				{
//                    uSIMD<simd::Int, simd::vectorSize()> xi, yi, zi;
//
//#if SIMD_LEVEL == HN_NEON
//					xi.m = simd::_and(simd::shiftL(localCountSIMD, sampleScale2V), axisMask);
//					yi.m = simd::_and(simd::shiftL(localCountSIMD, sampleScaleV), axisMask);
//#else
//					xi.m = simd::_and(simd::shiftR(localCountSIMD, sampleScale * 2), axisMask);
//					yi.m = simd::_and(simd::shiftR(localCountSIMD, sampleScale), axisMask);
//#endif
//
//					zi.m = simd::_and(localCountSIMD, axisMask);
//
//					Float xf = simd::mulAdd(simd::convert(xi.m), axisScale, axisOffset);
//					Float yf = simd::mulAdd(simd::convert(yi.m), axisScale, axisOffset);
//					Float zf = simd::mulAdd(simd::convert(zi.m), axisScale, axisOffset);
//
//					xi.m = simd::add(xi.m, xSIMD);
//					yi.m = simd::add(yi.m, ySIMD);
//					zi.m = simd::add(zi.m, zSIMD);
//
//                    uSIMD<simd::Float, simd::vectorSize()> sampledResults;
//					sampledResults.m = Lerp<_SIMDType>(
//						Lerp<_SIMDType>(
//							Lerp<_SIMDType>(c000, c100, xf),
//							Lerp<_SIMDType>(c010, c110, xf), yf),
//						Lerp<_SIMDType>(
//							Lerp<_SIMDType>(c001, c101, xf),
//							Lerp<_SIMDType>(c011, c111, xf), yf), zf);
//
//					for (int i = 0; i < vMax; i++)
//					{
//						if (xi.a[i] >= 0 && xi.a[i] < xSize &&
//							yi.a[i] >= 0 && yi.a[i] < ySize &&
//							zi.a[i] >= 0 && zi.a[i] < zSize)
//						{
//							int index = SET_INDEX(xi.a[i], yi.a[i], zi.a[i]);
//							noiseSet[index] = sampledResults.a[i];
//						}
//					}
//
//					localCount += simd::vectorSize();
//					localCountSIMD = simd::add(localCountSIMD, Constant::numi_vectorSize);
//				}
//				zSIMD = simd::add(zSIMD, sampleSizeSIMD);
//			}
//			ySIMD = simd::add(ySIMD, sampleSizeSIMD);
//		}
//		xSIMD = simd::add(xSIMD, sampleSizeSIMD);
//	}
//
//	FreeNoiseSet(noiseSetSample);
//	simd::zeroAll();
//}
//
//template<SIMDType _SIMDType>
//void NoiseSIMD<_SIMDType>::FillSampledNoiseSet(float* noiseSet, VectorSet* vectorSet, float xOffset, float yOffset, float zOffset)
//{
//    SIMD_HELPERS
//
//	assert(noiseSet);
//	assert(vectorSet);
//	assert(vectorSet->size >= 0);
//	simd::zeroAll();
//
//	int sampleScale = vectorSet->sampleScale;
//
//	if (sampleScale <= 0)
//	{
//		FillNoiseSet(noiseSet, vectorSet, xOffset, yOffset, zOffset);
//		return;
//	}
//
//	int sampleSize = 1 << sampleScale;
//	int sampleMask = sampleSize - 1;
//	float scaleModifier = float(sampleSize);
//
//	int xSize = vectorSet->sampleSizeX;
//	int ySize = vectorSet->sampleSizeY;
//	int zSize = vectorSet->sampleSizeZ;
//
//	int xSizeSample = xSize;
//	int ySizeSample = ySize;
//	int zSizeSample = zSize;
//
//	if (xSizeSample & sampleMask)
//		xSizeSample = (xSizeSample & ~sampleMask) + sampleSize;
//
//	if (ySizeSample & sampleMask)
//		ySizeSample = (ySizeSample & ~sampleMask) + sampleSize;
//
//	if (zSizeSample & sampleMask)
//		zSizeSample = (zSizeSample & ~sampleMask) + sampleSize;
//
//	xSizeSample = (xSizeSample >> sampleScale) + 1;
//	ySizeSample = (ySizeSample >> sampleScale) + 1;
//	zSizeSample = (zSizeSample >> sampleScale) + 1;
//
//	float* noiseSetSample = GetEmptySet(vectorSet->size);
//	FillNoiseSet(noiseSetSample, vectorSet, xOffset - 0.5f, yOffset - 0.5f, zOffset - 0.5f);
//
//	int yzSizeSample = ySizeSample * zSizeSample;
//	int yzSize = ySize * zSize;
//
//	Int axisMask = simd::set(sampleMask);
//	Float axisScale = simd::set(1.f / scaleModifier);
//	Float axisOffset = simd::mulf(axisScale, Constant::numf_0_5);
//
//	Int sampleSizeSIMD =simd::set(sampleSize);
//	Int xSIMD =simd::zeroInt();
//
//	int localCountMax = (1 << (sampleScale * 3));
//	int vMax = simd::vectorSize();
//
//#if SIMD_LEVEL == HN_NEON
//	Int sampleScaleV = simd::set(-sampleScale);
//	Int sampleScale2V = simd::mul(sampleScaleV, Constant::numi_2);
//#endif
//
//	for (int x = 0; x < xSizeSample - 1; x++)
//	{
//		Int ySIMD = simd::zeroInt();
//		for (int y = 0; y < ySizeSample - 1; y++)
//		{
//			Int zSIMD =simd::zeroInt();
//
//			Float c001 =simd::set(noiseSetSample[SAMPLE_INDEX(x, y, 0)]);
//			Float c101 =simd::set(noiseSetSample[SAMPLE_INDEX(x + 1, y, 0)]);
//			Float c011 =simd::set(noiseSetSample[SAMPLE_INDEX(x, y + 1, 0)]);
//			Float c111 =simd::set(noiseSetSample[SAMPLE_INDEX(x + 1, y + 1, 0)]);
//			for (int z = 0; z < zSizeSample - 1; z++)
//			{
//				Float c000 = c001;
//				Float c100 = c101;
//				Float c010 = c011;
//				Float c110 = c111;
//				c001 = simd::set(noiseSetSample[SAMPLE_INDEX(x, y, z + 1)]);
//				c101 = simd::set(noiseSetSample[SAMPLE_INDEX(x + 1, y, z + 1)]);
//				c011 = simd::set(noiseSetSample[SAMPLE_INDEX(x, y + 1, z + 1)]);
//				c111 = simd::set(noiseSetSample[SAMPLE_INDEX(x + 1, y + 1, z + 1)]);
//
//				Int localCountSIMD = Constant::numi_incremental;
//
//				int localCount = 0;
//				while (localCount < localCountMax)
//				{
//                    uSIMD<simd::Int, simd::vectorSize()> xi, yi, zi;
//
//#if SIMD_LEVEL == HN_NEON
//					xi.m = simd::_and(simd::shiftL(localCountSIMD, sampleScale2V), axisMask);
//					yi.m = simd::_and(simd::shiftL(localCountSIMD, sampleScaleV), axisMask);
//#else
//					xi.m = simd::_and(simd::shiftR(localCountSIMD, sampleScale * 2), axisMask);
//					yi.m = simd::_and(simd::shiftR(localCountSIMD, sampleScale), axisMask);
//#endif
//
//					zi.m = simd::_and(localCountSIMD, axisMask);
//
//					Float xf = simd::mulAdd(simd::convert(xi.m), axisScale, axisOffset);
//					Float yf = simd::mulAdd(simd::convert(yi.m), axisScale, axisOffset);
//					Float zf = simd::mulAdd(simd::convert(zi.m), axisScale, axisOffset);
//
//					xi.m = simd::add(xi.m, xSIMD);
//					yi.m = simd::add(yi.m, ySIMD);
//					zi.m = simd::add(zi.m, zSIMD);
//
//                    uSIMD<simd::Float, simd::vectorSize()> sampledResults;
//					sampledResults.m = Lerp<_SIMDType>(
//						Lerp<_SIMDType>(
//							Lerp<_SIMDType>(c000, c100, xf),
//							Lerp<_SIMDType>(c010, c110, xf), yf),
//						Lerp<_SIMDType>(
//							Lerp<_SIMDType>(c001, c101, xf),
//							Lerp<_SIMDType>(c011, c111, xf), yf), zf);
//
//					for (int i = 0; i < vMax; i++)
//					{
//						if (xi.a[i] < xSize &&
//							yi.a[i] < ySize &&
//							zi.a[i] < zSize)
//						{
//							int index = SET_INDEX(xi.a[i], yi.a[i], zi.a[i]);
//							noiseSet[index] = sampledResults.a[i];
//						}
//					}
//
//					localCount += simd::vectorSize();
//					localCountSIMD = simd::add(localCountSIMD, Constant::numi_vectorSize);
//				}
//				zSIMD = simd::add(zSIMD, sampleSizeSIMD);
//			}
//			ySIMD = simd::add(ySIMD, sampleSizeSIMD);
//		}
//		xSIMD = simd::add(xSIMD, sampleSizeSIMD);
//	}
//
//	FreeNoiseSet(noiseSetSample);
//	simd::zeroAll();
//}

}//namespace details
}//namespace HastyNoise