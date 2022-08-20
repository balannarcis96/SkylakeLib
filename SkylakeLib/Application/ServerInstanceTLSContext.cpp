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
    RStatus ServerInstanceTLSContext::Initialize( ServerInstance* InServerInstance, WorkerGroupTag InWorkerGroupTag ) noexcept 
    {
        SKL_ASSERT( nullptr != InServerInstance );
        SKL_ASSERT( true == InWorkerGroupTag.IsValid() );
    
        SourceServerInstance = InServerInstance;
        ParentWorkerGroup = InWorkerGroupTag;

        while( false == DelayedTasks.empty() )
        {
            TSharedPtr<ITask>::Static_Reset( DelayedTasks.top() );
            DelayedTasks.pop();
        }

        Reset();

        // Build name
        snprintf( NameBuffer, 512, "[%ws ServerInstanceTLSContext]", SourceServerInstance->GetName() );

        return RSuccess;
    }

    void ServerInstanceTLSContext::Reset() noexcept
    {   
        DeferredTasksHandlingGroups.clear();
        ServerFlags.Flags = 0;

        if( nullptr == SourceServerInstance )   
        {
            SKL_WRN( "ServerInstanceTLSContext::Reset() no server instance specified!" );
            return;
        }

        //Cache for fast, thread local, access
        ServerFlags.Flags           = SourceServerInstance->ServerBuiltFlags.Flags;
        DeferredTasksHandlingGroups = SourceServerInstance->DeferredTasksHandlingGroups;
    }
}