cmake_minimum_required (VERSION 3.8)
project ("SkyalakeLib_TLS_Tests" VERSION 1.0.0)

add_executable(SkylakeLibTCPAcceptorTests main.cpp)
target_link_libraries(SkylakeLibTCPAcceptorTests PUBLIC SkylakeLib)

# Link Google Test
target_link_libraries(SkylakeLibTCPAcceptorTests PUBLIC gtest_main)
#target_link_libraries(SkylakeLibTCPAcceptorTests PUBLIC GTest::gmock)

# Set C++20
set_property(TARGET SkylakeLibTCPAcceptorTests PROPERTY CXX_STANDARD 20)

set_target_properties(SkylakeLibTCPAcceptorTests PROPERTIES FOLDER Tests)