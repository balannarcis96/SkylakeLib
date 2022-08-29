cmake_minimum_required (VERSION 3.8)
project(mysql VERSION 1.0.0)

set(_src_root_path "${CMAKE_CURRENT_SOURCE_DIR}")

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

target_include_directories(mysql INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/")
