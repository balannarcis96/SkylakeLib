cmake_minimum_required (VERSION 3.8)    
project("SkylakeLibHeaderOnly" VERSION 1.0.0)

if(SKL_HEADER_ONLY_BUILD_TYPE STREQUAL "Static_Dev")
    file(GLOB_RECURSE _Root_files LIST_DIRECTORIES false "${CMAKE_CURRENT_SOURCE_DIR}/Static_Dev/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/Static_Dev/*.cpp")
    add_library(SkylakeLibHeaderOnly STATIC ${_Root_files})
    source_group("/" FILES ${_Root_files})
    target_include_directories(SkylakeLibHeaderOnly PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Static_Dev/")
elseif(SKL_HEADER_ONLY_BUILD_TYPE STREQUAL "Interface")
    add_library(SkylakeLibHeaderOnly INTERFACE ${_Root_files})
    target_include_directories(SkylakeLibHeaderOnly INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/Interface/")
else()
    message(FATAL_ERROR "Invalid SkylakeLibHeaderOnly build type! [${SKL_HEADER_ONLY_BUILD_TYPE}]")
endif()
     
#Set C++20
set_property(TARGET SkylakeLibHeaderOnly PROPERTY CXX_STANDARD 20)

