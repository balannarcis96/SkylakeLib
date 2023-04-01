cmake_minimum_required (VERSION 3.8)    
project("SkylakeLibDB" VERSION 1.0.0)

set(SKYLAKE_SRC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE _Public_files LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Public/*.h")
file(GLOB_RECURSE _Private_files LIST_DIRECTORIES false "${SKYLAKE_SRC_ROOT}/Private/*.cpp" "${SKYLAKE_SRC_ROOT}/Private/*.h")

add_library(SkylakeLibDB STATIC ${_Private_files} ${_Public_files})
            
source_group("Public" FILES ${_Public_files})
source_group("Private" FILES ${_Private_files})

# Set C++20
set_property(TARGET SkylakeLibDB PROPERTY CXX_STANDARD 20)

target_include_directories(SkylakeLibDB PUBLIC "${SKYLAKE_SRC_ROOT}/Public/")

target_link_libraries(SkylakeLibDB PUBLIC SkylakeLib)
target_link_libraries(SkylakeLibDB PRIVATE mysql)

if(WIN32)
    target_link_libraries(SkylakeLibDB PRIVATE "${SKL_MYSQL_STATIC_LIB_DIRECTORY}/$<CONFIG>/mysqlclient_$<$<CONFIG:DEBUG>:${SKL_WIN32_DEBUG_RT_FALG}>$<$<CONFIG:RELEASE>:${SKL_WIN32_RELEASE_RT_FALG}>$<$<CONFIG:RELWITHDEBINFO>:${SKL_WIN32_RELEASE_RT_FALG}>.lib")
else()
    message(FATAL_ERROR "@TODO link mysqlclient lib")
endif()

if(WIN32)
    target_precompile_headers(SkylakeLibDB PUBLIC "${SKYLAKE_SRC_ROOT}/Public/SkylakeLibDB.h")
endif()
