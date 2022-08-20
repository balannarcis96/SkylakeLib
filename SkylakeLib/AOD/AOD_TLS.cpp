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
    AODTLSContext::~AODTLSContext() noexcept
    {
        DeferredTasksHandlingGroups.clear();
        ServerFlags.Flags = 0;
        
        while( false == DelayedTasks.empty() )
        {
            DelayedTasks.pop();
        }
    }

    RStatus AODTLSContext::Initialize( ServerInstance* InServerInstance, WorkerGroupTag InWorkerGroupTag ) noexcept 
    {
        SKL_ASSERT( nullptr != InServerInstance );
        SKL_ASSERT( true == InWorkerGroupTag.IsValid() );
    
        SourceServerInstance     = InServerInstance;
        ParentWorkerGroup        = InWorkerGroupTag;
        bScheduleAODDelayedTasks = false == ParentWorkerGroup.bHandlesTimerTasks;

        Reset();

        // Build name
        snprintf( NameBuffer, 512, "[%ws AODTLSContext]", SourceServerInstance->GetName() );

        return RSuccess;
    }

    void AODTLSContext::Reset() noexcept
    {   
        DeferredTasksHandlingGroups.clear();
        ServerFlags.Flags = 0;
        
        while( false == DelayedTasks.empty() )
        {
            TSharedPtr<IAODTask>::Static_Reset( DelayedTasks.top() );
            DelayedTasks.pop();
        }

        if( nullptr == SourceServerInstance )   
        {
            SKL_WRN( "AODTLSContext::Reset() no server instance specified!" );
            return;
        }

        //Cache for fast, thread local, access
        ServerFlags.Flags           = SourceServerInstance->ServerBuiltFlags.Flags;
        DeferredTasksHandlingGroups = SourceServerInstance->DeferredTasksHandlingGroups;
    }
}