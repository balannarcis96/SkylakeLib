cmake_minimum_required (VERSION 3.8)
project ("SkyalakeLib_TLSSync_Tests" VERSION 1.0.0)

add_executable(SkylakeLibTLSSyncTests main.cpp)
add_test(NAME "TEST_SkylakeLibTLSSyncTests" COMMAND SkylakeLibTLSSyncTests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
target_link_libraries(SkylakeLibTLSSyncTests PUBLIC SkylakeLib)

# Link Google Test
target_link_libraries(SkylakeLibTLSSyncTests PUBLIC gtest_main)
#target_link_libraries(SkylakeLibTLSSyncTests PUBLIC GTest::gmock)

target_link_libraries(SkylakeLibTLSSyncTests PUBLIC SkylakeLibTestsSharedLib)

# Set C++20
set_property(TARGET SkylakeLibTLSSyncTests PROPERTY CXX_STANDARD 20)

set_target_properties(SkylakeLibTLSSyncTests PROPERTIES FOLDER Tests)
