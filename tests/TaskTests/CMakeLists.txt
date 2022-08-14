cmake_minimum_required (VERSION 3.8)
project ("SkyalakeLib_Task_Tests" VERSION 1.0.0)

add_executable(SkylakeLibTaskTests main.cpp)
target_link_libraries(SkylakeLibTaskTests PUBLIC SkylakeLib)

# Link Google Test
target_link_libraries(SkylakeLibTaskTests PUBLIC gtest_main)
#target_link_libraries(SkylakeLibTaskTests PUBLIC GTest::gmock)

# Set C++20
set_property(TARGET SkylakeLibTaskTests PROPERTY CXX_STANDARD 20)

set_target_properties(SkylakeLibTaskTests PROPERTIES FOLDER Tests)