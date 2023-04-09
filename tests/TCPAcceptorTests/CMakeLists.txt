cmake_minimum_required (VERSION 3.8)
project ("SkyalakeLib_TLS_Tests" VERSION 1.0.0)

add_executable(SkylakeLibTCPAcceptorTests main.cpp)
add_test(NAME "TEST_SkylakeLibTCPAcceptorTests" COMMAND SkylakeLibTCPAcceptorTests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

Create_SkylakeLibTarget_Name(skl_lib_target ${SKL_DEV_NAME})
target_link_libraries(SkylakeLibTCPAcceptorTests PUBLIC ${skl_lib_target})

# Link Google Test
target_link_libraries(SkylakeLibTCPAcceptorTests PUBLIC gtest_main)
#target_link_libraries(SkylakeLibTCPAcceptorTests PUBLIC GTest::gmock)

# Set C++20
set_property(TARGET SkylakeLibTCPAcceptorTests PROPERTY CXX_STANDARD 20)

set_target_properties(SkylakeLibTCPAcceptorTests PROPERTIES FOLDER Tests)