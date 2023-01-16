cmake_minimum_required (VERSION 3.8)    
project("SkylakeLib" VERSION 1.0.0)

set(SKYLAKE_SRC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

file( GLOB_RECURSE _AdvancedSingleDispatch_files LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/AdvancedSingleDispatch/*.cpp" "${SKYLAKE_SRC_ROOT}/AdvancedSingleDispatch/*.h" )
file( GLOB_RECURSE _Application_files            LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Application/*.cpp"            "${SKYLAKE_SRC_ROOT}/Application/*.h" )
file( GLOB_RECURSE _AOD_files                    LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/AOD/*.cpp"                    "${SKYLAKE_SRC_ROOT}/AOD/*.h" )
file( GLOB_RECURSE _Diagnostics_files            LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Diagnostics/*.cpp"            "${SKYLAKE_SRC_ROOT}/Diagnostics/*.h" )
file( GLOB_RECURSE _ECS_files                    LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/ECS/*.cpp"                    "${SKYLAKE_SRC_ROOT}/ECS/*.h" )
file( GLOB_RECURSE _Enums_files                  LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Enums/*.cpp"                  "${SKYLAKE_SRC_ROOT}/Enums/*.h" )
file( GLOB_RECURSE _Macros_files                 LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Macros/*.cpp"                 "${SKYLAKE_SRC_ROOT}/Macros/*.h" )
file( GLOB_RECURSE _Math_files                   LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Math/*.cpp"                   "${SKYLAKE_SRC_ROOT}/Math/*.h" )
file( GLOB_RECURSE _Memory_files                 LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Memory/*.cpp"                 "${SKYLAKE_SRC_ROOT}/Memory/*.h" )
file( GLOB_RECURSE _Math_files                   LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Math/*.cpp"                   "${SKYLAKE_SRC_ROOT}/Math/*.h" )
file( GLOB_RECURSE _Networking_files             LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Networking/*.cpp"             "${SKYLAKE_SRC_ROOT}/Networking/*.h" )
file( GLOB_RECURSE _Port_files                   LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Port/*.cpp"                   "${SKYLAKE_SRC_ROOT}/Port/*.h" )
file( GLOB         _Root_files                   LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/*.cpp"                        "${SKYLAKE_SRC_ROOT}/*.h" )
file( GLOB_RECURSE _Service_files                LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Service/*.cpp"                "${SKYLAKE_SRC_ROOT}/Service/*.h" )
file( GLOB_RECURSE _Std_files                    LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Std/*.cpp"                    "${SKYLAKE_SRC_ROOT}/Std/*.h" )
file( GLOB_RECURSE _Task_files                   LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Task/*.cpp"                   "${SKYLAKE_SRC_ROOT}/Task/*.h" )
file( GLOB_RECURSE _Tuning_files                 LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Tuning/*.cpp"                 "${SKYLAKE_SRC_ROOT}/Tuning/*.h" )
file( GLOB_RECURSE _Types_files                  LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Types/*.cpp"                  "${SKYLAKE_SRC_ROOT}/Types/*.h" )
file( GLOB_RECURSE _Threading_files              LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Threading/*.cpp"              "${SKYLAKE_SRC_ROOT}/Threading/*.h" )
file( GLOB_RECURSE _TLSSync_files                LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/TLSSync/*.cpp"                "${SKYLAKE_SRC_ROOT}/TLSSync/*.h" )
file( GLOB_RECURSE _Utils_files                  LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Utils/*.cpp"                  "${SKYLAKE_SRC_ROOT}/Utils/*.h" )

source_group("AdvancedSingleDispatch" FILES ${_AdvancedSingleDispatch_files})
source_group("Application"            FILES ${_Application_files}           )
source_group("AOD"                    FILES ${_AOD_files}                   )
source_group("Diagnostics"            FILES ${_Diagnostics_files}           )
source_group("ECS"                    FILES ${_ECS_files}                   )
source_group("Enums"                  FILES ${_Enums_files}                 )
source_group("Math"                   FILES ${_Macros_files}                )
source_group("Macros"                 FILES ${_Macros_files}                )
source_group("Memory"                 FILES ${_Memory_files}                )
source_group("Math"                   FILES ${_Math_files}                  )
source_group("Networking"             FILES ${_Networking_files}            )
source_group("Port"                   FILES ${_Port_files}                  )
source_group("/"                      FILES ${_Root_files}                  )
source_group("Service"                FILES ${_Service_files}               )
source_group("Std"                    FILES ${_Std_files}                   )
source_group("Task"                   FILES ${_Task_files}                  )
source_group("Tuning"                 FILES ${_Tuning_files}                )
source_group("Types"                  FILES ${_Types_files}                 )
source_group("Threading"              FILES ${_Threading_files}             )
source_group("TLSSync"                FILES ${_TLSSync_files}               )
source_group("Utils"                  FILES ${_Utils_files}                 )

function(Create_SkylakeLibTarget target_name)
    add_library (${target_name} STATIC  
        ${_AdvancedSingleDispatch_files}
        ${_Application_files}
        ${_AOD_files}                 
        ${_Diagnostics_files}               
        ${_ECS_files}              
        ${_Enums_files}                 
        ${_Math_files}                   
        ${_Macros_files}                  
        ${_Math_files}                
        ${_Memory_files}                 
        ${_Networking_files}                   
        ${_Root_files}                     
        ${_Port_files}                 
        ${_Service_files}                     
        ${_Std_files}     
        ${_Task_files}
        ${_Tuning_files}                 
        ${_Types_files}                 
        ${_Threading_files}             
        ${_TLSSync_files}     
        ${_Utils_files}
    )

    # Set C++20
    set_property(TARGET ${target_name} PROPERTY CXX_STANDARD 20)

    # Set include directory
    target_include_directories(${target_name} PUBLIC "${SKYLAKE_SRC_ROOT}/")

    # Link SkylakeLibStandalone
    target_link_libraries(${target_name} PUBLIC SkylakeLibStandalone)

    # Link mimalloc-static
    if(SKL_USE_MIMALLOC)
        target_link_libraries(${target_name} PUBLIC mimalloc-static)
    endif()

    # Set precompiled headers
    target_precompile_headers(${target_name} PUBLIC "${SKYLAKE_SRC_ROOT}/SkylakeLib.h")

endfunction(Create_SkylakeLibTarget)

# Create all skylake lib targets
foreach(target_name ${SKL_LIB_TARGETS})
    Create_SkylakeLibTarget(${target_name})
endforeach()