# Depreciated
HastyNoise is now depreciated, it has been replaced by [FastNoise 2](https://github.com/Auburn/FastNoise2).

[![license](https://img.shields.io/github/license/mashape/apistatus.svg?style=flat-square "License")](https://github.com/caseymcc/HastyNoise/blob/master/LICENSE)
[![hunter](https://img.shields.io/badge/hunter-HastyNoise-blue.svg)](https://docs.hunter.sh/en/latest/packages/pkg/HastyNoise.html#pkg-hastynoise)
[![discord](https://img.shields.io/discord/495955797872869376.svg?logo=discord "Discord")](https://discord.gg/BfceAsX)\
[![travis](https://img.shields.io/travis/caseymcc/HastyNoise/master.svg?logo=travis&style=flat-square&label=Linux%20OSX "Travis CI")](https://travis-ci.org/caseymcc/HastyNoise)
[![appveyor](https://img.shields.io/appveyor/ci/caseymcc/HastyNoise/master.svg?logo=appveyor&style=flat-square&label=Windows "AppVeyor CI")](https://ci.appveyor.com/project/caseymcc/hastynoise)

## Wiki
[Documentation](https://github.com/caseymcc/HastyNoise/wiki)

# HastyNoise
Hasty Noise is a noise library. It aims to provide high performance noise through the use of SIMD instructions on a variety of platforms. Vectorisation of the code allows noise functions to process data in sets of 4/8/16 increasing performance by 700% in some cases (Simplex).

![preview](https://github.com/caseymcc/HastyNoise/raw/master/examples/preview_perlinfractal.png)

The library compiles a shared library for each of the SIMD instructions sets. During compile time the library will build the highest level of SIMD supported by the compilier and at runtime all of the SIMD libraries built are loaded and the highest supported instruction set is reported. By default the library will use the highest level of SIMD detected. If no support is found it will fallback to standard types (float/int).

"We must not be hasty." - Treebeard


## Features

| Supported Noise   | Fractal | Perturb |
|-------------------|:-------:|:-------:|
| White             | no      | yes     |
| Value             | yes     | yes     |
| Perlin            | yes     | yes     |
| Simplex           | yes     | yes     |
| OpenSimplex2      | yes     | yes     |
| Cubic             | yes     | yes     |
| Cellular (Worley) | no      | yes     |

| Fractal Types | Perturb                    | Cellular Value   | Celllar Distance |
|---------------|----------------------------|------------------|------------------|
| FBM           | Gradient                   | Value            | Eucliden         |
| Billow        | Gradient Fractal           | Distance         | Manhattan        |
| RidgidMulti   | Normalize                  | Value Distance 2 | Natural          |
|               | Gradient Normalize         | Distance 2       |                  |
|               | Gradient Fractal Normalize | Distance 2 Add   |                  |
|               |                            | Distance 2 Sub   |                  |
|               |                            | Distance 2 Mul   |                  |
|               |                            | Distance 2 Div   |                  |
|               |                            | Lookup           |                  |
|               |                            | Distance 2 Cave  |                  |

## Supported Instruction/Compilers/Platforms
| Instructions | Compilers          | Platforms  |
|--------------|--------------------|------------|
|~ARM NEON~*   | MSVC v140/v150     | Windows    |
| AVX-512F     | GCC 7 Linux        | Linux      |
| AVX2 - FMA3  | Clang Linux/MacOSX | ~Android~* |
| SSE4.1       |                    | MacOSX     |
| SSE2         |                    | ~iOs~*     |

\* needs work

# Origins
This is an altered version of [FastNoiseSIMD](https://github.com/Auburns/FastNoiseSIMD). All of the macros in the original version have been replace with templates allowing for a little better debugging. Also the library has been altered to create multiple shared libraries each compiled with the proper SIMD Instructions. The shared libraries are dynamically loaded by the main library.



## Related repositories

- [FastNoiseSIMD](https://github.com/Auburns/FastNoiseSIMD)
- [FastNoise](https://github.com/Auburns/FastNoise)
- [CubicNoise](https://github.com/jobtalle/CubicNoise)

# Performance Comparisons
Timings below timings are to generate 100x 64x64x64 (~26.2M) points of noise on a single thread.

- CPU: Intel Core i7-5820K @ 3.3Ghz
- OS: Windows 10
- Compiler: Visual Studio 2017 x64

|  Noise Type  | AVX512 |  AVX2  | SSE4.1 |  SSE2  |    None   |
|--------------|--------|--------|--------|--------|-----------|
| White Noise  |        |    9ms |   18ms |   21ms |      48ms |
| Value        |        |  114ms |  243ms |  282ms |    2071ms |
| Perlin       |        |  193ms |  416ms |  534ms |    2816ms |
| Simplex      |        |  198ms |  372ms |  474ms |    2769ms |
| OpenSimplex2 |        |  218ms |  451ms |  471ms |    6683ms |
| Cellular     |        |  915ms | 2095ms | 2218ms |   16388ms |
| Cubic        |        |  668ms | 1370ms | 2336ms |    5698ms |


- CPU: Intel Xeon W-2125 @ 4.0Ghz
- OS: Ubuntu 18.04
- Compiler: gcc 7+ -O3

|  Noise Type  | AVX512 |  AVX2  | SSE4.1 |  SSE2  |    None   |
|--------------|--------|--------|--------|--------|-----------|
| White Noise  |    3ms |    4ms |    8ms |   18ms |      21ms |
| Value        |   50ms |   80ms |  161ms |  263ms |     509ms |
| Perlin       |   62ms |  163ms |  366ms |  445ms |    1556ms |
| Simplex      |   59ms |  155ms |  300ms |  339ms |    1129ms |
| OpenSimplex2 |   82ms |  175ms |  313ms |  421ms |    1683ms |
| Cellular     |  328ms |  580ms | 1316ms | 1358ms |    6145ms |
| Cubic        |  257ms |  490ms |  984ms | 1790ms |    2567ms |


# Examples
![preview](https://github.com/caseymcc/HastyNoise/raw/master/examples/preview_simplexfractal.png)
![preview](https://github.com/caseymcc/HastyNoise/raw/master/examples/preview_simplexfractal_billow.png)
![preview](https://github.com/caseymcc/HastyNoise/raw/master/examples/preview_cellularvalue.png)
![preview](https://github.com/caseymcc/HastyNoise/raw/master/examples/preview_cellulardistance2add.png)
![preview](https://github.com/caseymcc/HastyNoise/raw/master/examples/preview_cellulardistance2div_inv.png)
![preview](https://github.com/caseymcc/HastyNoise/raw/master/examples/preview_value.png)
![preview](https://github.com/caseymcc/HastyNoise/raw/master/examples/preview_whitenoise.png)
