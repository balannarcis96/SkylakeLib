cmake_minimum_required (VERSION 3.8)    
project("SkylakeDatacenterLib" VERSION 1.0.0)

set(SKYLAKEDC_SRC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}" CACHE STRING "SkylakeDBLib Root Dir")

file(GLOB _Public_files LIST_DIRECTORIES false "${SKYLAKEDC_SRC_ROOT}/Public/*.h")
file(GLOB _Internal_files LIST_DIRECTORIES false "${SKYLAKEDC_SRC_ROOT}/Internal/*.h")
file(GLOB _Private_files LIST_DIRECTORIES false "${SKYLAKEDC_SRC_ROOT}/Private/*.cpp")
file(GLOB _Builder_Public_files LIST_DIRECTORIES false "${SKYLAKEDC_SRC_ROOT}/Builder/Public/*.h")
file(GLOB _Builder_Private_files LIST_DIRECTORIES false "${SKYLAKEDC_SRC_ROOT}/Builder/Private/*.cpp")

set( SKL_DC_LIB_Public_files "${_Public_files}"  CACHE STRING "SkylakeDCLib Files" )
set( SKL_DC_LIB_Internal_files "${_Internal_files}"  CACHE STRING "SkylakeDCLib Files" )
set( SKL_DC_LIB_Private_files "${_Private_files}" CACHE STRING "SkylakeDCLib Files" )
set( SKL_DC_LIB_Builder_Public_files"${_Builder_Public_files}" CACHE STRING "SkylakeDCLib Files" )
set( SKL_DC_LIB_Builder_Private_files "${_Builder_Private_files}" CACHE STRING "SkylakeDCLib Files" )

source_group("Public" FILES ${_Public_files})
source_group("Internal" FILES ${_Internal_files})
source_group("Private" FILES ${_Private_files})
source_group("Public" FILES ${_Builder_Public_files})
source_group("Private" FILES ${_Builder_Private_files})

function(Create_SkylakeDCLibTarget target_postfix)
    Create_SkylakeDCLibTarget_Name(target_name ${target_postfix})
    Create_SkylakeDCBuilderLibTarget_Name(builder_target_name ${target_postfix})
    Create_SkylakeStandaloneLibTarget_Name(skl_standalone_lib_target_name ${target_postfix})
    
    # Add DC Lib target
    add_library(${target_name} STATIC 
                ${SKL_DC_LIB_Internal_files}
                ${SKL_DC_LIB_Public_files}
                ${SKL_DC_LIB_Private_files})
    
    # Set C++20
    set_property(TARGET ${target_name} PROPERTY CXX_STANDARD 20)

    # Link the standalone skylake lib
    target_link_libraries(${target_name} PUBLIC ${skl_standalone_lib_target_name})

    # Include directory
    target_include_directories(${target_name} PUBLIC "${SKYLAKEDC_SRC_ROOT}/Public/")

    # Add Builder Lib target
    add_library(${builder_target_name} STATIC ${SKL_DC_LIB_Builder_Public_files} ${SKL_DC_LIB_Builder_Private_files})

    # Include directory
    target_include_directories(${builder_target_name} PUBLIC "${SKYLAKEDC_SRC_ROOT}/Builder/Public/")

    # Set C++20
    set_property(TARGET ${builder_target_name} PROPERTY CXX_STANDARD 20)

    # Link the DC lib
    target_link_libraries(${builder_target_name} PUBLIC ${target_name})

    # Link the rapidXML lib
    target_link_libraries(${builder_target_name} PUBLIC rapidXML)

    # Precompiled headers
	if(WIN32)
        target_precompile_headers(${target_name} PUBLIC "${SKYLAKEDC_SRC_ROOT}/Public/SkylakeDatacenter.h")
        target_precompile_headers(${builder_target_name} PUBLIC "${SKYLAKEDC_SRC_ROOT}/Builder/Public/SkylakeDatacenterAdapter.h")
        target_precompile_headers(${builder_target_name} PUBLIC "${SKYLAKEDC_SRC_ROOT}/Builder/Public/SkylakeDatacenterBuilder.h")
        target_precompile_headers(${builder_target_name} PUBLIC "${SKYLAKEDC_SRC_ROOT}/Builder/Public/SkylakeDatacenterXMLAdapter.h")
    else()
        
    endif()
endfunction(Create_SkylakeDCLibTarget)
