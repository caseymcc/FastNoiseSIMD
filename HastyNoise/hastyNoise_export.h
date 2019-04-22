#if defined(_WIN32) || defined(_WIN64)
#   ifndef HASTYNOISE_EXPORT
#     ifdef hastyNoise_EXPORTS
#       define HASTYNOISE_EXPORT __declspec(dllexport)
#     else
#       define HASTYNOISE_EXPORT __declspec(dllimport)
#     endif
#   endif
#else
#   define HASTYNOISE_EXPORT 
#endif

#ifndef HASTYNOISE_NO_EXPORT
#  define HASTYNOISE_NO_EXPORT 
#endif
