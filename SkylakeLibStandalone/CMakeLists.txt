cmake_minimum_required (VERSION 3.8)    
project("SkylakeLibStandalone" VERSION 1.0.0)

set(SKYLAKE_SRC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

file( GLOB_RECURSE _Diagnostics_files LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Diagnostics/*.cpp" "${SKYLAKE_SRC_ROOT}/Diagnostics/*.h" )
file( GLOB_RECURSE _ECS_files         LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/ECS/*.cpp"         "${SKYLAKE_SRC_ROOT}/ECS/*.h" )
file( GLOB_RECURSE _Std_files         LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Std/*.cpp"         "${SKYLAKE_SRC_ROOT}/Std/*.h" )
file( GLOB_RECURSE _Tuning_files      LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Tuning/*.cpp"      "${SKYLAKE_SRC_ROOT}/Tuning/*.h" )
file( GLOB_RECURSE _Utils_files       LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Utils/*.cpp"       "${SKYLAKE_SRC_ROOT}/Utils/*.h" )
file( GLOB         _Root_files        LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/*.cpp"             "${SKYLAKE_SRC_ROOT}/*.h" )

add_library (SkylakeLibStandalone STATIC  
    ${_Diagnostics_files}
    ${_ECS_files}
    ${_Std_files}                 
    ${_Tuning_files}               
    ${_Utils_files}
    ${_Root_files}
)

source_group("Diagnostics"            FILES ${_Diagnostics_files})
source_group("ECS"                    FILES ${_ECS_files}        )
source_group("Std"                    FILES ${_Std_files}        )
source_group("Tuning"                 FILES ${_Tuning_files}     )
source_group("Utils"                  FILES ${_Utils_files}      )
source_group("/"                      FILES ${_Root_files}       )

# Set C++20
set_property(TARGET SkylakeLibStandalone PROPERTY CXX_STANDARD 20)

target_include_directories(SkylakeLibStandalone PUBLIC "${SKYLAKE_SRC_ROOT}/")

target_precompile_headers(SkylakeLibStandalone PUBLIC "${SKYLAKE_SRC_ROOT}/SkylakeLibStandalone.h")
