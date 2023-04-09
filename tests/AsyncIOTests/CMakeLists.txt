cmake_minimum_required (VERSION 3.8)
project ("SkyalakeLib_AsyncIO_Tests" VERSION 1.0.0)

add_executable(SkylakeLibAsyncIOTests main.cpp)
add_test(NAME "TEST_SkylakeLibAsyncIOTests" COMMAND SkylakeLibAsyncIOTests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

Create_SkylakeLibTarget_Name(skl_lib_target ${SKL_DEV_NAME})
target_link_libraries(SkylakeLibAsyncIOTests PUBLIC ${skl_lib_target})

# Link Google Test
target_link_libraries(SkylakeLibAsyncIOTests PUBLIC gtest_main)
#target_link_libraries(SkylakeLibAsyncIOTests PUBLIC GTest::gmock)

# Set C++20
set_property(TARGET SkylakeLibAsyncIOTests PROPERTY CXX_STANDARD 20)

set_target_properties(SkylakeLibAsyncIOTests PROPERTIES FOLDER Tests)