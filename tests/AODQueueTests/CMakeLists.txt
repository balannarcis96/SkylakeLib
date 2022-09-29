cmake_minimum_required (VERSION 3.8)
project ("SkyalakeLib_AODQueue_Tests" VERSION 1.0.0)

add_executable(SkylakeLibAODQueueTests main.cpp)
add_test(NAME "TEST_SkylakeLibAODQueueTests" COMMAND SkylakeLibAODQueueTests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
target_link_libraries(SkylakeLibAODQueueTests PUBLIC SkylakeLib)

# Link Google Test
target_link_libraries(SkylakeLibAODQueueTests PUBLIC gtest_main)
#target_link_libraries(SkylakeLibAODQueueTests PUBLIC GTest::gmock)

# Set C++20
set_property(TARGET SkylakeLibAODQueueTests PROPERTY CXX_STANDARD 20)

set_target_properties(SkylakeLibAODQueueTests PROPERTIES FOLDER Tests)