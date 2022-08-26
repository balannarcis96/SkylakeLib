cmake_minimum_required (VERSION 3.8)
project ("SkyalakeLib_MManagement_Tests" VERSION 1.0.0)

add_executable(SkylakeLibMManagementTests main.cpp)
add_test(NAME "TEST_SkylakeLibMManagementTests" COMMAND SkylakeLibMManagementTests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
target_link_libraries(SkylakeLibMManagementTests PUBLIC SkylakeLib)

# Link Google Test
target_link_libraries(SkylakeLibMManagementTests PUBLIC gtest_main)
#target_link_libraries(SkylakeLibMManagementTests PUBLIC GTest::gmock)

# Set C++20
set_property(TARGET SkylakeLibMManagementTests PROPERTY CXX_STANDARD 20)

set_target_properties(SkylakeLibMManagementTests PROPERTIES FOLDER Tests)