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
add_subdirectory(AODQueueTests)
add_subdirectory(ServiceTests)
add_subdirectory(TLSSyncTests)
add_subdirectory(ECSTEsts)
add_subdirectory(UtilsTests)
add_subdirectory(ProtocolTests)

if(SKL_BUILD_DB)
    add_subdirectory(DBTests)
endif()

if(SKL_MATH)
    add_subdirectory(MathTests)
endif()


if(SKL_BUILD_DATACENTER)
    add_subdirectory(DatacenterTests)
endif()
