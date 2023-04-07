cmake_minimum_required (VERSION 3.8)

if(SKL_BUILD_DATACENTER)
    add_subdirectory("rapidXML")
endif()

if(SKL_BUILD_DB)
    add_subdirectory("mysqlclient")
endif()

add_subdirectory("fmt")