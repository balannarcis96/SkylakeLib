cmake_minimum_required (VERSION 3.8)    
project("SkylakeLibProtocol" VERSION 1.0.0)

set(SKYLAKE_SRC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

file( GLOB_RECURSE _files LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/src/*.cpp" "${SKYLAKE_SRC_ROOT}/include/*.h" )

add_library (SkylakeLibProtocol STATIC ${_files})
            
source_group("source" FILES ${_files})

# Set C++20
set_property(TARGET SkylakeLibProtocol PROPERTY CXX_STANDARD 20)

target_include_directories(SkylakeLibProtocol PUBLIC "${SKYLAKE_SRC_ROOT}/include/")

if(SKL_HEADER_ONLY_BUILD_TYPE STREQUAL "Static_Dev")
    target_link_libraries(SkylakeLibProtocol PUBLIC SkylakeLibHeaderOnly)
elseif(SKL_HEADER_ONLY_BUILD_TYPE STREQUAL "Interface")
    target_link_libraries(SkylakeLibProtocol INTERFACE SkylakeLibHeaderOnly)
else()
    message(FATAL_ERROR "Invalid SkylakeLibHeaderOnly build type! [${SKL_HEADER_ONLY_BUILD_TYPE}]")
endif()
     