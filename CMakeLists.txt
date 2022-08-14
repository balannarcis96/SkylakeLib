cmake_minimum_required (VERSION 3.8)
project ("SkylakeLib_repo" VERSION 1.0.0)

option(SKL_BUILD_TESTS            "Build tests"                         ON)  
option(SKL_HEADERONLY_ENABLE_MATH "SkylakeHeaderOnly enable math types" ON)   
option(SKL_BUILD_SHIPPING         "Build for shipping"                 OFF) 
option(SKL_MEMORY_STATISTICS      "Enable memory statistics"           OFF) 

#SKL_BUILD_TYPE
set(SKL_BUILD_TYPE_OPTIONS Full HeaderOnly)
set(SKL_BUILD_TYPE Full CACHE STRING "SkyaleLib build type")
set_property(CACHE SKL_BUILD_TYPE PROPERTY STRINGS ${SKL_BUILD_TYPE_OPTIONS})

#SKL_BUILD_PLATFORM
set(SKL_BUILD_PLATFORM_OPTIONS Win64 FreeBSD Ubuntu)
set(SKL_BUILD_PLATFORM Win64 CACHE STRING "SkyaleLib target platform")
set_property(CACHE SKL_BUILD_PLATFORM PROPERTY STRINGS ${SKL_BUILD_PLATFORM_OPTIONS})

#SKL_HEADER_ONLY_BUILD_TYPE
set(SKL_HEADER_ONLY_BUILD_TYPE_OPTIONS Interface Static_Dev)
set(SKL_HEADER_ONLY_BUILD_TYPE Static_Dev CACHE STRING "SkylakeHeaderOnly build type")
set_property(CACHE SKL_HEADER_ONLY_BUILD_TYPE PROPERTY STRINGS ${SKL_HEADER_ONLY_BUILD_TYPE_OPTIONS})

#SKL_REAL_TYPE
set(SKL_REAL_TYPE_OPTIONS Single Double)
set(SKL_REAL_TYPE Single CACHE STRING "Skylake real type (float/double)")
set_property(CACHE SKL_REAL_TYPE PROPERTY STRINGS ${SKL_REAL_TYPE_OPTIONS})

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

if(SKL_BUILD_PLATFORM STREQUAL "Win64")
    add_compile_definitions(SKL_BUILD_WINDOWS)
elseif(SKL_BUILD_PLATFORM STREQUAL "FreeBSD")
    add_compile_definitions(    )
    message(FATAL_ERROR "FreeBSD not supported yet!")
elseif(SKL_BUILD_PLATFORM STREQUAL "Ubuntu")
    add_compile_definitions(SKL_BUILD_UBUNTU)
    message(FATAL_ERROR "Ubuntu not supported yet!")
endif()

if(SKL_BUILD_SHIPPING)
    add_compile_definitions(SKL_BUILD_SHIPPING)
endif()

if(SKL_MEMORY_STATISTICS)
    add_compile_definitions(SKL_MEMORY_STATISTICS)
endif()

if(SKL_HEADERONLY_ENABLE_MATH)
    add_compile_definitions(SKL_HEADERONLY_ENABLE_MATH)
endif()

add_subdirectory("SkylakeLibHeaderOnly")

add_subdirectory("SkylakeLib/MiMalloc")

if(SKL_BUILD_TYPE STREQUAL "Full")
    add_subdirectory("SkylakeLib")

    if(SKL_BUILD_TESTS)
        # google test lib
        add_subdirectory("googletest")    
        
        # SKL tests 
        add_subdirectory ("tests")
    endif()
endif()