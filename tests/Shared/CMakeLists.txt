cmake_minimum_required (VERSION 3.8)
project ("SkyalakeLib_TestsShared" VERSION 1.0.0)

add_library(SkylakeLibTestsSharedLib STATIC 
    "include/ApplicationSetup.h"
    "private/ApplicationSetup.cpp"
)

Create_SkylakeLibTarget_Name(skl_lib_target ${SKL_DEV_NAME})
target_link_libraries(SkylakeLibTestsSharedLib PUBLIC ${skl_lib_target})

# Link Google Test
target_link_libraries(SkylakeLibTestsSharedLib PUBLIC gtest_main)
#target_link_libraries(SkylakeLibTestsSharedLib PUBLIC GTest::gmock)

# Set C++20
set_property(TARGET SkylakeLibTestsSharedLib PROPERTY CXX_STANDARD 20)

set_target_properties(SkylakeLibTestsSharedLib PROPERTIES FOLDER Tests)
target_include_directories(SkylakeLibTestsSharedLib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")