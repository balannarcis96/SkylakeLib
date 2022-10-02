cmake_minimum_required (VERSION 3.8)
project(mysql VERSION 1.0.0)

if(SKL_BUILD_OS STREQUAL Win64)
    set(SKYLAKEDB_ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/mysql-8.0.30-winx64" CACHE STRING "SkylakeLib Root Path")
else()
    message(FATAL_ERROR "@TODO mysql library!")
endif()

set(SKL_MYSQL_STATIC_LIB_DIRECTORY "${SKYLAKEDB_ROOT_PATH}/lib/static/" CACHE STRING "Mysql Static Library Directory")
set(SKL_MYSQL_SHARED_LIB_DIRECTORY "${SKYLAKEDB_ROOT_PATH}/lib/shared/" CACHE STRING "Mysql Shared Library Directory")
set(SKL_MYSQL_PREREQUISITES_LIB_DIRECTORY "${SKYLAKEDB_ROOT_PATH}/lib/prerequisites/" CACHE STRING "Mysql Prerequisites Shared Libraries Directory")

file(
	GLOB_RECURSE _public_files
    LIST_DIRECTORIES false
    "${_src_root_path}/include/mysql/*.h"
    "${_src_root_path}/include/openssl/*.h"
    "${_src_root_path}/include/*.h"
)

add_library (mysql INTERFACE)
			
source_group("public" FILES ${_public_files})

# Set C++20
set_property(TARGET mysql PROPERTY CXX_STANDARD 20)

target_include_directories(mysql INTERFACE "${SKYLAKEDB_ROOT_PATH}/include/")
