cmake_minimum_required (VERSION 3.8)
project ("SkyalakeLib_Tests" VERSION 1.0.0)

#Use solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(Shared)

add_subdirectory(AsyncIOTests)
add_subdirectory(TLSTests)
add_subdirectory(TaskTests)
add_subdirectory(ObjectPoolTests)
add_subdirectory(MemoryManagementTests)
add_subdirectory(WorkersTests)
add_subdirectory(TCPAcceptorTests)
add_subdirectory(AODTests)
add_subdirectory(ServiceTests)
add_subdirectory(TLSSyncTests)
add_subdirectory(ECSTEsts)

if(SKL_BUILD_DB)
    add_subdirectory(DBTests)
endif()
