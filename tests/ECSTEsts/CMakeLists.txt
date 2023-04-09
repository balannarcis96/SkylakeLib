cmake_minimum_required (VERSION 3.8)
project ("SkyalakeLib_ECS_Tests" VERSION 1.0.0)

add_executable(SkylakeLibECSTests main.cpp)
add_test(NAME "TEST_SkylakeLibECSTests" COMMAND SkylakeLibECSTests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

Create_SkylakeLibTarget_Name(skl_lib_target ${SKL_DEV_NAME})
target_link_libraries(SkylakeLibECSTests PUBLIC ${skl_lib_target})

# Link Google Test
target_link_libraries(SkylakeLibECSTests PUBLIC gtest_main)
#target_link_libraries(SkylakeLibECSTests PUBLIC GTest::gmock)

# Set C++20
set_property(TARGET SkylakeLibECSTests PROPERTY CXX_STANDARD 20)

set_target_properties(SkylakeLibECSTests PROPERTIES FOLDER Tests)