cmake_minimum_required (VERSION 3.8)
project ("SkylakeLib_repo" VERSION 1.0.0)

set(SKL_CACHE_LINE_MEM_MANAGER_DESC "Allign all memory blocks inside the MemoryManager to the cache line, removing any possiblity of false sharing between memory blocks in the MemoryManager at the cost of more memory consumtion" )
set(SKL_USE_PRECISE_SLEEP_DESC "Use very precise sleep function for the active, non-task-handling workers")
set(SKL_DEBUG_MEMORY_ALLOCATORS_DESC "Enable checks for correct allocation/deallocation")

option(SKL_BUILD_TESTS              "Build tests"                               ON)  
option(SKL_DISABLE_EXCEPTIONS       "Disable exceptions[Recommended]!"         OFF)   
option(SKL_BUILD_SHIPPING           "Build for shipping"                       OFF) 
option(SKL_MEMORY_STATISTICS        "Enable memory statistics"                  ON) 
option(SKL_NO_ASSERTS               "Disabled all runtime asserts"             OFF) 
option(SKL_GUARD_ALLOC_SIZE         "Cap allocation sizes to a max value"       ON)
option(SKL_USE_PRECISE_SLEEP        "${SKL_USE_PRECISE_SLEEP_DESC}"             ON)
option(SKL_CACHE_LINE_MEM_MANAGER   "${SKL_CACHE_LINE_MEM_MANAGER_DESC}"        ON)
option(SKL_DEBUG_MEMORY_ALLOCATORS  "${SKL_DEBUG_MEMORY_ALLOCATORS_DESC}"       ON)
option(SKL_BUILD_DB                 "Build the DB library"                      ON)
option(SKL_BUILD_DATACENTER         "Build the Datacenter library"              ON)
option(SKL_ADD_TEST_LIBS            "Add gmock and gtest libs"                  ON)
option(SKL_NO_NAMESPACE             "Collapse the SKL namespace"               OFF)
option(SKL_MATH                     "Enable SkylakeLib Math abstractions"       ON)
option(SKL_MYSQL_COMPRESS_NET       "Enable MYSQL network traffic compression"  ON)
option(SKL_LINK_TIME_OPTIMIZATION   "Enable Link-time optimization"             ON)
option(SKL_USE_MIMALLOC   			"Use MiMalloc instead of malloc"           OFF)

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

if (WIN32)
    set(MSVC_COMPILE_DEBUG_OPTIONS "${CMAKE_CXX_FLAGS_DEBUG} /MP /MTd /Od /GR- /std:c17 /Gd /fp:fast /Ob2 /W4 /Zc:wchar_t /WX /external:anglebrackets /wd4577 /wd4201 /wd4100")
    set(MSVC_COMPILE_RELEASE_OPTIONS "${CMAKE_CXX_FLAGS_RELEASE} /MP /MT /O2 /Ot /Gd /std:c17 /GR- /fp:fast /Ob2 /W4 /Zc:wchar_t /WX /external:anglebrackets /wd4577 /wd4201 /wd4100")
    
    if(SKL_LINK_TIME_OPTIMIZATION)
        set(MSVC_COMPILE_RELEASE_OPTIONS "${MSVC_COMPILE_RELEASE_OPTIONS} /GL")
        add_link_options("/LTCG")
    endif()
    
    if(SKL_USE_EIS STREQUAL SSE2)
        set(MSVC_COMPILE_RELEASE_OPTIONS "${MSVC_COMPILE_RELEASE_OPTIONS} /arch:SSE2")
    elseif(SKL_USE_EIS STREQUAL AVX2)
        set(MSVC_COMPILE_RELEASE_OPTIONS "${MSVC_COMPILE_RELEASE_OPTIONS} /arch:AVX2")
    endif()

    set(CMAKE_CXX_FLAGS_DEBUG "${MSVC_COMPILE_DEBUG_OPTIONS}")
    set(CMAKE_CXX_FLAGS_RELEASE "${MSVC_COMPILE_RELEASE_OPTIONS}")
else()
    message(FATAL_ERROR "@TODO compiler flags")
endif()

if(SKL_USE_MIMALLOC)
	# MiMalloc Options
	option(MI_USE_CXX           "Use the C++ compiler to compile the library (instead of the C compiler)" ON)
	option(MI_BUILD_SHARED      "Build shared library" OFF)
	option(MI_BUILD_STATIC      "Build static library" ON)
	option(MI_BUILD_OBJECT      "Build object library" OFF)
	option(MI_BUILD_TESTS       "Build test executables" OFF)

	if(!SKL_BUILD_SHIPPING)
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
    
add_subdirectory("SkylakeLibHeaderOnly")

if(SKL_USE_MIMALLOC)
	add_subdirectory("SkylakeLib/MiMalloc")
endif()

add_subdirectory("SkylakeLibStandalone")

add_subdirectory("SkylakeLib")

if(SKL_BUILD_DATACENTER)
    add_subdirectory("SkylakeDatacenterLib")
endif()

if(SKL_BUILD_DB)
    add_subdirectory("SkylakeLibDB")
    
    if(SKL_MYSQL_COMPRESS_NET)
        target_compile_definitions(SkylakeLibDB PUBLIC SKL_MYSQL_COMPRESS_NET)
    endif()
endif() 

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

################################################################

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DSKL_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DSKL_DEBUG")

if(SKL_USE_MIMALLOC)
    target_compile_definitions(SkylakeLib PUBLIC SKL_USE_MIMALLOC)
endif()

if(SKL_BUILD_TESTS)
    target_compile_definitions(SkylakeLibStandalone PUBLIC SKL_TEST_ENABLED=1)
else()
    target_compile_definitions(SkylakeLibStandalone PUBLIC SKL_TEST_ENABLED=0)
endif()

if(SKL_NO_NAMESPACE)
    target_compile_definitions(SkylakeLib PUBLIC SKL_NO_NAMESPACE)
    target_compile_definitions(SkylakeLibStandalone PUBLIC SKL_STANDALONE_NO_NAMESPACE)
endif()

if(SKL_REAL_TYPE STREQUAL "Single")
    target_compile_definitions(SkylakeLibStandalone PUBLIC SKL_REAL_TYPE_SINGLE)
elseif(SKL_REAL_TYPE STREQUAL "Double")
    target_compile_definitions(SkylakeLibStandalone PUBLIC SKL_REAL_TYPE_DOUBLE)
    target_compile_definitions(SkylakeLibStandalone PUBLIC SKL_USE_LARGE_WORLD_COORDS)
    
    target_compile_definitions(SkylakeLibHeaderOnly PUBLIC SKL_REAL_TYPE_DOUBLE)
    target_compile_definitions(SkylakeLibHeaderOnly PUBLIC SKL_USE_LARGE_WORLD_COORDS)
else()
    message(FATAL_ERROR "Invalid SKL_REAL_TYPE=${SKL_REAL_TYPE}")
endif()

message("Expected L1 cache size is ${SKL_L1_CACHE_LINE_SIZE}!")
if(SKL_L1_CACHE_LINE_SIZE STREQUAL "64bytes")
    target_compile_definitions(SkylakeLibHeaderOnly PUBLIC SKL_L1_CACHE_LINE_64)
    target_compile_definitions(SkylakeLibStandalone PUBLIC SKL_L1_CACHE_LINE_64)
elseif(SKL_L1_CACHE_LINE_SIZE STREQUAL "128bytes")
    target_compile_definitions(SkylakeLibStandalone PUBLIC SKL_L1_CACHE_LINE_128)
    target_compile_definitions(SkylakeLibHeaderOnly PUBLIC SKL_L1_CACHE_LINE_128)
elseif(SKL_L1_CACHE_LINE_SIZE STREQUAL "512bytes")
    target_compile_definitions(SkylakeLibStandalone PUBLIC SKL_L1_CACHE_LINE_512)
    target_compile_definitions(SkylakeLibHeaderOnly PUBLIC SKL_L1_CACHE_LINE_512)
endif()

if(SKL_BUILD_OS STREQUAL "Win64")
    target_compile_definitions(SkylakeLib PUBLIC SKL_BUILD_WINDOWS)
    message("Building for Windows 64bit!")
elseif(SKL_BUILD_OS STREQUAL "FreeBSD")
    target_compile_definitions(SkylakeLib PUBLIC SKL_BUILD_FREEBSD)
    message(FATAL_ERROR "FreeBSD not supported yet!")
elseif(SKL_BUILD_OS STREQUAL "Ubuntu")
    target_compile_definitions(SkylakeLib PUBLIC SKL_BUILD_UBUNTU)
    message(FATAL_ERROR "Ubuntu not supported yet!")
endif()

if(SKL_BUILD_SHIPPING)
    target_compile_definitions(SkylakeLibStandalone PUBLIC SKL_BUILD_SHIPPING)
endif()

if(SKL_MEMORY_STATISTICS)
    target_compile_definitions(SkylakeLib PUBLIC SKL_MEMORY_STATISTICS)
endif()

if(SKL_NO_ASSERTS)
    target_compile_definitions(SkylakeLibStandalone PUBLIC SKL_NO_ASSERTS)
endif()

if(SKL_GUARD_ALLOC_SIZE)
    target_compile_definitions(SkylakeLib PUBLIC SKL_GUARD_ALLOC_SIZE)
endif()

if(SKL_USE_PRECISE_SLEEP)
    target_compile_definitions(SkylakeLib PUBLIC SKL_USE_PRECISE_SLEEP)
endif()

if(SKL_CACHE_LINE_MEM_MANAGER)
    target_compile_definitions(SkylakeLib PUBLIC SKL_CACHE_LINE_MEM_MANAGER)
endif()

if(SKL_DEBUG_MEMORY_ALLOCATORS)
    target_compile_definitions(SkylakeLib PUBLIC SKL_DEBUG_MEMORY_ALLOCATORS)
endif()

if(SKL_MATH)
    target_compile_definitions(SkylakeLib PUBLIC SKL_MATH)
endif()

if(SKL_DISABLE_EXCEPTIONS)
    # Disable std exceptions
    target_compile_definitions(SkylakeLibStandalone PUBLIC _HAS_EXCEPTIONS=0)
    if (WIN32)
        string(REPLACE "/GR" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS})   # Disable RTTI
        string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS}) # Disable Exceptions
        string(REPLACE "/EHa" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS}) # Disable Exceptions
        string(REPLACE "/EHs" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS}) # Disable Exceptions
    
        string(REPLACE "/GR" "" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS})   # Disable RTTI
        string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS}) # Disable Exceptions
        string(REPLACE "/EHa" "" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS}) # Disable Exceptions
        string(REPLACE "/EHs" "" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS}) # Disable Exceptions
    endif()
endif()

# LOG LEVEL
if(SKL_LIB_LOG_LEVEL STREQUAL VERBOSE)
    target_compile_definitions(SkylakeLibStandalone PUBLIC SKLL_LOG_LEVEL=0)
elseif(SKL_LIB_LOG_LEVEL STREQUAL DEBUG)
    target_compile_definitions(SkylakeLibStandalone PUBLIC SKLL_LOG_LEVEL=1)
elseif(SKL_LIB_LOG_LEVEL STREQUAL ERR)
    target_compile_definitions(SkylakeLibStandalone PUBLIC SKLL_LOG_LEVEL=2)
elseif(SKL_LIB_LOG_LEVEL STREQUAL WRN)
    target_compile_definitions(SkylakeLibStandalone PUBLIC SKLL_LOG_LEVEL=3)
elseif(SKL_LIB_LOG_LEVEL STREQUAL INF)
    target_compile_definitions(SkylakeLibStandalone PUBLIC SKLL_LOG_LEVEL=4)
endif()

if(SKL_USE_EIS STREQUAL AVX2)
    target_compile_definitions(SkylakeLib PUBLIC SKL_USE_AVX2)
elseif(SKL_USE_EIS STREQUAL AVX_512)
    target_compile_definitions(SkylakeLib PUBLIC SKL_USE_AVX_512)
endif()
