cmake_minimum_required (VERSION 3.8)
project ("SkyalakeLib_Utils_Tests" VERSION 1.0.0)

add_executable(SkylakeLibUtilsTests main.cpp)
add_test(NAME "TEST_SkylakeLibUtilsTests" COMMAND SkylakeLibUtilsTests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
target_link_libraries(SkylakeLibUtilsTests PUBLIC SkylakeLib)

# Link Google Test
target_link_libraries(SkylakeLibUtilsTests PUBLIC gtest_main)
#target_link_libraries(SkylakeLibUtilsTests PUBLIC GTest::gmock)

# Set C++20
set_property(TARGET SkylakeLibUtilsTests PROPERTY CXX_STANDARD 20)

set_target_properties(SkylakeLibUtilsTests PROPERTIES FOLDER Tests)