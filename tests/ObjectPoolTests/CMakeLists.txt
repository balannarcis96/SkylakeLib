cmake_minimum_required (VERSION 3.8)
project ("SkyalakeLib_ObjectPool_Tests" VERSION 1.0.0)

add_executable(SkylakeLibObjectPoolTests main.cpp)
add_test(NAME "TEST_SkylakeLibObjectPoolTests" COMMAND SkylakeLibObjectPoolTests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

Create_SkylakeLibTarget_Name(skl_lib_target ${SKL_DEV_NAME})
target_link_libraries(SkylakeLibObjectPoolTests PUBLIC ${skl_lib_target})

# Link Google Test
target_link_libraries(SkylakeLibObjectPoolTests PUBLIC gtest_main)
#target_link_libraries(SkylakeLibObjectPoolTests PUBLIC GTest::gmock)

# Set C++20
set_property(TARGET SkylakeLibObjectPoolTests PROPERTY CXX_STANDARD 20)

set_target_properties(SkylakeLibObjectPoolTests PROPERTIES FOLDER Tests)