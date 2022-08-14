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
    FILE* GLogOutput { stdout }; //!< Defaulted to stdout

    RStatus InitializeLibrary( int32_t Argc, char** Argv, FILE* InLogOutput ) noexcept 
    {
        if( nullptr != InLogOutput )
        {
            // set the log output file handle
            GLogOutput = InLogOutput;
        }

        auto Result = AsyncIO::InitializeSystem();
        if( RSuccess != Result ) SKL_UNLIKELY
        {
            SKL_ERR( "Failed to initialize the async io system!" );
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
