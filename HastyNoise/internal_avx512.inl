// internal_avx512.inl
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

#include <immintrin.h>

namespace HastyNoise
{
namespace details
{

template<>
struct SIMD<SIMDType::AVX512>
{
    typedef __m512 Float;
    typedef __m512i Int;
    typedef __mmask16 Mask;
    static constexpr size_t const vectorSize() { return 16; }
    static constexpr size_t const alignment() { return 64; }
    static constexpr int const level() { return (size_t)SIMDType::AVX512; }

    static Float set(float a) { return _mm512_set1_ps(a); }
    static Float zeroFloat() { return _mm512_setzero_ps(); }
    static Int set(int a) { return _mm512_set1_epi32(a); }
    static Int zeroInt() { return _mm512_setzero_si512(); }
    static void zeroAll() {}

#ifdef HN_ALIGNED_SETS
    static void store(float *p, Float a) { _mm512_store_ps(p, a); }
    static Float load(float *p) { return _mm512_load_ps(p); }
#else
    static void store(float *p, Float a) { _mm512_storeu_ps(p, a); }
    static Float load(float *p) { return _mm512_loadu_ps(p); }
#endif

    static Float undefinedFloat() { return _mm512_undefined_ps(); }
    static Int undefinedInt() { return _mm512_undefined_epi32(); }

    static Float convert(Int a) { return _mm512_cvtepi32_ps(a); }
    static Float cast(Int a) { return _mm512_castsi512_ps(a); }
    static Int convert(Float a) { return _mm512_cvtps_epi32(a); }
    static Int cast(Float a) { return  _mm512_castps_si512(a); }

    static Float add(Float a, Float b) { return _mm512_add_ps(a, b); }
    static Float sub(Float a, Float b) { return _mm512_sub_ps(a, b); }
    static Float mulf(Float a, Float b) { return _mm512_mul_ps(a, b); }
    static Float div(Float a, Float b) { return _mm512_div_ps(a, b); }

#ifdef HN_USE_FMA
    static Float mulAdd(Float a, Float b, Float c) { return _mm512_fmadd_ps(a, b, c); }
    static Float nmulAdd(Float a, Float b, Float c) { return _mm512_fnmadd_ps(a, b, c); }
    static Float mulSub(Float a, Float b, Float c) { return _mm512_fmsub_ps(a, b, c); }
#else
    static Float mulAdd(Float a, Float b, Float c) { return add(mulf(a, b), c); }
    static Float nmulAdd(Float a, Float b, Float c) { return sub(c, mulf(a, b)); }
    static Float mulSub(Float a, Float b, Float c) { return sub(mulf(a, b), c); }
#endif

    static Float min(Float a, Float b) { return _mm512_min_ps(a, b); }
    static Float max(Float  a, Float b) { return _mm512_max_ps(a, b); }
    static Float invSqrt(Float a) { return _mm512_rsqrt14_ps(a); }

    static Mask equal(Float a, Float b) { return _mm512_cmp_ps_mask(a, b, _CMP_EQ_OQ); }
    static Mask lessThan(Float a, Float b) { return _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ); }
    static Mask greaterThan(Float a, Float b) { return _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ); }
    static Mask lessEqual(Float a, Float b) { return _mm512_cmp_ps_mask(a, b, _CMP_LE_OQ); }
    static Mask greaterEqual(Float a, Float b) { return _mm512_cmp_ps_mask(a, b, _CMP_GE_OQ); }

    static Float _and(Float a, Float b) { return _mm512_and_ps(a, b); }
    static Float andNot(Float a, Float b) { return _mm512_andnot_ps(a, b); }
    static Float _xor(Float a, Float b) { return _mm512_xor_ps(a, b); }
    static Float _or(Float a, Float b) { return _mm512_or_ps(a, b); }

    static Float floor(Float a) 
    { 
#ifdef _MSC_VER //MSVC doesnt seem to have the floor function yet
#if _MSC_VER < 1920 // here is hoping VS2019 has it
        __m256 a0=_mm512_castps512_ps256(a);
        __m256 a1=_mm512_extractf32x8_ps(a, 1);

        a0=_mm256_floor_ps(a0);
        a1=_mm256_floor_ps(a1);

        Float b=_mm512_castps256_ps512(a0);
        b=_mm512_insertf32x8(b, a1, 1);
        return b;
#else
        return _mm512_floor_ps(a);
#endif
#else
        return _mm512_floor_ps(a);
#endif
    }
    static Float abs(Float a) { return _mm512_abs_ps(a); }
    static Float blend(Float a, Float b, Mask mask) { return _mm512_mask_blend_ps(mask, a, b); }

    static Int add(Int a, Int b) { return _mm512_add_epi32(a, b); }
    static Int sub(Int a, Int b) { return _mm512_sub_epi32(a, b); }
    static Int mul(Int a, Int b) { return _mm512_mullo_epi32(a, b); }

    static Int _and(Int a, Int b) { return _mm512_and_si512(a, b); }
    static Int andNot(Int a, Int b) { return _mm512_andnot_si512(a, b); }
    static Int _or(Int a, Int b) { return _mm512_or_si512(a, b); }
    static Int _xor(Int a, Int b) { return _mm512_xor_si512(a, b); }
    static Int _not(Int a) { return _xor(a, Constants<Float, Int, SIMDType::AVX512>::numi_0xffffffff); }

    static Int shiftR(Int a, int b) { return _mm512_srai_epi32(a, b); }
    static Int shiftL(Int a, int b) { return _mm512_slli_epi32(a, b); }
    
//    static Int vshiftR(Int a, int b) { return _mm512_srl_epi32(a, b); }
//    static Int vshiftL(Int a, int b) { return _mm512_sll_epi32(a, b); }

    static Mask equal(Int a, Int b) { return _mm512_cmpeq_epi32_mask(a, b); }
    static Mask greaterThan(Int a, Int b) { return _mm512_cmpgt_epi32_mask(a, b); }
    static Mask lessThan(Int a, Int b) { return _mm512_cmpgt_epi32_mask(b, a); }

    static Mask maskAnd(Mask a, Mask b) { return a&b; }
    static Mask maskAndNot(Mask a, Mask b) { return (~a)&b; }
    static Mask maskOr(Mask a, Mask b) { return a|b; }
    static Mask maskNot(Mask a) { return (~a); }

    static Float mask(Mask m, Float a) { return _mm512_maskz_mov_ps(m, a); }
    static Float maskAdd(Mask m, Float a, Float b) { return _mm512_mask_add_ps(a, m, a, b); }
    static Float maskSub(Mask m, Float a, Float b) { return _mm512_mask_sub_ps(a, m, a, b); }

    static Int maskAdd(Mask m, Int a, Int b) { return _mm512_mask_add_epi32(a, m, a, b); }
    static Int maskSub(Mask m, Int a, Int b) { return _mm512_mask_sub_epi32(a, m, a, b); }

    static Float gather(float *p, Int a) { return _mm512_i32gather_ps(a, p, 4); }
    static Float permute(Float a, Int b) { return _mm512_permutexvar_ps(b, a); }
};

}//namespace details
}//namespace HastyNoise
