cmake_minimum_required (VERSION 3.8)    
project("SkylakeLib" VERSION 1.0.0)

set(SKYLAKE_SRC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

file( GLOB_RECURSE _AdvancedSingleDispatch_files LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/AdvancedSingleDispatch/*.cpp" "${SKYLAKE_SRC_ROOT}/AdvancedSingleDispatch/*.h" )
file( GLOB_RECURSE _Async_files                  LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Async/*.cpp"                  "${SKYLAKE_SRC_ROOT}/Async/*.h" )
file( GLOB_RECURSE _Diagnostics_files            LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Diagnostics/*.cpp"            "${SKYLAKE_SRC_ROOT}/Diagnostics/*.h" )
file( GLOB_RECURSE _Enums_files                  LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Enums/*.cpp"                  "${SKYLAKE_SRC_ROOT}/Enums/*.h" )
file( GLOB_RECURSE _Macros_files                 LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Macros/*.cpp"                 "${SKYLAKE_SRC_ROOT}/Macros/*.h" )
file( GLOB_RECURSE _Memory_files                 LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Memory/*.cpp"                 "${SKYLAKE_SRC_ROOT}/Memory/*.h" )
file( GLOB_RECURSE _Math_files                   LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Math/*.cpp"                   "${SKYLAKE_SRC_ROOT}/Math/*.h" )
file( GLOB_RECURSE _Networking_files             LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Networking/*.cpp"             "${SKYLAKE_SRC_ROOT}/Networking/*.h" )
file( GLOB_RECURSE _Platform_files               LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Platform/*.cpp"               "${SKYLAKE_SRC_ROOT}/Platform/*.h" )
file( GLOB         _Root_files                   LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/*.cpp"                        "${SKYLAKE_SRC_ROOT}/*.h" )
file( GLOB_RECURSE _Std_files                    LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Std/*.cpp"                    "${SKYLAKE_SRC_ROOT}/Std/*.h" )
file( GLOB_RECURSE _Task_files                   LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Task/*.cpp"                   "${SKYLAKE_SRC_ROOT}/Task/*.h" )
file( GLOB_RECURSE _Tuning_files                 LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Tuning/*.cpp"                 "${SKYLAKE_SRC_ROOT}/Tuning/*.h" )
file( GLOB_RECURSE _Types_files                  LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Types/*.cpp"                  "${SKYLAKE_SRC_ROOT}/Types/*.h" )
file( GLOB_RECURSE _Threading_files              LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Threading/*.cpp"              "${SKYLAKE_SRC_ROOT}/Threading/*.h" )
file( GLOB_RECURSE _ThreadLocalStorage_files     LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/ThreadLocalStorage/*.cpp"     "${SKYLAKE_SRC_ROOT}/ThreadLocalStorage/*.h" )
file( GLOB_RECURSE _Utils_files                  LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Utils/*.cpp"                  "${SKYLAKE_SRC_ROOT}/Utils/*.h" )

add_library (SkylakeLib STATIC  
    ${_AdvancedSingleDispatch_files}
    ${_Async_files}                 
    ${_Diagnostics_files}             
    ${_Enums_files}                 
    ${_Macros_files}                 
    ${_Memory_files}                 
    ${_Math_files}                   
    ${_Networking_files}                   
    ${_Platform_files}                 
    ${_Root_files}                     
    ${_Std_files}     
    ${_Task_files}
    ${_Tuning_files}                 
    ${_Types_files}                 
    ${_Threading_files}             
    ${_ThreadLocalStorage_files}     
    ${_Utils_files}
)
            
source_group("AdvancedSingleDispatch" FILES ${_AdvancedSingleDispatch_files})
source_group("Async"                  FILES ${_Async_files}                 )
source_group("Diagnostics"            FILES ${_Diagnostics_files}           )
source_group("Enums"                  FILES ${_Enums_files}                 )
source_group("Macros"                 FILES ${_Macros_files}                )
source_group("Memory"                 FILES ${_Memory_files}                )
source_group("Math"                   FILES ${_Math_files}                  )
source_group("Networking"             FILES ${_Networking_files}            )
source_group("Platform"               FILES ${_Platform_files}              )
source_group("/"                      FILES ${_Root_files}                  )
source_group("Std"                    FILES ${_Std_files}                   )
source_group("Task"                   FILES ${_Task_files}                  )
source_group("Tuning"                 FILES ${_Tuning_files}                )
source_group("Types"                  FILES ${_Types_files}                 )
source_group("Threading"              FILES ${_Threading_files}             )
source_group("ThreadLocalStorage"     FILES ${_ThreadLocalStorage_files}    )
source_group("Utils"                  FILES ${_Utils_files}                 )

# Set C++20
set_property(TARGET SkylakeLib PROPERTY CXX_STANDARD 20)

target_include_directories(SkylakeLib PUBLIC "${SKYLAKE_SRC_ROOT}/")

if(SKL_HEADER_ONLY_BUILD_TYPE STREQUAL "Static_Dev")
    target_link_libraries(SkylakeLib PUBLIC SkylakeLibHeaderOnly)
elseif(SKL_HEADER_ONLY_BUILD_TYPE STREQUAL "Interface")
    target_link_libraries(SkylakeLib INTERFACE SkylakeLibHeaderOnly)
else()
    message(FATAL_ERROR "Invalid SkylakeLibHeaderOnly build type! [${SKL_HEADER_ONLY_BUILD_TYPE}]")
endif()
     
target_link_libraries(SkylakeLib PUBLIC mimalloc-static)