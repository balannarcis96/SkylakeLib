cmake_minimum_required (VERSION 3.8)
project ("SkyalakeLib_DB_Tests" VERSION 1.0.0)

add_executable(SkylakeLibDBTests main.cpp)
add_test(NAME "TEST_SkylakeLibDBTests" COMMAND SkylakeLibDBTests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
target_link_libraries(SkylakeLibDBTests PUBLIC SkylakeLibDB)

# Link Google Test
target_link_libraries(SkylakeLibDBTests PUBLIC gtest_main)
#target_link_libraries(SkylakeLibDBTests PUBLIC GTest::gmock)

# Set C++20
set_property(TARGET SkylakeLibDBTests PROPERTY CXX_STANDARD 20)

set_target_properties(SkylakeLibDBTests PROPERTIES FOLDER Tests)

file(GLOB ALL_PREQREQUISITES "${SKL_MYSQL_PREREQUISITES_LIB_DIRECTORY}/*.dll")
foreach(Item IN LISTS ALL_PREQREQUISITES)
    add_custom_command(
                TARGET SkylakeLibDBTests PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Item} "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/"
                COMMENT "Copying DLL: ${Item}")
endforeach()
