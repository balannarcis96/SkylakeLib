cmake_minimum_required (VERSION 3.8)
project ("SkyalakeLib_TLS_Tests" VERSION 1.0.0)

add_executable(SkylakeLibTLSTests main.cpp)
add_test(NAME "TEST_SkylakeLibTLSTests" COMMAND SkylakeLibTLSTests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
target_link_libraries(SkylakeLibTLSTests PUBLIC SkylakeLib)

# Link Google Test
target_link_libraries(SkylakeLibTLSTests PUBLIC gtest_main)
#target_link_libraries(SkylakeLibTLSTests PUBLIC GTest::gmock)

# Set C++20
set_property(TARGET SkylakeLibTLSTests PROPERTY CXX_STANDARD 20)

set_target_properties(SkylakeLibTLSTests PROPERTIES FOLDER Tests)