cmake_minimum_required (VERSION 3.8)
project ("SkyalakeLib_Services_Tests" VERSION 1.0.0)

add_executable(SkylakeLibServicesTests main.cpp)
add_test(NAME "TEST_SkylakeLibServicesTests" COMMAND SkylakeLibServicesTests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
target_link_libraries(SkylakeLibServicesTests PUBLIC SkylakeLib)

# Link Google Test
target_link_libraries(SkylakeLibServicesTests PUBLIC gtest_main)
#target_link_libraries(SkylakeLibServicesTests PUBLIC GTest::gmock)

target_link_libraries(SkylakeLibServicesTests PUBLIC SkylakeLibTestsSharedLib)

# Set C++20
set_property(TARGET SkylakeLibServicesTests PROPERTY CXX_STANDARD 20)

set_target_properties(SkylakeLibServicesTests PROPERTIES FOLDER Tests)