cmake_minimum_required(VERSION 3.1)

project(FastNoise)

set(fastNoise_sources
    FastNoiseSIMD/FastNoiseSIMD.cpp
    FastNoiseSIMD/FastNoiseSIMD_internal.cpp
    FastNoiseSIMD/FastNoiseSIMD_avx2.cpp
    FastNoiseSIMD/FastNoiseSIMD_avx512.cpp
    FastNoiseSIMD/FastNoiseSIMD_neon.cpp
    FastNoiseSIMD/FastNoiseSIMD_sse2.cpp
    FastNoiseSIMD/FastNoiseSIMD_sse41.cpp
)

set(fastNoise_headers
    FastNoiseSIMD/FastNoiseSIMD.h
    FastNoiseSIMD/FastNoiseSIMD_internal.h
)

set(fastNoise_libraries
)

add_library(fastNoise STATIC
    ${fastNoise_sources}
    ${fastNoise_headers}
)

target_link_libraries(fastNoise ${fastNoise_libraries})

target_include_directories(fastNoise PUBLIC ${CMAKE_CURRENT_LIST_DIR})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_definitions(-DNOMINMAX)
    
    if(MSVC_VERSION EQUAL 1900)
        target_compile_options(fastNoise PUBLIC "/arch:AVX2")
    elseif(MSVC_VERSION GREATER 1900)
        target_compile_options(fastNoise PUBLIC "/arch:AVX512")
    endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    target_compile_options(fastNoise PUBLIC "-mavx2")
endif()

add_executable(testApp testApp/main.cpp)
target_link_libraries(testApp fastNoise)
