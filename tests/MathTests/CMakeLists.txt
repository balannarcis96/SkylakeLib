cmake_minimum_required (VERSION 3.8)
project ("SkyalakeLib_Math_Tests" VERSION 1.0.0)

add_executable(SkylakeLibMathTests main.cpp)
add_test(NAME "TEST_SkylakeLibMathTests" COMMAND SkylakeLibMathTests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
target_link_libraries(SkylakeLibMathTests PUBLIC SkylakeLib)

# Link Google Test
target_link_libraries(SkylakeLibMathTests PUBLIC gtest_main)
#target_link_libraries(SkylakeLibMathTests PUBLIC GTest::gmock)

# Set C++20
set_property(TARGET SkylakeLibMathTests PROPERTY CXX_STANDARD 20)

set_target_properties(SkylakeLibMathTests PROPERTIES FOLDER Tests)