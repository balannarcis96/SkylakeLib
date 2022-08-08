cmake_minimum_required (VERSION 3.8)
project ("SkyalakeLib_AsyncIO_Tests" VERSION 1.0.0)

add_executable(SkylakeLibAsyncIOTests main.cpp)
target_link_libraries(SkylakeLibAsyncIOTests PUBLIC SkylakeLib)

# Link Google Test
target_link_libraries(SkylakeLibAsyncIOTests PUBLIC gtest_main)
#target_link_libraries(SkylakeLibAsyncIOTests PUBLIC GTest::gmock)

# Set C++20
set_property(TARGET SkylakeLibAsyncIOTests PROPERTY CXX_STANDARD 20)

set_target_properties(SkylakeLibAsyncIOTests PROPERTIES FOLDER Tests)