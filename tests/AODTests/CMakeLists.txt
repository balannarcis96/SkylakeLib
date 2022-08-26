cmake_minimum_required (VERSION 3.8)
project ("SkyalakeLib_AOD_Tests" VERSION 1.0.0)

add_executable(SkylakeLibAODTests main.cpp)
add_test(NAME "TEST_SkylakeLibAODTests" COMMAND SkylakeLibAODTests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

target_link_libraries(SkylakeLibAODTests PUBLIC SkylakeLib)

# Link Google Test
target_link_libraries(SkylakeLibAODTests PUBLIC gtest_main)
#target_link_libraries(SkylakeLibAODTests PUBLIC GTest::gmock)

target_link_libraries(SkylakeLibAODTests PUBLIC SkylakeLibTestsSharedLib)

# Set C++20
set_property(TARGET SkylakeLibAODTests PROPERTY CXX_STANDARD 20)

set_target_properties(SkylakeLibAODTests PROPERTIES FOLDER Tests)

