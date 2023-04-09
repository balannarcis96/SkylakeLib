cmake_minimum_required (VERSION 3.8)    
project("SkylakeLib" VERSION 1.0.0)

set(SKYLAKE_SRC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}" CACHE STRING "SkylakeLib Root Dir")

file( GLOB_RECURSE _SKL_LIB_AdvancedSingleDispatch_files LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/AdvancedSingleDispatch/*.cpp" "${SKYLAKE_SRC_ROOT}/AdvancedSingleDispatch/*.h")
file( GLOB_RECURSE _SKL_LIB_Application_files            LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Application/*.cpp"            "${SKYLAKE_SRC_ROOT}/Application/*.h" )
file( GLOB_RECURSE _SKL_LIB_AOD_files                    LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/AOD/*.cpp"                    "${SKYLAKE_SRC_ROOT}/AOD/*.h" )
file( GLOB_RECURSE _SKL_LIB_Diagnostics_files            LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Diagnostics/*.cpp"            "${SKYLAKE_SRC_ROOT}/Diagnostics/*.h" )
file( GLOB_RECURSE _SKL_LIB_ECS_files                    LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/ECS/*.cpp"                    "${SKYLAKE_SRC_ROOT}/ECS/*.h" )
file( GLOB_RECURSE _SKL_LIB_Enums_files                  LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Enums/*.cpp"                  "${SKYLAKE_SRC_ROOT}/Enums/*.h" )
file( GLOB_RECURSE _SKL_LIB_Macros_files                 LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Macros/*.cpp"                 "${SKYLAKE_SRC_ROOT}/Macros/*.h" )
file( GLOB_RECURSE _SKL_LIB_Math_files                   LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Math/*.cpp"                   "${SKYLAKE_SRC_ROOT}/Math/*.h" )
file( GLOB_RECURSE _SKL_LIB_Memory_files                 LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Memory/*.cpp"                 "${SKYLAKE_SRC_ROOT}/Memory/*.h" )
file( GLOB_RECURSE _SKL_LIB_Math_files                   LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Math/*.cpp"                   "${SKYLAKE_SRC_ROOT}/Math/*.h" )
file( GLOB_RECURSE _SKL_LIB_Networking_files             LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Networking/*.cpp"             "${SKYLAKE_SRC_ROOT}/Networking/*.h" )
file( GLOB_RECURSE _SKL_LIB_Port_files                   LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Port/*.cpp"                   "${SKYLAKE_SRC_ROOT}/Port/*.h" )
file( GLOB         _SKL_LIB_Root_files                   LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/*.cpp"                        "${SKYLAKE_SRC_ROOT}/*.h" )
file( GLOB_RECURSE _SKL_LIB_Service_files                LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Service/*.cpp"                "${SKYLAKE_SRC_ROOT}/Service/*.h" )
file( GLOB_RECURSE _SKL_LIB_Std_files                    LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Std/*.cpp"                    "${SKYLAKE_SRC_ROOT}/Std/*.h" )
file( GLOB_RECURSE _SKL_LIB_Task_files                   LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Task/*.cpp"                   "${SKYLAKE_SRC_ROOT}/Task/*.h" )
file( GLOB_RECURSE _SKL_LIB_Tuning_files                 LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Tuning/*.cpp"                 "${SKYLAKE_SRC_ROOT}/Tuning/*.h" )
file( GLOB_RECURSE _SKL_LIB_Types_files                  LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Types/*.cpp"                  "${SKYLAKE_SRC_ROOT}/Types/*.h" )
file( GLOB_RECURSE _SKL_LIB_Threading_files              LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Threading/*.cpp"              "${SKYLAKE_SRC_ROOT}/Threading/*.h" )
file( GLOB_RECURSE _SKL_LIB_TLSSync_files                LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/TLSSync/*.cpp"                "${SKYLAKE_SRC_ROOT}/TLSSync/*.h" )
file( GLOB_RECURSE _SKL_LIB_Utils_files                  LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Utils/*.cpp"                  "${SKYLAKE_SRC_ROOT}/Utils/*.h" )
file( GLOB_RECURSE _SKL_LIB_Measurements_files           LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Measurements/*.cpp"           "${SKYLAKE_SRC_ROOT}/Measurements/*.h" )

#####SkylakeLib Files #####
set_property(GLOBAL PROPERTY USE_FOLDERS YES)
source_group("AdvancedSingleDispatch" FILES ${_SKL_LIB_AdvancedSingleDispatch_files})
source_group("Application"            FILES ${_SKL_LIB_Application_files}           )
source_group("AOD"                    FILES ${_SKL_LIB_AOD_files}                   )
source_group("Diagnostics"            FILES ${_SKL_LIB_Diagnostics_files}           )
source_group("ECS"                    FILES ${_SKL_LIB_ECS_files}                   )
source_group("Enums"                  FILES ${_SKL_LIB_Enums_files}                 )
source_group("Math"                   FILES ${_SKL_LIB_Macros_files}                )
source_group("Macros"                 FILES ${_SKL_LIB_Macros_files}                )
source_group("Memory"                 FILES ${_SKL_LIB_Memory_files}                )
source_group("Math"                   FILES ${_SKL_LIB_Math_files}                  )
source_group("Networking"             FILES ${_SKL_LIB_Networking_files}            )
source_group("Port"                   FILES ${_SKL_LIB_Port_files}                  )
source_group("/"                      FILES ${_SKL_LIB_Root_files}                  )
source_group("Service"                FILES ${_SKL_LIB_Service_files}               )
source_group("Std"                    FILES ${_SKL_LIB_Std_files}                   )
source_group("Task"                   FILES ${_SKL_LIB_Task_files}                  )
source_group("Tuning"                 FILES ${_SKL_LIB_Tuning_files}                )
source_group("Types"                  FILES ${_SKL_LIB_Types_files}                 )
source_group("Threading"              FILES ${_SKL_LIB_Threading_files}             )
source_group("TLSSync"                FILES ${_SKL_LIB_TLSSync_files}               )
source_group("Utils"                  FILES ${_SKL_LIB_Utils_files}                 ) 
source_group("Measurements"           FILES ${_SKL_LIB_Measurements_files}          ) 

set( SKL_LIB_AdvancedSingleDispatch_files "${_SKL_LIB_AdvancedSingleDispatch_files}" CACHE STRING "SkylakeLib Files" )
set( SKL_LIB_Application_files            "${_SKL_LIB_Application_files}"            CACHE STRING "SkylakeLib Files" )
set( SKL_LIB_AOD_files                    "${_SKL_LIB_AOD_files}"                    CACHE STRING "SkylakeLib Files" )
set( SKL_LIB_Diagnostics_files            "${_SKL_LIB_Diagnostics_files}"            CACHE STRING "SkylakeLib Files" )
set( SKL_LIB_ECS_files                    "${_SKL_LIB_ECS_files}"                    CACHE STRING "SkylakeLib Files" )
set( SKL_LIB_Enums_files                  "${_SKL_LIB_Enums_files}"                  CACHE STRING "SkylakeLib Files" )
set( SKL_LIB_Macros_files                 "${_SKL_LIB_Macros_files}"                 CACHE STRING "SkylakeLib Files" )
set( SKL_LIB_Math_files                   "${_SKL_LIB_Math_files}"                   CACHE STRING "SkylakeLib Files" )
set( SKL_LIB_Memory_files                 "${_SKL_LIB_Memory_files}"                 CACHE STRING "SkylakeLib Files" )
set( SKL_LIB_Math_files                   "${_SKL_LIB_Math_files}"                   CACHE STRING "SkylakeLib Files" )
set( SKL_LIB_Networking_files             "${_SKL_LIB_Networking_files}"             CACHE STRING "SkylakeLib Files" )
set( SKL_LIB_Port_files                   "${_SKL_LIB_Port_files}"                   CACHE STRING "SkylakeLib Files" )
set( SKL_LIB_Root_files                   "${_SKL_LIB_Root_files}"                   CACHE STRING "SkylakeLib Files" )
set( SKL_LIB_Service_files                "${_SKL_LIB_Service_files}"                CACHE STRING "SkylakeLib Files" )
set( SKL_LIB_Std_files                    "${_SKL_LIB_Std_files}"                    CACHE STRING "SkylakeLib Files" )
set( SKL_LIB_Task_files                   "${_SKL_LIB_Task_files}"                   CACHE STRING "SkylakeLib Files" )
set( SKL_LIB_Tuning_files                 "${_SKL_LIB_Tuning_files}"                 CACHE STRING "SkylakeLib Files" )
set( SKL_LIB_Types_files                  "${_SKL_LIB_Types_files}"                  CACHE STRING "SkylakeLib Files" )
set( SKL_LIB_Threading_files              "${_SKL_LIB_Threading_files}"              CACHE STRING "SkylakeLib Files" )
set( SKL_LIB_TLSSync_files                "${_SKL_LIB_TLSSync_files}"                CACHE STRING "SkylakeLib Files" )
set( SKL_LIB_Utils_files                  "${_SKL_LIB_Utils_files}"                  CACHE STRING "SkylakeLib Files" )
set( SKL_LIB_Measurements_files           "${_SKL_LIB_Measurements_files}"           CACHE STRING "SkylakeLib Files" )
	
function(Create_SkylakeLibTarget target_postfix)
    Create_SkylakeLibTarget_Name(target_name ${target_postfix})
    Create_SkylakeStandaloneLibTarget_Name(standalone_target_name ${target_postfix})

    add_library (${target_name} STATIC  
        ${SKL_LIB_AdvancedSingleDispatch_files}
        ${SKL_LIB_Application_files}
        ${SKL_LIB_AOD_files}                 
        ${SKL_LIB_Diagnostics_files}               
        ${SKL_LIB_ECS_files}              
        ${SKL_LIB_Enums_files}                 
        ${SKL_LIB_Math_files}                   
        ${SKL_LIB_Macros_files}                  
        ${SKL_LIB_Math_files}                
        ${SKL_LIB_Memory_files}                 
        ${SKL_LIB_Networking_files}                   
        ${SKL_LIB_Root_files}                     
        ${SKL_LIB_Port_files}                 
        ${SKL_LIB_Service_files}                     
        ${SKL_LIB_Std_files}     
        ${SKL_LIB_Task_files}
        ${SKL_LIB_Tuning_files}                 
        ${SKL_LIB_Types_files}                 
        ${SKL_LIB_Threading_files}             
        ${SKL_LIB_TLSSync_files}     
        ${SKL_LIB_Utils_files} 
        ${SKL_LIB_Measurements_files}
    )

    # Set C++20
    set_property(TARGET ${target_name} PROPERTY CXX_STANDARD 20)

    # Set include directory
    target_include_directories(${target_name} PUBLIC "${SKYLAKE_SRC_ROOT}/")

    # Link SkylakeLibStandalone
    target_link_libraries(${target_name} PUBLIC "${standalone_target_name}")
    
    # Default Options
    set(bUseFMTLib ON)
    set(bNoNamespace ${SKL_NO_NAMESPACE})
    set(bBuildShipping ${SKL_BUILD_SHIPPING})
    set(bUseEIS ${SKL_USE_EIS})
    set(bUseMiMalloc ${SKL_USE_MIMALLOC})
    set(bMemoryStatistics ${SKL_MEMORY_STATISTICS})
    set(bGuardAllocSize ${SKL_GUARD_ALLOC_SIZE})
    set(bUsePreciseSleep ${SKL_USE_PRECISE_SLEEP})
    set(bCacheLineMemManager ${SKL_CACHE_LINE_MEM_MANAGER})
    set(bDebugMemoryAllocators ${SKL_DEBUG_MEMORY_ALLOCATORS})
    set(bMath ${SKL_MATH})
    set(bKPIsForQueueSizes ${SKL_KPI_QUEUE_SIZES})
    set(bKPIsForWorkers ${SKL_KPI_WORKER_TICK})
    set(KPIsForMemAllocCount ${SKL_KPI_MEM_ALLOC_CNT})
    set(bKPIsForTLSMemAllocCount ${SKL_KPI_TLS_MEM_ALLOC_CNT})
    set(KPIsForMemAllocTiming ${SKL_KPI_MEM_ALLOC_TIME})
    set(bKPIsForTLSMemAllocTiming ${SKL_KPI_TLS_MEM_ALLOC_TIME})
    set(logLevel ${SKL_LIB_LOG_LEVEL})
    set(targetOS ${SKL_BUILD_OS})
    set(realType ${SKL_REAL_TYPE})

    if(targetOS STREQUAL "Win64")
        target_compile_definitions(${target_name} PUBLIC SKL_BUILD_WINDOWS)
    elseif(targetOS STREQUAL "FreeBSD")
        target_compile_definitions(${target_name} PUBLIC SKL_BUILD_FREEBSD)
    elseif(targetOS STREQUAL "Ubuntu")
        target_compile_definitions(${target_name} PUBLIC SKL_BUILD_UBUNTU)
    endif()

    # fmt
	if(bUseFMTLib)
		target_link_libraries(${target_name} PUBLIC fmt::fmt)
	endif()
	
    # Link mimalloc-static
    if(bUseMiMalloc)
        target_link_libraries(${target_name} PUBLIC mimalloc-static)
        target_compile_definitions(${target_name} PUBLIC SKL_USE_MIMALLOC)
    endif()
    
    # No Namespace
    if(bNoNamespace)
        target_compile_definitions(${target_name} PUBLIC SKL_NO_NAMESPACE)
    endif()

    # Set precompiled headers
    target_precompile_headers(${target_name} PUBLIC "${SKYLAKE_SRC_ROOT}/SkylakeLib.h")
    
	# Set EIS flags
	if(bUseEIS STREQUAL AVX2)
        target_compile_definitions(${target_name} PUBLIC SKL_USE_AVX2)
	elseif(bUseEIS STREQUAL AVX_512)
		target_compile_definitions(${target_name} PUBLIC SKL_USE_AVX_512)
	endif()
	
    # [DevOnly] - MemoryStatistics
    if(bMemoryStatistics)
        target_compile_definitions(${target_name} PUBLIC SKL_MEMORY_STATISTICS)
    endif()
    
    # Guard allocation sizes
    if(bGuardAllocSize)
        target_compile_definitions(${target_name} PUBLIC SKL_GUARD_ALLOC_SIZE)
    endif()

    # Use precise sleep
    if(bUsePreciseSleep)
        target_compile_definitions(${target_name} PUBLIC SKL_USE_PRECISE_SLEEP)
    endif()

    # MemoryManager - All blocks must be cacheline alligned
    if(bCacheLineMemManager)
        target_compile_definitions(${target_name} PUBLIC SKL_CACHE_LINE_MEM_MANAGER)
    endif()

    # Compile instrumentation for debugging mem allocators
    if(bDebugMemoryAllocators)
        target_compile_definitions(${target_name} PUBLIC SKL_DEBUG_MEMORY_ALLOCATORS)
    endif()

    # Include math abstractions and utils
    if(bMath)
        target_compile_definitions(${target_name} PUBLIC SKL_MATH)
    endif()

	# Set KPI flags
	if(bKPIsForQueueSizes)
		target_compile_definitions(${target_name} PUBLIC SKL_KPI_QUEUE_SIZES)
	endif()
	if(bKPIsForWorkers)
		target_compile_definitions(${target_name} PUBLIC SKL_KPI_WORKER_TICK)
	endif()
	
	# Allocations counters
	if(KPIsForMemAllocCount STREQUAL "OsOnly")
		target_compile_definitions(${target_name} PUBLIC SKL_MEM_COUNTER_OS)
	elseif(KPIsForMemAllocCount STREQUAL "All")
		target_compile_definitions(${target_name} PUBLIC SKL_MEM_COUNTER_OS)
		target_compile_definitions(${target_name} PUBLIC SKL_MEM_COUNTER_GLOBAL)
	endif()

	if(bKPIsForTLSMemAllocCount)
		target_compile_definitions(${target_name} PUBLIC SKL_KPI_TLS_MEM_ALLOC_CNT)
	endif()

	# Allocations timings
	if(KPIsForMemAllocTiming STREQUAL "OsOnly")
		target_compile_definitions(${target_name} PUBLIC SKL_MEM_TIME_OS)
	elseif(KPIsForMemAllocTiming STREQUAL "All")
		target_compile_definitions(${target_name} PUBLIC SKL_MEM_TIME_OS)
		target_compile_definitions(${target_name} PUBLIC SKL_MEM_TIME_GLOBAL)
	endif()

	if(bKPIsForTLSMemAllocTiming)
		target_compile_definitions(${target_name} PUBLIC SKL_KPI_TLS_MEM_ALLOC_TIME)
	endif()

endfunction(Create_SkylakeLibTarget)
