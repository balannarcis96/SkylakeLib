cmake_minimum_required (VERSION 3.8)
project ("SkyalakeLib_Workers_Tests" VERSION 1.0.0)

add_executable(SkylakeLibWorkersTests main.cpp)
add_test(NAME "TEST_SkylakeLibWorkersTests" COMMAND SkylakeLibWorkersTests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
target_link_libraries(SkylakeLibWorkersTests PUBLIC SkylakeLib)

# Link Google Test
target_link_libraries(SkylakeLibWorkersTests PUBLIC gtest_main)
#target_link_libraries(SkylakeLibWorkersTests PUBLIC GTest::gmock)

# Set C++20
set_property(TARGET SkylakeLibWorkersTests PROPERTY CXX_STANDARD 20)

set_target_properties(SkylakeLibWorkersTests PROPERTIES FOLDER Tests)