cmake_minimum_required (VERSION 3.8)
project ("SkyalakeLib_Workers_Tests" VERSION 1.0.0)

add_executable(SkylakeLibWorkersTests main.cpp)
add_test(NAME "TEST_SkylakeLibWorkersTests" COMMAND SkylakeLibWorkersTests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

Create_SkylakeLibTarget_Name(skl_lib_target ${SKL_DEV_NAME})
target_link_libraries(SkylakeLibWorkersTests PUBLIC ${skl_lib_target})

# Link Google Test
target_link_libraries(SkylakeLibWorkersTests PUBLIC gtest_main)
#target_link_libraries(SkylakeLibWorkersTests PUBLIC GTest::gmock)

target_link_libraries(SkylakeLibWorkersTests PUBLIC SkylakeLibTestsSharedLib)

# Set C++20
set_property(TARGET SkylakeLibWorkersTests PROPERTY CXX_STANDARD 20)

set_target_properties(SkylakeLibWorkersTests PROPERTIES FOLDER Tests)