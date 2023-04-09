cmake_minimum_required (VERSION 3.8)
project ("SkyalakeLib_Datacenter_Tests" VERSION 1.0.0)

add_executable(SkylakeLibDatacenterTests main.cpp)
add_test(NAME "TEST_SkylakeLibDatacenterTests" COMMAND SkylakeLibDatacenterTests WORKING_DIRECTORY "$<TARGET_FILE_DIR:SkylakeLibDatacenterTests>/../")

Create_SkylakeLibTarget_Name(skl_lib_target ${SKL_DEV_NAME})
target_link_libraries(SkylakeLibDatacenterTests PUBLIC ${skl_lib_target})

Create_SkylakeDCBuilderLibTarget_Name(skldcbuilder_lib_target ${SKL_DEV_NAME})
target_link_libraries(SkylakeLibDatacenterTests PUBLIC ${skldcbuilder_lib_target})

# Link Google Test
target_link_libraries(SkylakeLibDatacenterTests PUBLIC gtest_main)
#target_link_libraries(SkylakeLibDatacenterTests PUBLIC GTest::gmock)

# Set C++20
set_property(TARGET SkylakeLibDatacenterTests PROPERTY CXX_STANDARD 20)

set_target_properties(SkylakeLibDatacenterTests PROPERTIES FOLDER Tests)

add_custom_command(TARGET SkylakeLibDatacenterTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/data"
        "$<TARGET_FILE_DIR:SkylakeLibDatacenterTests>/../")

if (WIN32)
    set_target_properties(SkylakeLibDatacenterTests PROPERTIES COMPILE_FLAGS "/W3 /WX-")
else()
    message(FATAL_ERROR "@TODO compiler flags")
endif()