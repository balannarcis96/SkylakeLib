cmake_minimum_required (VERSION 3.8)    
project("SkylakeLibHeaderOnly" VERSION 1.0.0)

file(GLOB_RECURSE _Root_files LIST_DIRECTORIES false "${CMAKE_CURRENT_SOURCE_DIR}/Static_Dev/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/Static_Dev/*.cpp")
add_library(SkylakeLibHeaderOnly INTERFACE ${_Root_files})
source_group("/" FILES ${_Root_files})
target_include_directories(SkylakeLibHeaderOnly INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/Static_Dev/")

#Set C++20
set_property(TARGET SkylakeLibHeaderOnly PROPERTY CXX_STANDARD 20)

target_precompile_headers(SkylakeLibHeaderOnly INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/Static_Dev/SkylakeLibHeaderOnly.h")

if(SKL_REAL_TYPE STREQUAL "Double")
    target_compile_definitions(SkylakeLibHeaderOnly INTERFACE SKL_REAL_TYPE_DOUBLE)
    target_compile_definitions(SkylakeLibHeaderOnly INTERFACE SKL_USE_LARGE_WORLD_COORDS)
endif()

if(SKL_ENABLE_LIKELY_FLAGS)
    target_compile_definitions(SkylakeLibHeaderOnly INTERFACE SKL_ENABLE_LIKELY_FLAGS)
endif()

if(SKL_ENABLE_UNLIKELY_FLAGS)
    target_compile_definitions(SkylakeLibHeaderOnly INTERFACE SKL_ENABLE_UNLIKELY_FLAGS)
endif()

# CacheLine size
if(SKL_L1_CACHE_LINE_SIZE STREQUAL "64bytes")
    target_compile_definitions(SkylakeLibHeaderOnly INTERFACE SKL_L1_CACHE_LINE_64)
    target_compile_definitions(SkylakeLibHeaderOnly INTERFACE SKL_CACHE_LINE_SIZE=64)
    target_compile_definitions(SkylakeLibHeaderOnly INTERFACE "SKL_CACHE_ALIGNED=alignas(64)")
elseif(SKL_L1_CACHE_LINE_SIZE STREQUAL "128bytes")
    target_compile_definitions(SkylakeLibHeaderOnly INTERFACE SKL_L1_CACHE_LINE_128)
    target_compile_definitions(SkylakeLibHeaderOnly INTERFACE SKL_CACHE_LINE_SIZE=128)
    target_compile_definitions(SkylakeLibHeaderOnly INTERFACE "SKL_CACHE_ALIGNED=alignas(128)")
elseif(SKL_L1_CACHE_LINE_SIZE STREQUAL "512bytes")
    target_compile_definitions(SkylakeLibHeaderOnly INTERFACE SKL_L1_CACHE_LINE_512)
    target_compile_definitions(SkylakeLibHeaderOnly INTERFACE SKL_CACHE_LINE_SIZE=512)
    target_compile_definitions(SkylakeLibHeaderOnly INTERFACE "SKL_CACHE_ALIGNED=alignas(512)")
endif()

# Disable std exceptions
if(SKL_DISABLE_EXCEPTIONS)
    target_compile_definitions(SkylakeLibHeaderOnly INTERFACE _HAS_EXCEPTIONS=0)
endif()
