
[![license](https://img.shields.io/github/license/mashape/apistatus.svg?style=flat-square "License")](https://github.com/caseymcc/HastyNoise/blob/master/LICENSE)
[![travis](https://img.shields.io/travis/caseymcc/HastyNoise/master.svg?logo=travis&style=flat-square&label=Linux%20OSX "Travis CI")](https://travis-ci.org/caseymcc/HastyNoise)
[![appveyor](https://img.shields.io/appveyor/ci/caseymcc/HastyNoise/master.svg?logo=appveyor&style=flat-square&label=Windows "AppVeyor CI")](https://ci.appveyor.com/project/caseymcc/hastynoise)


# Origins
This is an altered version of [FastNoiseSIMD](https://github.com/Auburns/FastNoiseSIMD). All of the macros in the original version have been replace with templates allowing for a little better debugging. Also the library has been altered to create multiple shared libraries each compiled with the proper SIMD Instructions. The shared libraries are dynamically loaded by the main library.

# HastyNoise
Hasty Noise is a noise library. It aims to provide high performance noise through the use of SIMD instructions on a variety of platforms. Vectorisation of the code allows noise functions to process data in sets of 4/8/16 increasing performance by 700% in some cases (Simplex).

The library compiles a shared library for each of the SIMD instructions sets. During compile time the library will build the highest level of SIMD supported by the compilier and at runtime all of the SIMD libraries built are loaded and the highest supported instruction set is reported. By default the library will use the highest level of SIMD detected. If no support is found it will fallback to standard types (float/int).

"We must not be hasty." - Treebeard

## Features

- Value Noise 3D
- Perlin Noise 3D
- Simplex Noise 3D
- Cubic Noise 3D
- Multiple fractal options for all of the above
- White Noise 3D
- Cellular Noise 3D
- Perturb input coordinates in 3D space
- Integrated up-sampling
- Easy to use 3D cave noise

## Supported Instruction Sets
- ~ARM NEON~ needs testing
- AVX-512F
- AVX2 - FMA3
- SSE4.1
- SSE2

## Tested Compilers
- MSVC v140/v150
- GCC 7 Linux
- Clang Linux/MacOSX

## Tested Platforms
- Windows
- Linux
- ~Android~ needs updates
- MacOSX
- ~iOs~ needs updates

## Wiki
[Docs](https://github.com/caseymcc/HastyNoise/wiki)

## Related repositories

- [FastNoiseSIMD](https://github.com/Auburns/FastNoiseSIMD)
- [FastNoise](https://github.com/Auburns/FastNoise)
- [CubicNoise](https://github.com/jobtalle/CubicNoise)

# Performance Comparisons
Timings below timings are to generate 100x 64x64x64 (~26.2M) points of noise on a single thread.

- CPU: Intel Core i7-5820K @ 3.3Ghz
- Compiler: Visual Studio 2017 x64

|  Noise Type | AVX512 |  AVX2  | SSE4.1 |  SSE2  |    None   |
|-------------|--------|--------|--------|--------|-----------|
| White Noise |        |    9ms |   18ms |   21ms |      48ms |
| Value       |        |  114ms |  243ms |  282ms |    2071ms |
| Perlin      |        |  193ms |  416ms |  534ms |    2816ms |
| Simplex     |        |  198ms |  372ms |  474ms |    2769ms |
| Cellular    |        |  915ms | 2095ms | 2218ms |   16388ms |
| Cubic       |        |  668ms | 1370ms | 2336ms |    5698ms |
