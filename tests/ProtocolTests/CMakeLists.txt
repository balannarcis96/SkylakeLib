cmake_minimum_required (VERSION 3.8)
project ("SkyalakeLib_PROTOCOL_Tests" VERSION 1.0.0)

add_executable(SkylakeLibPROTOCOLTests main.cpp)
add_test(NAME "TEST_SkylakeLibPROTOCOLTests" COMMAND SkylakeLibPROTOCOLTests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

target_link_libraries(SkylakeLibPROTOCOLTests PUBLIC SkylakeLibStandalone)

# Link Google Test
target_link_libraries(SkylakeLibPROTOCOLTests PUBLIC gtest_main)
#target_link_libraries(SkylakeLibPROTOCOLTests PUBLIC GTest::gmock)

# Set C++20
set_property(TARGET SkylakeLibPROTOCOLTests PROPERTY CXX_STANDARD 20)

set_target_properties(SkylakeLibPROTOCOLTests PROPERTIES FOLDER Tests)