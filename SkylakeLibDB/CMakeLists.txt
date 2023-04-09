cmake_minimum_required (VERSION 3.8)    
project("SkylakeLibDB" VERSION 1.0.0)

set(SKYLAKEDB_SRC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}" CACHE STRING "SkylakeDBLib Root Dir")

file(GLOB_RECURSE _Public_files LIST_DIRECTORIES false "${SKYLAKEDB_SRC_ROOT}/Public/*.h")
file(GLOB_RECURSE _Private_files LIST_DIRECTORIES false "${SKYLAKEDB_SRC_ROOT}/Private/*.cpp" "${SKYLAKEDB_SRC_ROOT}/Private/*.h")

set( SKL_DB_LIB_Public_files   "${_Public_files}"  CACHE STRING "SkylakeDBLib Files" )
set( SKL_DB_LIB_Private_files  "${_Private_files}" CACHE STRING "SkylakeDBLib Files" )

source_group("Public" FILES ${_Public_files})
source_group("Private" FILES ${_Private_files})
	
function(Create_SkylakeDBLibTarget target_postfix)
    Create_SkylakeDBLibTarget_Name(target_name ${target_postfix})
    Create_SkylakeLibTarget_Name(skl_lib_target_name ${target_postfix})

	add_library(${target_name} STATIC ${SKL_DB_LIB_Public_files} ${SKL_DB_LIB_Private_files})
				
	# Set C++20
	set_property(TARGET ${target_name} PROPERTY CXX_STANDARD 20)

	# Include dir
	target_include_directories(${target_name} PUBLIC "${SKYLAKEDB_SRC_ROOT}/Public/")

	# Link SkylakeLib
	target_link_libraries(${target_name} PUBLIC ${skl_lib_target_name})
	
	# Link mysql interface lib
	target_link_libraries(${target_name} PRIVATE mysql)

    # Default Options
    set(bEnableMysqlTrafficCompression ${SKL_MYSQL_COMPRESS_NET})
	
	# Link the mysql static lib
	if(WIN32)
		target_link_libraries(${target_name} PRIVATE "${SKL_MYSQL_STATIC_LIB_DIRECTORY}/$<CONFIG>/mysqlclient_$<$<CONFIG:DEBUG>:${SKL_WIN32_DEBUG_RT_FALG}>$<$<CONFIG:RELEASE>:${SKL_WIN32_RELEASE_RT_FALG}>$<$<CONFIG:RELWITHDEBINFO>:${SKL_WIN32_RELEASE_RT_FALG}>.lib")
	else()
		message(FATAL_ERROR "@TODO link mysqlclient lib")
	endif()

	# Precompile headers
	if(WIN32)
		target_precompile_headers(${target_name} PUBLIC "${SKYLAKEDB_SRC_ROOT}/Public/SkylakeLibDB.h")
	endif()
	
	# Mysql traffic compression
	if(bEnableMysqlTrafficCompression)
		target_compile_definitions(${target_name} PRIVATE SKL_MYSQL_COMPRESS_NET)
	endif()
endfunction(Create_SkylakeDBLibTarget)