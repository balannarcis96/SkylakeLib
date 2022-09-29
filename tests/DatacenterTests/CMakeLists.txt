cmake_minimum_required (VERSION 3.8)
project ("SkyalakeLib_Datacenter_Tests" VERSION 1.0.0)

add_executable(SkylakeLibDatacenterTests main.cpp)
add_test(NAME "TEST_SkylakeLibDatacenterTests" COMMAND SkylakeLibDatacenterTests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
target_link_libraries(SkylakeLibDatacenterTests PUBLIC SkylakeDatacenterLib)

# Link Google Test
target_link_libraries(SkylakeLibDatacenterTests PUBLIC gtest_main)
#target_link_libraries(SkylakeLibDatacenterTests PUBLIC GTest::gmock)

# Set C++20
set_property(TARGET SkylakeLibDatacenterTests PROPERTY CXX_STANDARD 20)

set_target_properties(SkylakeLibDatacenterTests PROPERTIES FOLDER Tests)