cmake_minimum_required (VERSION 3.8)
project ("SkyalakeLib_DB_Tests" VERSION 1.0.0)

add_executable(SkylakeLibDBTests main.cpp)
add_test(NAME "TEST_SkylakeLibDBTests" COMMAND SkylakeLibDBTests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
target_link_libraries(SkylakeLibDBTests PUBLIC mysql)
target_link_libraries(SkylakeLibDBTests PUBLIC SkylakeLibDB)

# Link Google Test
target_link_libraries(SkylakeLibDBTests PUBLIC gtest_main)
#target_link_libraries(SkylakeLibDBTests PUBLIC GTest::gmock)

# Set C++20
set_property(TARGET SkylakeLibDBTests PROPERTY CXX_STANDARD 20)

set_target_properties(SkylakeLibDBTests PROPERTIES FOLDER Tests)