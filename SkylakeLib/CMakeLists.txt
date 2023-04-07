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

##### Group SkylakeLib Files #####
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

function(Create_SkylakeLibTarget target_name)
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
    target_link_libraries(${target_name} PUBLIC SkylakeLibStandalone)
    
    if(SKL_BUILD_OS STREQUAL "Win64")
        target_compile_definitions(${target_name} PUBLIC SKL_BUILD_WINDOWS)
    elseif(SKL_BUILD_OS STREQUAL "FreeBSD")
        target_compile_definitions(${target_name} PUBLIC SKL_BUILD_FREEBSD)
    elseif(SKL_BUILD_OS STREQUAL "Ubuntu")
        target_compile_definitions(${target_name} PUBLIC SKL_BUILD_UBUNTU)
    endif()

    # Link mimalloc-static
    if(SKL_USE_MIMALLOC)
        target_link_libraries(${target_name} PUBLIC mimalloc-static)
        target_compile_definitions(${target_name} PUBLIC SKL_USE_MIMALLOC)
    endif()
    
    # No Namespace
    if(SKL_NO_NAMESPACE)
        target_compile_definitions(${target_name} PUBLIC SKL_NO_NAMESPACE)
    endif()

    # Set precompiled headers
    target_precompile_headers(${target_name} PUBLIC "${SKYLAKE_SRC_ROOT}/SkylakeLib.h")
    
	# Set EIS flags
	if(SKL_USE_EIS STREQUAL AVX2)
        target_compile_definitions(${target_name} PUBLIC SKL_USE_AVX2)
	elseif(SKL_USE_EIS STREQUAL AVX_512)
		target_compile_definitions(${target_name} PUBLIC SKL_USE_AVX_512)
	endif()
	
    # [DevOnly] - MemoryStatistics
    if(SKL_MEMORY_STATISTICS)
        target_compile_definitions(${target_name} PUBLIC SKL_MEMORY_STATISTICS)
    endif()
    
    # Guard allocation sizes
    if(SKL_GUARD_ALLOC_SIZE)
        target_compile_definitions(${target_name} PUBLIC SKL_GUARD_ALLOC_SIZE)
    endif()

    # Use precise sleep
    if(SKL_USE_PRECISE_SLEEP)
        target_compile_definitions(${target_name} PUBLIC SKL_USE_PRECISE_SLEEP)
    endif()

    # MemoryManager - All blocks must be cacheline alligned
    if(SKL_CACHE_LINE_MEM_MANAGER)
        target_compile_definitions(${target_name} PUBLIC SKL_CACHE_LINE_MEM_MANAGER)
    endif()

    # Compile instrumentation for debugging mem allocators
    if(SKL_DEBUG_MEMORY_ALLOCATORS)
        target_compile_definitions(${target_name} PUBLIC SKL_DEBUG_MEMORY_ALLOCATORS)
    endif()

    # Include math abstractions and utils
    if(SKL_MATH)
        target_compile_definitions(${target_name} PUBLIC SKL_MATH)
    endif()

	# Set KPI flags
	if(SKL_KPI_QUEUE_SIZES)
		target_compile_definitions(${target_name} PUBLIC SKL_KPI_QUEUE_SIZES)
	endif()
	if(SKL_KPI_WORKER_TICK)
		target_compile_definitions(${target_name} PUBLIC SKL_KPI_WORKER_TICK)
	endif()
	
	# Allocations counters
	if(SKL_KPI_MEM_ALLOC_CNT STREQUAL "OsOnly")
		target_compile_definitions(${target_name} PUBLIC SKL_MEM_COUNTER_OS)
	elseif(SKL_KPI_MEM_ALLOC_CNT STREQUAL "All")
		target_compile_definitions(${target_name} PUBLIC SKL_MEM_COUNTER_OS)
		target_compile_definitions(${target_name} PUBLIC SKL_MEM_COUNTER_GLOBAL)
	endif()

	if(SKL_KPI_TLS_MEM_ALLOC_CNT)
		target_compile_definitions(${target_name} PUBLIC SKL_KPI_TLS_MEM_ALLOC_CNT)
	endif()

	# Allocations timings
	if(SKL_KPI_MEM_ALLOC_TIME STREQUAL "OsOnly")
		target_compile_definitions(${target_name} PUBLIC SKL_MEM_TIME_OS)
	elseif(SKL_KPI_MEM_ALLOC_TIME STREQUAL "All")
		target_compile_definitions(${target_name} PUBLIC SKL_MEM_TIME_OS)
		target_compile_definitions(${target_name} PUBLIC SKL_MEM_TIME_GLOBAL)
	endif()

	if(SKL_KPI_TLS_MEM_ALLOC_TIME)
		target_compile_definitions(${target_name} PUBLIC SKL_KPI_TLS_MEM_ALLOC_TIME)
	endif()

    # Magic Enum
    if(SKL_ADD_MAGIC_ENUM)
		target_compile_definitions(${target_name} PUBLIC SKL_MAGIC_ENUM)
    endif()

    # fmt
	target_link_libraries(${target_name} PUBLIC fmt::fmt)
	
	# Logger
	if(SKL_USE_SERIALIZED_LOGGER)
		target_compile_definitions(${target_name} PUBLIC SKL_USE_SERIALIZED_LOGGER)
	else()
		# N/A
	endif()

endfunction(Create_SkylakeLibTarget)
