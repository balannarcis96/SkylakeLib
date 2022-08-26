cmake_minimum_required (VERSION 3.8)
project ("SkylakeLib_repo" VERSION 1.0.0)

set(SKL_CACHE_LINE_MEM_MANAGER_DESC "Allign all memory blocks inside the MemoryManager to the cache line, removing any possiblity of false sharing between memory blocks in the MemoryManager at the cost of more memory consumtion" )
set(SKL_USE_PRECISE_SLEEP_DESC "Use very precise sleep function for the active, non-task-handling workers")
set(SKL_DEBUG_MEMORY_ALLOCATORS_DESC "Enable checks for correct allocation/deallocation")

option(SKL_BUILD_TESTS              "Build tests"                          ON)  
option(SKL_DISABLE_EXCEPTIONS       "Disable exceptions[Recommended]!"     ON)   
option(SKL_BUILD_SHIPPING           "Build for shipping"                  OFF) 
option(SKL_MEMORY_STATISTICS        "Enable memory statistics"             ON) 
option(SKL_NO_ASSERTS               "Disabled all runtime asserts"        OFF) 
option(SKL_GUARD_ALLOC_SIZE         "Cap allocation sizes to a max value"  ON)
option(SKL_USE_PRECISE_SLEEP        "${SKL_USE_PRECISE_SLEEP_DESC}"        ON)
option(SKL_CACHE_LINE_MEM_MANAGER   "${SKL_CACHE_LINE_MEM_MANAGER_DESC}"   ON)
option(SKL_DEBUG_MEMORY_ALLOCATORS  "${SKL_DEBUG_MEMORY_ALLOCATORS_DESC}"  ON)

#SKL_L1_CACHE_LINE_SIZE
set(SKL_L1_CACHE_LINE_SIZE_OPTIONS 64bytes 128bytes 512bytes)
set(SKL_L1_CACHE_LINE_SIZE 64bytes CACHE STRING "Expected L1 cache line size")
set_property(CACHE SKL_L1_CACHE_LINE_SIZE PROPERTY STRINGS ${SKL_L1_CACHE_LINE_SIZE_OPTIONS})

#SKL_BUILD_TYPE
set(SKL_BUILD_TYPE_OPTIONS Full HeaderOnly)
set(SKL_BUILD_TYPE Full CACHE STRING "SkyaleLib build type")
set_property(CACHE SKL_BUILD_TYPE PROPERTY STRINGS ${SKL_BUILD_TYPE_OPTIONS})

#SKL_BUILD_OS
set(SKL_BUILD_OS_OPTIONS Win64 FreeBSD64 Ubuntu64)
set(SKL_BUILD_OS Win64 CACHE STRING "SkyaleLib target OS")
set_property(CACHE SKL_BUILD_OS PROPERTY STRINGS ${SKL_BUILD_OS_OPTIONS})

#SKL_HEADER_ONLY_BUILD_TYPE
set(SKL_HEADER_ONLY_BUILD_TYPE_OPTIONS Interface Static_Dev)
set(SKL_HEADER_ONLY_BUILD_TYPE Static_Dev CACHE STRING "SkylakeHeaderOnly build type")
set_property(CACHE SKL_HEADER_ONLY_BUILD_TYPE PROPERTY STRINGS ${SKL_HEADER_ONLY_BUILD_TYPE_OPTIONS})

#SKL_REAL_TYPE
set(SKL_REAL_TYPE_OPTIONS Single Double)
set(SKL_REAL_TYPE Single CACHE STRING "Skylake real type (float/double)")
set_property(CACHE SKL_REAL_TYPE PROPERTY STRINGS ${SKL_REAL_TYPE_OPTIONS})

#SKL_LOG_LEVEL
set(SKL_LOG_LEVEL_OPTIONS DEBUG INF WRN ERR VERBOSE)
set(SKL_LOG_LEVEL DEBUG CACHE STRING "SkyaleLib minimum log level to process")
set_property(CACHE SKL_LOG_LEVEL PROPERTY STRINGS ${SKL_LOG_LEVEL_OPTIONS})

if(SKL_BUILD_TESTS)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(MSVC_COMPILE_OPTIONS "/MP /MT")
        set(MSVC_COMPILE_DEBUG_OPTIONS "${CMAKE_CXX_FLAGS_DEBUG} ${MSVC_COMPILE_OPTIONS} /ZI /Od")
        set(MSVC_COMPILE_RELEASE_OPTIONS "${CMAKE_CXX_FLAGS_RELEASE} ${MSVC_COMPILE_OPTIONS} /O2")
        set(CMAKE_CXX_FLAGS_DEBUG "${MSVC_COMPILE_DEBUG_OPTIONS}")
        set(CMAKE_CXX_FLAGS_RELEASE "${MSVC_COMPILE_RELEASE_OPTIONS}")
    endif()
    add_compile_definitions(SKL_TEST_ENABLED=1)
else()
    add_compile_definitions(SKL_TEST_ENABLED=0)
endif()

if(SKL_REAL_TYPE STREQUAL "Single")
    add_compile_definitions(SKL_REAL_TYPE_SINGLE)
elseif(SKL_REAL_TYPE STREQUAL "Double")
    add_compile_definitions(SKL_REAL_TYPE_DOUBLE)
else()
    message(FATAL_ERROR "Invalid SKL_REAL_TYPE=${SKL_REAL_TYPE}")
endif()

message("Expected L1 cache size is ${SKL_L1_CACHE_LINE_SIZE}!")
if(SKL_L1_CACHE_LINE_SIZE STREQUAL "64bytes")
    add_compile_definitions(SKL_L1_CACHE_LINE_64)
elseif(SKL_L1_CACHE_LINE_SIZE STREQUAL "128bytes")
    add_compile_definitions(SKL_L1_CACHE_LINE_128)
elseif(SKL_L1_CACHE_LINE_SIZE STREQUAL "512bytes")
    add_compile_definitions(SKL_L1_CACHE_LINE_512)
endif()

if(SKL_BUILD_OS STREQUAL "Win64")
    add_compile_definitions(SKL_BUILD_WINDOWS)
    message("Building for Windows 64bit!")
elseif(SKL_BUILD_OS STREQUAL "FreeBSD")
    add_compile_definitions(SKL_BUILD_FREEBSD)
    message(FATAL_ERROR "FreeBSD not supported yet!")
elseif(SKL_BUILD_OS STREQUAL "Ubuntu")
    add_compile_definitions(SKL_BUILD_UBUNTU)
    message(FATAL_ERROR "Ubuntu not supported yet!")
endif()

if(SKL_BUILD_SHIPPING)
    add_compile_definitions(SKL_BUILD_SHIPPING)
endif()

if(SKL_MEMORY_STATISTICS)
    add_compile_definitions(SKL_MEMORY_STATISTICS)
endif()

if(SKL_NO_ASSERTS)
    add_compile_definitions(SKL_NO_ASSERTS)
endif()

if(SKL_GUARD_ALLOC_SIZE)
    add_compile_definitions(SKL_GUARD_ALLOC_SIZE)
endif()

if(SKL_USE_PRECISE_SLEEP)
    add_compile_definitions(SKL_USE_PRECISE_SLEEP)
endif()

if(SKL_CACHE_LINE_MEM_MANAGER)
    add_compile_definitions(SKL_CACHE_LINE_MEM_MANAGER)
endif()

if(SKL_DEBUG_MEMORY_ALLOCATORS)
    add_compile_definitions(SKL_DEBUG_MEMORY_ALLOCATORS)
endif()

if(SKL_DISABLE_EXCEPTIONS)
    # Disable std exceptions
    add_compile_definitions(_HAS_EXCEPTIONS=0)
    
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        string(REPLACE "/GR" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})   # Disable RTTI
        string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}) # Disable Exceptions
    endif()
endif()

# LOG LEVEL
if(SKL_LOG_LEVEL STREQUAL VERBOSE)
    add_compile_definitions(SKL_LOG_LEVEL=0)
elseif(SKL_LOG_LEVEL STREQUAL DEBUG)
    add_compile_definitions(SKL_LOG_LEVEL=1)
elseif(SKL_LOG_LEVEL STREQUAL ERR)
    add_compile_definitions(SKL_LOG_LEVEL=2)
elseif(SKL_LOG_LEVEL STREQUAL WRN)
    add_compile_definitions(SKL_LOG_LEVEL=3)
elseif(SKL_LOG_LEVEL STREQUAL INF)
    add_compile_definitions(SKL_LOG_LEVEL=4)
endif()

################################################################

add_subdirectory("SkylakeLibHeaderOnly")

add_subdirectory("SkylakeLib/MiMalloc")

if(SKL_BUILD_TYPE STREQUAL "Full")
    add_subdirectory("SkylakeLib")

    if(SKL_BUILD_TESTS)
        enable_testing()

        # google test lib
        add_subdirectory("googletest")    
        
        # SKL tests 
        add_subdirectory ("tests")
    endif()
endif()
