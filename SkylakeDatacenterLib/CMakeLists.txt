cmake_minimum_required (VERSION 3.8)    
project("SkylakeDatacenterLib" VERSION 1.0.0)

set(_src_root_path "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB _Public_files LIST_DIRECTORIES false "${_src_root_path}/Public/*.h")
file(GLOB _Internal_files LIST_DIRECTORIES false "${_src_root_path}/Internal/*.h")
file(GLOB _Private_files LIST_DIRECTORIES false "${_src_root_path}/Private/*.cpp")
    
add_library(SkylakeDatacenterLib STATIC 
            ${_Internal_files}
            ${_Public_files}
            ${_Private_files})
    
source_group("Public" FILES ${_Public_files})
source_group("Internal" FILES ${_Internal_files})
source_group("Private" FILES ${_Private_files})

# Set C++20
set_property(TARGET SkylakeDatacenterLib PROPERTY CXX_STANDARD 20)
target_link_libraries(SkylakeDatacenterLib PUBLIC SkylakeLib)
target_include_directories(SkylakeDatacenterLib PUBLIC "${_src_root_path}/Public/")

# Builder Lib
file(GLOB _Builder_Public_files LIST_DIRECTORIES false "${_src_root_path}/Builder/Public/*.h")
file(GLOB _Builder_Private_files LIST_DIRECTORIES false "${_src_root_path}/Builder/Private/*.cpp")
add_library(SkylakeDatacenterBuilder STATIC ${_Builder_Public_files} ${_Builder_Private_files})
target_include_directories(SkylakeDatacenterBuilder PUBLIC "${_src_root_path}/Builder/Public/")

source_group("Public" FILES ${_Builder_Public_files})
source_group("Private" FILES ${_Builder_Private_files})

# Set C++20
set_property(TARGET SkylakeDatacenterBuilder PROPERTY CXX_STANDARD 20)
target_link_libraries(SkylakeDatacenterBuilder PUBLIC SkylakeDatacenterLib)
target_link_libraries(SkylakeDatacenterBuilder PUBLIC rapidXML)
