#include "hastyNoise.h"
#include "hastyNoise_internal.h"

namespace HastyNoise
{
namespace details
{

#ifdef HN_COMPILE_NEON
template class NoiseSIMD<SIMDType::Neon>;
#endif

}//namespace details
}//namespace HastyNoise

