//!
//! \file SkylakeLib.cpp
//! 
//! \brief Implementation for the SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#include "SkylakeLib.h"

namespace SKL
{
    std::relaxed_value<FILE*> GLogOutput { stdout }; //!< Defaulted to stdout

    RStatus InitializeLibrary( int32_t Argc, char** Argv, FILE* InLogOutput ) noexcept 
    {
        if( nullptr != InLogOutput )
        {
            // set the log output file handle
            GLogOutput.exchange( InLogOutput );
        }

        auto Result = AsyncIO::InitializeSystem();
        if( RSuccess != Result ) SKL_UNLIKELY
        {
            SKL_ERR( "Failed to initialize the async io system!" );
            return Result;
        }

        Result = InitializeLibrary_Thread();
        if( RSuccess != Result ) SKL_UNLIKELY
        {
            SKL_ERR( "Failed to initialize the SkylakeLibrary for the main thread!" );
            return Result;
        }

        return RSuccess;
    }

    RStatus TerminateLibrary() noexcept
    {
        auto Result = AsyncIO::ShutdownSystem();
        if( RSuccess != Result ) SKL_UNLIKELY
        {
            SKL_ERR( "Failed to shutdown the async io system!" );
            return Result;
        }

        Result = TerminateLibrary_Thread();
        if( RSuccess != Result ) SKL_UNLIKELY
        {
            SKL_ERR( "Failed to terminate the SkylakeLibrary for the main thread!" );
            return Result;
        }

        return RSuccess;
    }

    RStatus InitializeLibrary_Thread() noexcept 
    {
        
        return RSuccess;
    }

    RStatus TerminateLibrary_Thread() noexcept
    {

        return RSuccess;
    }
}

//Memory Manager
namespace SKL
{
    SKL_IFMEMORYSTATS( SKL_CACHE_ALIGNED std::atomic<size_t> MemoryManager::CustomSizeAllocations  { 0 } );
    SKL_IFMEMORYSTATS( SKL_CACHE_ALIGNED std::atomic<size_t> MemoryManager::CustomSizeDeallocations{ 0 } );
    SKL_IFMEMORYSTATS( SKL_CACHE_ALIGNED std::atomic<size_t> MemoryManager::TotalAllocations       { 0 } );
    SKL_IFMEMORYSTATS( SKL_CACHE_ALIGNED std::atomic<size_t> MemoryManager::TotalDeallocations     { 0 } );
}