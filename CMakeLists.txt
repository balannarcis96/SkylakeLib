cmake_minimum_required (VERSION 3.8)    
project ("SkylakeLib_repo" VERSION 1.0.0)

set(SKL_CACHE_LINE_MEM_MANAGER_DESC "Allign all memory blocks inside the MemoryManager to the cache line, removing any possiblity of false sharing between memory blocks in the MemoryManager at the cost of more memory consumtion" )
set(SKL_USE_PRECISE_SLEEP_DESC "Use very precise sleep function for the active, non-task-handling workers")
set(SKL_DEBUG_MEMORY_ALLOCATORS_DESC "Enable checks for correct allocation/deallocation")
set(SKL_ENABLE_LIKELY_FLAGS_DESC "SkylakeLib is laced out with SKL_LIKELY, enable SKL_LIKELY?")
set(SKL_ENABLE_UNLIKELY_FLAGS_DESC "SkylakeLib is laced out with SKL_UNLIKELY, enable SKL_UNLIKELY?")

option(SKL_BUILD_TESTS                   "Build tests"                               ON)
option(SKL_DISABLE_EXCEPTIONS            "Disable exceptions[Recommended]!"         OFF)
option(SKL_BUILD_SHIPPING                "Build for shipping"                       OFF)
option(SKL_MEMORY_STATISTICS             "[DevOnly] Enable memory statistics"       OFF)
option(SKL_NO_ASSERTS                    "Disabled all runtime asserts"             OFF)
option(SKL_GUARD_ALLOC_SIZE              "Cap allocation sizes to a max value"       ON)
option(SKL_USE_PRECISE_SLEEP             "${SKL_USE_PRECISE_SLEEP_DESC}"             ON)
option(SKL_CACHE_LINE_MEM_MANAGER        "${SKL_CACHE_LINE_MEM_MANAGER_DESC}"        ON)
option(SKL_DEBUG_MEMORY_ALLOCATORS       "${SKL_DEBUG_MEMORY_ALLOCATORS_DESC}"       ON)
option(SKL_BUILD_DB                      "Build the DB library"                      ON)
option(SKL_BUILD_DATACENTER              "Build the Datacenter library"              ON)
option(SKL_ADD_TEST_LIBS                 "Add gmock and gtest libs"                  ON)
option(SKL_NO_NAMESPACE                  "Collapse the SKL namespace"               OFF)
option(SKL_MATH                          "Enable SkylakeLib Math abstractions"       ON)
option(SKL_MYSQL_COMPRESS_NET            "Enable MYSQL network traffic compression"  ON)
option(SKL_LINK_TIME_OPTIMIZATION        "Enable Link-time optimization"             ON)
option(SKL_USE_MIMALLOC   			     "Use MiMalloc instead of malloc"           OFF)
option(SKL_ENABLE_ASAN                   "Enable asan"                              OFF)
option(SKL_DO_NOT_MODIFY_COMIPLER_FLAGS  "Set this ON for easier integration"       OFF)
option(SKL_ENABLE_LIKELY_FLAGS           "${SKL_ENABLE_LIKELY_FLAGS_DESC}"          OFF)
option(SKL_ENABLE_UNLIKELY_FLAGS         "${SKL_ENABLE_UNLIKELY_FLAGS_DESC}"        OFF)
option(SKL_ADD_MAGIC_ENUM		         "Include the magic_enum lib"         		 ON)
option(SKL_KPI_TLS_MEM_ALLOC_TIME		 "TLS Memory Allocations Time"               ON)
option(SKL_KPI_TLS_MEM_ALLOC_CNT		 "TLS Memory Allocations Counters"           ON)

# KPIs
option(SKL_KPI_QUEUE_SIZES		         "KPI - Sample Queue Sizes API"	     ON)
option(SKL_KPI_WORKER_TICK		         "KPI - Per Worker Tick Duration"	 ON)

#Timing - Global Memory Allocations
set(SKL_KPI_MEM_ALLOC_TIME_OPTIONS None OSOnly All)
set(SKL_KPI_MEM_ALLOC_TIME All CACHE STRING "Global Memory Allocations Timings")
set_property(CACHE SKL_KPI_MEM_ALLOC_TIME PROPERTY STRINGS ${SKL_KPI_MEM_ALLOC_TIME_OPTIONS})

#Counter - Global Memory Allocations
set(SKL_KPI_MEM_ALLOC_CNT_OPTIONS None OSOnly All)
set(SKL_KPI_MEM_ALLOC_CNT All CACHE STRING "Global Memory Allocations Counter")
set_property(CACHE SKL_KPI_MEM_ALLOC_CNT PROPERTY STRINGS ${SKL_KPI_MEM_ALLOC_CNT_OPTIONS})

# Override this to create as many skylake lib targets as you need
set(SKL_LIB_TARGETS "SkylakeLib" CACHE STRING "Skyalke lib targets to create")

#SKL_L1_CACHE_LINE_SIZE
set(SKL_L1_CACHE_LINE_SIZE_OPTIONS 64bytes 128bytes 512bytes)
set(SKL_L1_CACHE_LINE_SIZE 64bytes CACHE STRING "Expected L1 cache line size")
set_property(CACHE SKL_L1_CACHE_LINE_SIZE PROPERTY STRINGS ${SKL_L1_CACHE_LINE_SIZE_OPTIONS})

#SKL_BUILD_OS
set(SKL_BUILD_OS_OPTIONS Win64 FreeBSD64 Ubuntu64)
set(SKL_BUILD_OS Win64 CACHE STRING "SkyaleLib target OS")
set_property(CACHE SKL_BUILD_OS PROPERTY STRINGS ${SKL_BUILD_OS_OPTIONS})

#SKL_REAL_TYPE
set(SKL_REAL_TYPE_OPTIONS Single Double)
set(SKL_REAL_TYPE Double CACHE STRING "Real type used in the math abstractions(float/double)")
set_property(CACHE SKL_REAL_TYPE PROPERTY STRINGS ${SKL_REAL_TYPE_OPTIONS})

#SKL_USE_EIS
set(SKL_USE_EIS_OPTIONS AVX2 AVX_512 None)
set(SKL_USE_EIS AVX2 CACHE STRING "What EIS(Enhanced Instruction Set) to use")
set_property(CACHE SKL_USE_EIS PROPERTY STRINGS ${SKL_USE_EIS_OPTIONS})

#SKL_LOG_LEVEL
set(SKL_LOG_LEVEL_OPTIONS DEBUG INF WRN ERR VERBOSE)
set(SKL_LIB_LOG_LEVEL DEBUG CACHE STRING "SkyaleLib minimum log level to process")
set_property(CACHE SKL_LIB_LOG_LEVEL PROPERTY STRINGS ${SKL_LOG_LEVEL_OPTIONS})

message("Compiler: ${CMAKE_CXX_COMPILER_ID} | FrontEnd: ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT} | Version: ${CMAKE_CXX_COMPILER_VERSION} | Generator: ${CMAKE_GENERATOR}")
message("Expected L1 cache size is ${SKL_L1_CACHE_LINE_SIZE}!")

if(SKL_DO_NOT_MODIFY_COMIPLER_FLAGS STREQUAL OFF)
    if (WIN32)
        set(SKL_WIN32_DEBUG_RT_FALG "MTd" CACHE STRING "[Debug] Win32 RunTime library flag")
        set(SKL_WIN32_RELEASE_RT_FALG "MT" CACHE STRING "[Release] Win32 RunTime library flag")
        set(SKL_DIABLED_COMPILER_WARNINGS "-Wno-c++98-compat-pedantic -Wno-extra-semi-stmt -Wno-global-constructors -Wno-nonportable-system-include-path -Wno-shadow-uncaptured-local -Wno-float-equal -Wno-old-style-cast")
        set(SKL_DIABLED_COMPILER_WARNINGS " ${SKL_DIABLED_COMPILER_WARNINGS} -Wno-shadow-field-in-constructor -Wno-format-non-iso -Wno-documentation-unknown-command -Wno-zero-as-null-pointer-constant -Wno-documentation")
        set(SKL_DIABLED_COMPILER_WARNINGS " ${SKL_DIABLED_COMPILER_WARNINGS} -Wno-missing-variable-declarations -Wno-suggest-override -Wno-deprecated-dynamic-exception-spec -Wno-sign-conversion -Wno-ctad-maybe-unsupported")
        set(SKL_DIABLED_COMPILER_WARNINGS " ${SKL_DIABLED_COMPILER_WARNINGS} -Wno-reserved-identifier -Wno-exit-time-destructors")
    
        set(MSVC_COMPILE_DEBUG_OPTIONS "${CMAKE_CXX_FLAGS_DEBUG} /MP /${SKL_WIN32_DEBUG_RT_FALG} /Od /GR- /std:c17 /Gd /fp:fast /Ob2 /Wall /Zc:wchar_t /WX /external:anglebrackets /wd4577 /wd4201 /wd4100 ${SKL_DIABLED_COMPILER_WARNINGS}")
        set(MSVC_COMPILE_RELWITHDEBINFO_OPTIONS "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MP /${SKL_WIN32_RELEASE_RT_FALG} /std:c17 /GR- /GS- /fp:fast /Ob2 /Wall /Zc:wchar_t /WX /external:anglebrackets /wd4577 /wd4201 /wd4100 ${SKL_DIABLED_COMPILER_WARNINGS}")
        set(MSVC_COMPILE_RELEASE_OPTIONS "${CMAKE_CXX_FLAGS_RELEASE} /MP /${SKL_WIN32_RELEASE_RT_FALG} /O2 /Ot /Gd /std:c17 /GR- /GS- /fp:fast /Ob2 /Wall /Zc:wchar_t /WX /external:anglebrackets /wd4577 /wd4201 /wd4100 ${SKL_DIABLED_COMPILER_WARNINGS}")
    
        if(SKL_LINK_TIME_OPTIMIZATION)
            if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND "${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "MSVC")
                set(MSVC_COMPILE_RELEASE_OPTIONS "${MSVC_COMPILE_RELEASE_OPTIONS} /GL")
                add_link_options("-LTCG")
            elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
                set(MSVC_COMPILE_RELEASE_OPTIONS "${MSVC_COMPILE_RELEASE_OPTIONS} /GL")
                add_link_options("/LTCG")
            else()
                message(FATAL_ERROR "@TODO compiler flags!")
            endif()
        endif()
    
        if(SKL_ENABLE_ASAN)
            if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
                set(MSVC_COMPILE_DEBUG_OPTIONS "${MSVC_COMPILE_DEBUG_OPTIONS} /fsanitize=address")
                set(MSVC_COMPILE_RELWITHDEBINFO_OPTIONS "${MSVC_COMPILE_RELWITHDEBINFO_OPTIONS} /fsanitize=address")
                set(MSVC_COMPILE_RELEASE_OPTIONS "${MSVC_COMPILE_RELEASE_OPTIONS} /fsanitize=address")
            else()
                message(FATAL_ERROR "@TODO compiler flags!")
            endif()
        endif()

        if(SKL_USE_EIS STREQUAL SSE2)
            set(MSVC_COMPILE_RELEASE_OPTIONS "${MSVC_COMPILE_RELEASE_OPTIONS} /arch:SSE2")
        elseif(SKL_USE_EIS STREQUAL AVX2)
            set(MSVC_COMPILE_RELEASE_OPTIONS "${MSVC_COMPILE_RELEASE_OPTIONS} /arch:AVX2")
        endif()

        set(CMAKE_CXX_FLAGS_DEBUG "${MSVC_COMPILE_DEBUG_OPTIONS}")
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${MSVC_COMPILE_RELWITHDEBINFO_OPTIONS}")
        set(CMAKE_CXX_FLAGS_RELEASE "${MSVC_COMPILE_RELEASE_OPTIONS}")
    else()
        message(FATAL_ERROR "@TODO compiler flags")
    endif()
endif()

if(SKL_USE_MIMALLOC)
	# MiMalloc Options
	option(MI_USE_CXX           "Use the C++ compiler to compile the library (instead of the C compiler)" ON)
	option(MI_BUILD_SHARED      "Build shared library" OFF)
	option(MI_BUILD_STATIC      "Build static library" ON)
	option(MI_BUILD_OBJECT      "Build object library" OFF)
	option(MI_BUILD_TESTS       "Build test executables" OFF)

	if(NOT SKL_BUILD_SHIPPING)
		option(MI_SECURE 			"Use full security mitigations (like guard pages, allocation randomization, double-free mitigation, and free-list corruption detection)" ON)
		option(MI_DEBUG_FULL        "Use full internal heap invariant checking in DEBUG mode (expensive)" ON)
		option(MI_PADDING           "Enable padding to detect heap block overflow (used only in DEBUG mode or with Valgrind)" ON)
		option(MI_SKIP_COLLECT_ON_EXIT, "Skip collecting memory on program exit" OFF)
	else()
		option(MI_SECURE 			"Use full security mitigations (like guard pages, allocation randomization, double-free mitigation, and free-list corruption detection)" OFF)
		option(MI_DEBUG_FULL        "Use full internal heap invariant checking in DEBUG mode (expensive)" OFF)
		option(MI_PADDING           "Enable padding to detect heap block overflow (used only in DEBUG mode or with Valgrind)" OFF)
		option(MI_SKIP_COLLECT_ON_EXIT, "Skip collecting memory on program exit" ON)
	endif()
endif()	

################################################################

add_subdirectory("ThirdParty")
    
if(SKL_USE_MIMALLOC)
	add_subdirectory("SkylakeLib/MiMalloc")
endif()

# SkylakeLib Stack
add_subdirectory("SkylakeLibHeaderOnly")
add_subdirectory("SkylakeLibStandalone")
add_subdirectory("SkylakeLib")

if(SKL_BUILD_DATACENTER)
    add_subdirectory("SkylakeDatacenterLib")
endif()

if(SKL_BUILD_DB)
    add_subdirectory("SkylakeLibDB")
endif() 

################################################################

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DSKL_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DSKL_DEBUG")

if(SKL_BUILD_OS STREQUAL "Win64")
    message("Building for Windows 64bit!")
elseif(SKL_BUILD_OS STREQUAL "FreeBSD")
    message(FATAL_ERROR "FreeBSD not supported yet!")
elseif(SKL_BUILD_OS STREQUAL "Ubuntu")
    message(FATAL_ERROR "Ubuntu not supported yet!")
endif()

if(SKL_BUILD_SHIPPING)
    message("Building SHIPPING!")
endif()

if(SKL_DISABLE_EXCEPTIONS)
    if (WIN32)
        string(REPLACE "/GR" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}) # Disable Exceptions
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-") #Disable RTTI

        string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS}) # Disable Exceptions
        string(REPLACE "/EHa" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS}) # Disable Exceptions
        string(REPLACE "/EHs" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS}) # Disable Exceptions
    
        string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS})# Disable Exceptions
        string(REPLACE "/EHa" "" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS}) # Disable Exceptions
        string(REPLACE "/EHs" "" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS}) # Disable Exceptions
    endif()
endif()

##### Create all SkyalekLib Targets #####
foreach(target_name ${SKL_LIB_TARGETS})
    Create_SkylakeLibTarget(${target_name})
endforeach()

##### Tests #####
if(SKL_BUILD_TESTS)
    enable_testing()

    if(SKL_ADD_TEST_LIBS)
        # google test lib
        add_subdirectory("googletest")    
        
        set_property(TARGET gtest PROPERTY CXX_STANDARD 20)
        set_property(TARGET gtest_main PROPERTY CXX_STANDARD 20)
        set_property(TARGET gmock PROPERTY CXX_STANDARD 20)
        set_property(TARGET gmock_main PROPERTY CXX_STANDARD 20)
    endif()
    
    # SKL tests 
    add_subdirectory ("tests")
endif()
