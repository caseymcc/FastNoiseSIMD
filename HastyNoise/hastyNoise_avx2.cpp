#ifdef hastyNoise_avx2_EXPORTS
#define hastyNoise_EXPORTS
#endif

#include "hastyNoise.h"

#include "simd_constants.inl"
#include "internal_none.inl"
#include "internal_avx2.inl"
#include "hastyNoise_internal.h"

#include "simd_init.inl"
#include "hastyNoise_internal.inl"

namespace HastyNoise
{
namespace details
{

template class HASTYNOISE_EXPORT NoiseSIMD<SIMDType::AVX2>;
//template struct Constants<typename SIMD<SIMDType::AVX2>::Float, typename SIMD<SIMDType::AVX2>::Int, SIMDType::AVX2>;

}//namespace details
}//namespace HastyNoise
