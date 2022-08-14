cmake_minimum_required (VERSION 3.8)
project ("SkyalakeLib_ObjectPool_Tests" VERSION 1.0.0)

add_executable(SkylakeLibObjectPoolTests main.cpp)
target_link_libraries(SkylakeLibObjectPoolTests PUBLIC SkylakeLib)

# Link Google Test
target_link_libraries(SkylakeLibObjectPoolTests PUBLIC gtest_main)
#target_link_libraries(SkylakeLibObjectPoolTests PUBLIC GTest::gmock)

# Set C++20
set_property(TARGET SkylakeLibObjectPoolTests PROPERTY CXX_STANDARD 20)

set_target_properties(SkylakeLibObjectPoolTests PROPERTIES FOLDER Tests)