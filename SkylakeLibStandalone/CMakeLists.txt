cmake_minimum_required (VERSION 3.8)    
project("SkylakeLibStandalone" VERSION 1.0.0)

set(SKYLAKESTANDALONE_SRC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}" CACHE STRING "SkylakeStandaloneLib Root Dir")

file( GLOB_RECURSE _Diagnostics_files LIST_DIRECTORIES false "${SKYLAKESTANDALONE_SRC_ROOT}/Diagnostics/*.cpp" "${SKYLAKESTANDALONE_SRC_ROOT}/Diagnostics/*.h" )
file( GLOB_RECURSE _ECS_files         LIST_DIRECTORIES false "${SKYLAKESTANDALONE_SRC_ROOT}/ECS/*.cpp"         "${SKYLAKESTANDALONE_SRC_ROOT}/ECS/*.h" )
file( GLOB_RECURSE _Std_files         LIST_DIRECTORIES false "${SKYLAKESTANDALONE_SRC_ROOT}/Std/*.cpp"         "${SKYLAKESTANDALONE_SRC_ROOT}/Std/*.h" )
file( GLOB_RECURSE _Tuning_files      LIST_DIRECTORIES false "${SKYLAKESTANDALONE_SRC_ROOT}/Tuning/*.cpp"      "${SKYLAKESTANDALONE_SRC_ROOT}/Tuning/*.h" )
file( GLOB_RECURSE _Utils_files       LIST_DIRECTORIES false "${SKYLAKESTANDALONE_SRC_ROOT}/Utils/*.cpp"       "${SKYLAKESTANDALONE_SRC_ROOT}/Utils/*.h" )
file( GLOB         _Root_files        LIST_DIRECTORIES false "${SKYLAKESTANDALONE_SRC_ROOT}/*.cpp"             "${SKYLAKESTANDALONE_SRC_ROOT}/*.h" )

set( SKL_STANDALONE_LIB_Diagnostics_files "${_Diagnostics_files}" CACHE STRING "SkylakeStandloneLib Files" )
set( SKL_STANDALONE_LIB_ECS_files         "${_ECS_files}"         CACHE STRING "SkylakeStandloneLib Files" )
set( SKL_STANDALONE_LIB_Std_files         "${_Std_files}"         CACHE STRING "SkylakeStandloneLib Files" )
set( SKL_STANDALONE_LIB_Tuning_files      "${_Tuning_files}"      CACHE STRING "SkylakeStandloneLib Files" )
set( SKL_STANDALONE_LIB_Utils_files       "${_Utils_files}"       CACHE STRING "SkylakeStandloneLib Files" )
set( SKL_STANDALONE_LIB_Root_files        "${_Root_files}"        CACHE STRING "SkylakeStandloneLib Files" )

source_group("Diagnostics" FILES ${_Diagnostics_files})
source_group("ECS"         FILES ${_ECS_files}        )
source_group("Std"         FILES ${_Std_files}        )
source_group("Tuning"      FILES ${_Tuning_files}     )
source_group("Utils"       FILES ${_Utils_files}      )
source_group("/"           FILES ${_Root_files}       )

function(Create_SkylakeStandaloneLibTarget target_name)
    add_library (${target_name} STATIC  
        ${SKL_STANDALONE_LIB_Diagnostics_files}
        ${SKL_STANDALONE_LIB_ECS_files}
        ${SKL_STANDALONE_LIB_Std_files}                 
        ${SKL_STANDALONE_LIB_Tuning_files}               
        ${SKL_STANDALONE_LIB_Utils_files}
        ${SKL_STANDALONE_LIB_Root_files}
    )

    # Link Header Only part
    target_link_libraries(${target_name} PUBLIC SkylakeLibHeaderOnly)

    # Set C++20
    set_property(TARGET ${target_name} PROPERTY CXX_STANDARD 20)

    target_include_directories(${target_name} PUBLIC "${SKYLAKESTANDALONE_SRC_ROOT}/")
    target_precompile_headers(${target_name} PUBLIC "${SKYLAKESTANDALONE_SRC_ROOT}/SkylakeLibStandalone.h")

    # Default Options
    set(bBuildTests ${SKL_BUILD_TESTS})
    set(bNoNamespace ${SKL_NO_NAMESPACE})
    set(bBuildShipping ${SKL_BUILD_SHIPPING})
    set(bNoAssert ${SKL_NO_ASSERTS})
    set(bUseSerializedLogger ${SKL_USE_SERIALIZED_LOGGER})
    set(logLevel ${SKL_LIB_LOG_LEVEL})
    set(realType ${SKL_REAL_TYPE})
    set(bUseMgicEnum ${SKL_ADD_MAGIC_ENUM})
    
    # Allow optional arguments to override options

    # Use Options
    if(bBuildTests)
        target_compile_definitions(${target_name} PUBLIC SKL_TEST_ENABLED=1)
    else()
        target_compile_definitions(${target_name} PUBLIC SKL_TEST_ENABLED=0)
    endif()

    if(bNoNamespace)
        target_compile_definitions(${target_name} PUBLIC SKL_STANDALONE_NO_NAMESPACE)
    endif()

    if(realType STREQUAL "Single")
        target_compile_definitions(${target_name} PUBLIC SKL_REAL_TYPE_SINGLE)
    elseif(realType STREQUAL "Double")
        target_compile_definitions(${target_name} PUBLIC SKL_REAL_TYPE_DOUBLE)
        target_compile_definitions(${target_name} PUBLIC SKL_USE_LARGE_WORLD_COORDS)
    else()
        message(FATAL_ERROR "Invalid SKL_REAL_TYPE=${SKL_REAL_TYPE}")
    endif()

    # Build Shipping library
    if(bBuildShipping)
        target_compile_definitions(${target_name} PUBLIC SKL_BUILD_SHIPPING)
    endif()

    # Disable all soft asserts
    if(bNoAssert)
        target_compile_definitions(${target_name} PUBLIC SKL_NO_ASSERTS)
    endif()
    
    # Logger
    if(bUseSerializedLogger)
	    target_compile_definitions(${target_name} INTERFACE SKL_USE_SERIALIZED_LOGGER)
    else()
	    # N/A
    endif()

    # Log Level
    if(logLevel STREQUAL DEBUG)
        target_compile_definitions(${target_name} INTERFACE SKLL_LOG_LEVEL=1)
    elseif(logLevel STREQUAL INFO)
        target_compile_definitions(${target_name} INTERFACE SKLL_LOG_LEVEL=2)
    elseif(logLevel STREQUAL WARNING)
        target_compile_definitions(${target_name} INTERFACE SKLL_LOG_LEVEL=3)
    elseif(logLevel STREQUAL ERROR)
        target_compile_definitions(${target_name} INTERFACE SKLL_LOG_LEVEL=4)
    elseif(logLevel STREQUAL FATAL)
        target_compile_definitions(${target_name} INTERFACE SKLL_LOG_LEVEL=5)
    elseif(logLevel STREQUAL MUTE)
        target_compile_definitions(${target_name} INTERFACE SKLL_LOG_LEVEL=6)
    endif()
    
    # Magic Enum
    if(bUseMgicEnum)
		target_compile_definitions(${target_name} PUBLIC SKL_MAGIC_ENUM)
    endif()

endfunction(Create_SkylakeStandaloneLibTarget)
