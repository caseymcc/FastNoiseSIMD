#ifdef hastyNoise_sse41_EXPORTS
#define hastyNoise_EXPORTS
#endif
#include "hastyNoise.h"

#include "simd_constants.inl"
#include "internal_none.inl"
#include "internal_sse2.inl"
#include "internal_sse41.inl"
#include "hastyNoise_internal.h"

#include "simd_init.inl"
#include "hastyNoise_internal.inl"

namespace HastyNoise
{
namespace details
{

template class HASTYNOISE_EXPORT NoiseSIMD<SIMDType::SSE4_1>;
//template struct Constants<typename SIMD<SIMDType::SSE4_1>::Float, typename SIMD<SIMDType::SSE4_1>::Int, SIMDType::SSE4_1>;

}//namespace details
}//namespace HastyNoise
