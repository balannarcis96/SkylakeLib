cmake_minimum_required (VERSION 3.8)
project(rapidXML VERSION 1.0.0)

set(_src_root_path "${CMAKE_CURRENT_SOURCE_DIR}")

file(
	GLOB_RECURSE _public_files
    LIST_DIRECTORIES false
    "${_src_root_path}/public/*.hpp"
)

add_library (rapidXML INTERFACE)
			
source_group("public" FILES ${_public_files})

# Set C++11
set_property(TARGET rapidXML PROPERTY CXX_STANDARD 11)

target_include_directories(rapidXML INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/public/")
