cmake_minimum_required (VERSION 3.8)    
project("SkylakeLibHeaderOnly" VERSION 1.0.0)

file(GLOB_RECURSE _Root_files LIST_DIRECTORIES false "${CMAKE_CURRENT_SOURCE_DIR}/Static_Dev/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/Static_Dev/*.cpp")
add_library(SkylakeLibHeaderOnly STATIC ${_Root_files})
source_group("/" FILES ${_Root_files})
target_include_directories(SkylakeLibHeaderOnly PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Static_Dev/")

#Set C++20
set_property(TARGET SkylakeLibHeaderOnly PROPERTY CXX_STANDARD 20)

target_precompile_headers(SkylakeLibHeaderOnly PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Static_Dev/SkylakeLibHeaderOnly.h")
