#include "hastyNoise.h"

#include "simd_constants.inl"
#include "internal_none.inl"
#include "hastyNoise_internal.h"

#include "simd_init.inl"
#include "hastyNoise_internal.inl"

namespace HastyNoise
{
namespace details
{

template class NoiseSIMD<SIMDType::None>;

}//namespace details
}//namespace HastyNoise

