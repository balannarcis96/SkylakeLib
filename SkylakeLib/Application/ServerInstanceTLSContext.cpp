//!
//! \file ServerInstanceTLSContext.cpp
//! 
//! \brief Thread local context for all workers in a server instance
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 

#include "SkylakeLib.h"

namespace SKL
{
    RStatus ServerInstanceTLSData::Initialize( ServerInstance* InServerInstance ) noexcept 
    {
        SKL_ASSERT( nullptr != InServerInstance );
    
        SourceServerInstance = InServerInstance;

        Reset();

        // Build name
        snprintf( NameBuffer, 512, "[%ws ServerInstanceTLSData]", SourceServerInstance->GetName() );

        return RSuccess;
    }

    void ServerInstanceTLSData::Reset() noexcept
    {   
        DeferredTasksHandlingGroups.clear();
        ServerFlags.Flags = 0;

        if( nullptr == SourceServerInstance )   
        {
            SKL_WRN( "ServerInstanceTLSData::Reset() no server instance specified!" );
            return;
        }

        //Cache for fast, thread local, access
        ServerFlags.Flags           = SourceServerInstance->ServerBuiltFlags.Flags;
        DeferredTasksHandlingGroups = SourceServerInstance->DeferredTasksHandlingGroups;
    }
}