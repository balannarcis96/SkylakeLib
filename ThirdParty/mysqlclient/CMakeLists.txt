cmake_minimum_required (VERSION 3.8)
project(mysql VERSION 8.0.32)

if(SKL_BUILD_OS STREQUAL Win64)
    set(SKYLAKEDB_ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}" CACHE STRING "SkylakeLib Root Path")
else()
    message(FATAL_ERROR "@TODO mysql library!")
endif()

set(SKL_MYSQL_STATIC_LIB_DIRECTORY "${SKYLAKEDB_ROOT_PATH}/static/" CACHE STRING "Mysql Static Library Directory")
set(SKL_MYSQL_SHARED_LIB_DIRECTORY "${SKYLAKEDB_ROOT_PATH}/runtime/" CACHE STRING "Mysql Shared Library Directory")
set(SKL_MYSQL_PREREQUISITES_LIB_DIRECTORY "${SKYLAKEDB_ROOT_PATH}/runtime/" CACHE STRING "Mysql Prerequisites Shared Libraries Directory")

file(
	GLOB_RECURSE _public_files
    LIST_DIRECTORIES false
    "${SKYLAKEDB_ROOT_PATH}/include/mysql/*.h"
    "${SKYLAKEDB_ROOT_PATH}/include/openssl/*.h"
    "${SKYLAKEDB_ROOT_PATH}/include/*.h"
)

add_library (mysql INTERFACE)
			
source_group("public" FILES ${_public_files})

# Set C++20
set_property(TARGET mysql PROPERTY CXX_STANDARD 20)

target_include_directories(mysql INTERFACE "${SKYLAKEDB_ROOT_PATH}/include/")
