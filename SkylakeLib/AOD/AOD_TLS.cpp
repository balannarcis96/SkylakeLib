//!
//! \file AOD_TLS.cpp
//! 
//! \brief Async Object bound Dispatcher thread local state 
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 

#include "SkylakeLib.h"

namespace SKL
{
    RStatus AODTLSData::Initialize( ServerInstance* InServerInstance ) noexcept 
    {
        SKL_ASSERT( nullptr != InServerInstance );
    
        SourceServerInstance = InServerInstance;

        Reset();

        // Build name
        snprintf( NameBuffer, 512, "[%ws AODTLSData]", SourceServerInstance->GetName() );

        return RSuccess;
    }

    void AODTLSData::Reset() noexcept
    {   
        DeferredTasksHandlingGroups.clear();
        ServerFlags.Flags = 0;

        if( nullptr == SourceServerInstance )   
        {
            SKL_WRN( "AODTLSData::Reset() no server instance specified!" );
            return;
        }

        //Cache for fast, thread local, access
        ServerFlags.Flags           = SourceServerInstance->ServerBuiltFlags.Flags;
        DeferredTasksHandlingGroups = SourceServerInstance->DeferredTasksHandlingGroups;
    }
}