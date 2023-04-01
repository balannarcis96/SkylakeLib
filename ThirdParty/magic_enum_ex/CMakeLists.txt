cmake_minimum_required (VERSION 3.8)
project(magic_enum_ex VERSION 1.0.0)

add_library(magic_enum_ex INTERFACE "include/magic_enum_ex.hpp")
target_include_directories(magic_enum_ex INTERFACE "include/")

set_property(TARGET magic_enum_ex PROPERTY CXX_STANDARD 20)
