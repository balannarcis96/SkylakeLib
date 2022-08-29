cmake_minimum_required (VERSION 3.8)    
project("SkylakeLibDB" VERSION 1.0.0)

set(SKYLAKE_SRC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

file( GLOB_RECURSE _files LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/src/*.cpp" "${SKYLAKE_SRC_ROOT}/include/*.h" )

add_library (SkylakeLibDB STATIC  ${_files})
            
source_group("source" FILES ${_files})

# Set C++20
set_property(TARGET SkylakeLibDB PROPERTY CXX_STANDARD 20)

target_include_directories(SkylakeLibDB PUBLIC "${SKYLAKE_SRC_ROOT}/include/")

target_link_libraries(SkylakeLibDB PUBLIC mysql)
target_link_libraries(SkylakeLibDB PUBLIC SkylakeLib)
