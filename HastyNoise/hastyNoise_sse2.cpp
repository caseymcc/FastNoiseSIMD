#ifdef hastyNoise_sse2_EXPORTS
#define hastyNoise_EXPORTS
#endif
#include "hastyNoise.h"

#include "simd_constants.inl"
#include "internal_none.inl"
#include "internal_sse2.inl"
#include "hastyNoise_internal.h"

#include "simd_init.inl"
#include "hastyNoise_internal.inl"

namespace HastyNoise
{
namespace details
{

template class HASTYNOISE_EXPORT NoiseSIMD<SIMDType::SSE2>;

}//namespace details
}//namespace HastyNoise
