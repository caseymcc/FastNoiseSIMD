cmake_minimum_required(VERSION 3.10)

string(COMPARE EQUAL "$ENV{TRAVIS}" "true" is_travis)
string(COMPARE EQUAL "$ENV{APPVEYOR}" "True" is_appveyor)

option(HN_FAST_INVSQRT "Use fast inverse sqrt" OFF)
option(HN_BUILD_APP "Build test app" OFF)
option(HN_PREVIEW_APP "Build preview app" ON)

if(is_travis OR is_appveyor)
    set(HN_BUILD_APP ON CACHE BOOL "Build test app" FORCE) #force building app on CI
    set(HN_PREVIEW_APP OFF CACHE BOOL "Build preview app" FORCE) #don't need for CI
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "is_travis: ${is_travis}")
message(STATUS "is_appveyor: ${is_appveyor}")
message(STATUS "HN_BUILD_APP: ${HN_BUILD_APP}")
message(STATUS "HN_PREVIEW_APP: ${HN_PREVIEW_APP}")

include(${CMAKE_CURRENT_LIST_DIR}/cmake/Hunter/HunterGate.cmake)

HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.23.181.tar.gz"
    SHA1 "270cafa2d67def4ad4d52409ff22a28890d41c84"
    LOCAL
)

project(HastyNoise VERSION 0.8.0)

if(MSVC)
    #setup pdb info
    set(pdb_output_dir "${CMAKE_CURRENT_BINARY_DIR}/pdb-files")

    set(CMAKE_PDB_OUTPUT_DIRECTORY "${pdb_output_dir}")
    set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY "${pdb_output_dir}") 

    #need to sync pdp files
    add_compile_options("/FS")
else()
    #requires c++17 for inline static variable support
    set(CMAKE_CXX_STANDARD 17)
endif()

set(HN_SIMD_SSE2 OFF)
set(HN_SIMD_SSE41 OFF)
set(HN_SIMD_AVX2 OFF)
set(HN_SIMD_AVX512 OFF)
set(HN_SIMD_NEON OFF)

set(HN_NO_FILESYSTEM 0)
set(HN_STD_FILESYSTEM 1)
set(HN_EXPERIMENTAL_FILESYSTEM 2)
set(HN_USE_FILESYSTEM ${HN_NO_FILESYSTEM})

set(HN_ARM64 OFF)

if(ANDROID_NDK_VERSION)
    string(SUBSTRING ${CMAKE_ANDROID_ARCH_ABI} 0 5 FM_ANDROID_ARCH)
    string(TOLOWER ${FM_ANDROID_ARCH} FM_ANDROID_ARCH)
    string(COMPARE EQUAL ${FM_ANDROID_ARCH} "arm64" HN_ARM64)
elseif(IOS_SDK_VERSION)
    string(SUBSTRING ${IPHONEOS_ARCHS} 0 3 FM_IOS_ARCH)
    string(TOLOWER ${FM_IOS_ARCH} FM_IOS_ARCH)
    string(COMPARE EQUAL ${FM_IOS_ARCH} "arm" HN_ARM64)
endif()

#get options based on architecture and compilier
if(HN_ARM64)
    set(HN_SIMD_NEON ON)
else()
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        message(STATUS "MSVC_VERSION: ${MSVC_VERSION}") 
        add_definitions(-DNOMINMAX)
    
        set(HN_SIMD_SSE2 ON)
        set(HN_SIMD_SSE41 ON)

        if(MSVC_VERSION GREATER_EQUAL 1900)
            set(HN_SIMD_AVX2 ON)
            set(HN_USE_FILESYSTEM ${HN_EXPERIMENTAL_FILESYSTEM})
            if(MSVC_VERSION GREATER 1900)
               set(HN_SIMD_AVX512 ON) #something wrong finding floor function
#                include(CheckCXXCompilerFlag)
#
#                CHECK_CXX_COMPILER_FLAG("/std:c++17" _cpp_17_flag_supported)
#                if(_cpp_17_flag_supported)
#                    add_compile_options("/std:c++17")
                    set(HN_USE_FILESYSTEM ${HN_STD_FILESYSTEM})
#                endif()
            endif()
        endif()
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
        set(HN_SIMD_SSE2 ON)
        set(HN_SIMD_SSE41 ON)
    
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 3.9)
            set(HN_SIMD_AVX2 ON)
            set(HN_SIMD_AVX512 ON)
        endif()
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        set(HN_SIMD_SSE2 ON)
        set(HN_SIMD_SSE41 ON)
        set(HN_USE_FILESYSTEM ${HN_EXPERIMENTAL_FILESYSTEM})

        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.6)
            set(HN_SIMD_AVX2 ON)
    
            if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.9)
                set(HN_SIMD_AVX512 ON)
            endif()

            if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.0)
                set(HN_USE_FILESYSTEM ${HN_STD_FILESYSTEM})
            endif()
        endif()
    endif()
endif()

message(STATUS "HN_SIMD_SSE2: ${HN_SIMD_SSE2}")
message(STATUS "HN_SIMD_SSE41: ${HN_SIMD_SSE41}")
message(STATUS "HN_SIMD_AVX2: ${HN_SIMD_AVX2}")
message(STATUS "HN_SIMD_AVX512: ${HN_SIMD_AVX512}")

set(hastyNoise_external_lib)

set(hastyNoise_sources
    HastyNoise/hastyNoise.cpp
    HastyNoise/hastyNoise_none.cpp
)

set(hastyNoise_headers
    HastyNoise/hastyNoise.h
    HastyNoise/hastyNoise_enums.h
    HastyNoise/hastyNoise_export.h
)

set(hastyNoise_inline
    HastyNoise/hastyNoise_internal.h
    HastyNoise/hastyNoise_internal.inl
    HastyNoise/simd_constants.inl
    HastyNoise/internal_none.inl
    HastyNoise/simd_init.inl
)

#project expects debug libs to have d at the end
if("${CMAKE_DEBUG_POSTFIX}" STREQUAL "")
    set(CMAKE_DEBUG_POSTFIX "d")
endif()

add_library(hastyNoise SHARED
    ${hastyNoise_sources}
    ${hastyNoise_headers}
    ${hastyNoise_inline}
)
set(library_targets ${library_targets} hastyNoise)

target_compile_definitions(hastyNoise PRIVATE "-DHN_USE_FILESYSTEM=${HN_USE_FILESYSTEM}")
if(HN_FAST_INVSQRT)
    target_compile_definitions(hastyNoise PUBLIC "-DHN_FAST_INVSQRT")
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    target_compile_options(hastyNoise PUBLIC "/bigobj")
    if(MSVC_VERSION GREATER 1900)
        target_compile_options(hastyNoise PUBLIC "/std:c++17")
    endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    target_compile_options(hastyNoise PUBLIC "-ldl")
    target_link_libraries(hastyNoise ${CMAKE_DL_LIBS})
    if(HN_USE_FILESYSTEM GREATER 0)
        target_compile_options(hastyNoise PUBLIC "-lstdc++fs")
        target_link_libraries(hastyNoise stdc++fs)
    endif()
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND is_travis)
        target_compile_options(hastyNoise PUBLIC "-Wno-ignored-attributes") #gcc warnings are mean to travis logs
    endif()
endif()
#target_include_directories(hastyNoise PUBLIC ${CMAKE_CURRENT_LIST_DIR})

if(HN_USE_FILESYSTEM EQUAL ${HN_NO_FILESYSTEM})
    hunter_add_package(Boost COMPONENTS filesystem)
    find_package(Boost CONFIG REQUIRED filesystem)

    list(APPEND hastyNoise_external_lib Boost::filesystem)
    target_link_libraries(hastyNoise Boost::filesystem)
endif()

if(HN_SIMD_SSE2)
    set(hastyNoise_sse2_sources
        HastyNoise/hastyNoise_sse2.cpp
    )

    set(hastyNoise_sse2_headers
        HastyNoise/internal_sse2.inl
    )

    add_library(hastyNoise_sse2 SHARED
        ${hastyNoise_sse2_sources}
        ${hastyNoise_sse2_headers}
    )
    set(library_targets ${library_targets} hastyNoise_sse2)

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        target_compile_options(hastyNoise_sse2 PUBLIC "/bigobj")
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
        target_compile_options(hastyNoise_sse2 PRIVATE "-msse2")
    endif()
    target_link_libraries(hastyNoise_sse2 hastyNoise)
endif()

if(HN_SIMD_SSE41)
    set(hastyNoise_sse41_sources
        HastyNoise/hastyNoise_sse41.cpp
    )

    set(hastyNoise_sse41_headers
        HastyNoise/internal_sse41.inl
    )

    add_library(hastyNoise_sse41 SHARED
        ${hastyNoise_sse41_sources}
        ${hastyNoise_sse41_headers}
    )
    set(library_targets ${library_targets} hastyNoise_sse41)

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        target_compile_options(hastyNoise_sse41 PUBLIC "/bigobj")
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
        target_compile_options(hastyNoise_sse41 PRIVATE "-msse4.1")
    endif()
    target_link_libraries(hastyNoise_sse41 hastyNoise)
endif()

if(HN_SIMD_AVX2)
    set(hastyNoise_avx2_sources
        HastyNoise/hastyNoise_avx2.cpp
    )

    set(hastyNoise_avx2_headers
        HastyNoise/internal_avx2.inl
    )

    add_library(hastyNoise_avx2 SHARED
        ${hastyNoise_avx2_sources}
        ${hastyNoise_avx2_headers}
    )
    set(library_targets ${library_targets} hastyNoise_avx2)

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        target_compile_options(hastyNoise_avx2 PRIVATE "/arch:AVX2" "/bigobj")
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
        target_compile_options(hastyNoise_avx2 PRIVATE "-mavx2" "-mfma")
    endif()
    target_link_libraries(hastyNoise_avx2 hastyNoise)
endif()

if(HN_SIMD_AVX512)
    set(hastyNoise_avx512_sources
        HastyNoise/hastyNoise_avx512.cpp
    )

    set(hastyNoise_avx512_headers
        HastyNoise/internal_avx512.inl
    )

    add_library(hastyNoise_avx512 SHARED
        ${hastyNoise_avx512_sources}
        ${hastyNoise_avx512_headers}
    )
    set(library_targets ${library_targets} hastyNoise_avx512)

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        target_compile_options(hastyNoise_avx512 PRIVATE "/arch:AVX512" "/bigobj")
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
        target_compile_options(hastyNoise_avx512 PRIVATE "-mavx512f" "-mavx512dq" "-mfma")
    endif()
    target_link_libraries(hastyNoise_avx512 hastyNoise)
endif()

if(HN_BUILD_APP OR HN_PREVIEW_APP)
#add launcher for apps
    hunter_add_package(CreateLaunchers)
    find_package(CreateLaunchers CONFIG REQUIRED)

    include(CreateLaunchers)
endif()

if(HN_BUILD_APP)
    include_directories(${CMAKE_CURRENT_LIST_DIR})
    add_executable(hastyNoiseApp testApp/main.cpp)
    set(library_targets ${library_targets} hastyNoiseApp)
    
    set(hastyNoise_libs
        hastyNoise
    )
    
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        set(hastyNoise_libs ${hastyNoise_libs}
            Mincore.lib
        )
    endif()
    target_link_libraries(hastyNoiseApp ${hastyNoise_libs})
    target_compile_definitions(hastyNoiseApp PRIVATE "-DHN_USE_FILESYSTEM=${HN_USE_FILESYSTEM}")
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
        if(HN_USE_FILESYSTEM GREATER 0)
            target_compile_options(hastyNoiseApp PUBLIC "-lstdc++fs")
        endif()
    endif()
    
    
    if(HN_SIMD_SSE2)
        add_dependencies(hastyNoiseApp hastyNoise_sse2)
    endif()
    if(HN_SIMD_SSE41)
        add_dependencies(hastyNoiseApp hastyNoise_sse41)
    endif()
    if(HN_SIMD_AVX2)
        add_dependencies(hastyNoiseApp hastyNoise_avx2)
    endif()
    if(HN_SIMD_AVX512)
        add_dependencies(hastyNoiseApp hastyNoise_avx512)
    endif()

    set(hastyNoiseApp_enviroment_dir)
    foreach(target ${hastyNoise_external_lib})
        list(APPEND hastyNoiseApp_enviroment_dir "$<TARGET_FILE_DIR:${target}>")
    endforeach()

    create_target_launcher(hastyNoiseApp
        RUNTIME_LIBRARY_DIRS "${hastyNoiseApp_enviroment_dir}"
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:hastyNoiseApp>"
    )
endif()

if(HN_PREVIEW_APP)
    hunter_add_package(glfw)
    find_package(glfw3 REQUIRED)

    hunter_add_package(imgui)
    find_package(imgui CONFIG REQUIRED)

    add_executable(hastyPreviewApp preview/main.cpp)
    target_include_directories(hastyPreviewApp PRIVATE ${CMAKE_CURRENT_LIST_DIR})
    target_link_libraries(hastyPreviewApp hastyNoise glfw imgui::imgui)

    if(HN_SIMD_SSE2)
        add_dependencies(hastyPreviewApp hastyNoise_sse2)
    endif()
    if(HN_SIMD_SSE41)
        add_dependencies(hastyPreviewApp hastyNoise_sse41)
    endif()
    if(HN_SIMD_AVX2)
        add_dependencies(hastyPreviewApp hastyNoise_avx2)
    endif()
    if(HN_SIMD_AVX512)
        add_dependencies(hastyPreviewApp hastyNoise_avx512)
    endif()

    #create launcher that will include hunter paths
    set(hastyPreviewApp_enviroment_dir)
    foreach(target ${hastyNoise_external_lib})
        list(APPEND hastyPreviewApp_enviroment_dir "$<TARGET_FILE_DIR:${target}>")
    endforeach()
    set(hastyPreviewApp_working_dir ${CMAKE_CURRENT_LIST_DIR})

    create_target_launcher(hastyPreviewApp
        RUNTIME_LIBRARY_DIRS "${hastyPreviewApp_enviroment_dir}"
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:hastyPreviewApp>"
    )
endif()


#Install -----------------------------------------------------------

# Introduce variables:
# * CMAKE_INSTALL_LIBDIR
# * CMAKE_INSTALL_BINDIR
include(GNUInstallDirs) 

# Layout. This works for all platforms:
#   * <prefix>/lib*/cmake/<PROJECT-NAME>
#   * <prefix>/lib*/
#   * <prefix>/include/
set(config_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")

# Configuration
set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")
set(namespace "${PROJECT_NAME}::")

# Include module with fuction 'write_basic_package_version_file'
include(CMakePackageConfigHelpers)

# Configure '<PROJECT-NAME>ConfigVersion.cmake'
# Use:
#   * PROJECT_VERSION
write_basic_package_version_file(
    "${version_config}" COMPATIBILITY SameMajorVersion
)

# Configure '<PROJECT-NAME>Config.cmake'
# Use variables:
#   * TARGETS_EXPORT_NAME
#   * PROJECT_NAME
configure_package_config_file(
    "cmake/Config.cmake.in"
    "${project_config}"
    INSTALL_DESTINATION "${config_install_dir}"
)

# Targets:
#   * <prefix>/lib/libname.a
#   * header location after install: <prefix>/include/${PROJECT_NAME}/include.hpp
#   * headers can be included by C++ code `#include <${PROJECT_NAME}/include.hpp>`
install(
    TARGETS ${library_targets}
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

# Headers:
install(
    FILES ${hastyNoise_headers}
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}"
)

# Config
#   * <prefix>/lib/cmake/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake
#   * <prefix>/lib/cmake/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake
install(
    FILES "${project_config}" "${version_config}"
    DESTINATION "${config_install_dir}"
)

# Config
#   * <prefix>/lib/cmake/${PROJECT_NAME}/${PROJECT_NAME}Targets.cmake
install(
    EXPORT "${TARGETS_EXPORT_NAME}"
    NAMESPACE "${namespace}"
    DESTINATION "${config_install_dir}"
)

if(MSVC)
    #install pdbs
    get_cmake_property(is_multi GENERATOR_IS_MULTI_CONFIG)

    if(is_multi)
      set(config_suffix "$<CONFIG>")
    else()
      set(config_suffix "")
    endif() 

    if(BUILD_SHARED_LIBS)
      set(pdb_dst ${CMAKE_INSTALL_BINDIR})
    else()
      set(pdb_dst ${CMAKE_INSTALL_LIBDIR})
    endif() 

    install(
        DIRECTORY "${pdb_output_dir}/${config_suffix}/"
        DESTINATION ${pdb_dst}
    )
endif()