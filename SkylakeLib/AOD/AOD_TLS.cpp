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
        Clear();
    }

    RStatus AODTLSContext::Initialize( ServerInstance* InServerInstance, WorkerGroupTag InWorkerGroupTag ) noexcept 
    {
        SKL_ASSERT( nullptr != InServerInstance );
        
        if( false == InWorkerGroupTag.Validate() )
        {
            return RInvalidParamters;
        }
    
        SourceServerInstance     = InServerInstance;
        ParentWorkerGroup        = InWorkerGroupTag;
        bScheduleAODDelayedTasks = false == ParentWorkerGroup.bIsActive;

        Reset();

        // Build name
        snprintf( NameBuffer, 512, "[%ws AODTLSContext]", SourceServerInstance->GetName() );

        return RSuccess;
    }

    void AODTLSContext::Clear() noexcept
    {
        DeferredAODTasksHandlingGroups.clear();
        ServerFlags.Flags = 0;
        
        while( false == DelayedSharedObjectTasks.empty() )
        {
            TSharedPtr<IAODSharedObjectTask>::Static_Reset( DelayedSharedObjectTasks.top() );
            DelayedSharedObjectTasks.pop();
        }

        while( false == DelayedStaticObjectTasks.empty() )
        {
            TSharedPtr<IAODStaticObjectTask>::Static_Reset( DelayedStaticObjectTasks.top() );
            DelayedStaticObjectTasks.pop();
        }
    }

    void AODTLSContext::Reset() noexcept
    {   
        Clear();

        if( nullptr == SourceServerInstance )   
        {
            SKL_WRN( "AODTLSContext::Reset() no server instance specified!" );
            return;
        }

        //Cache for fast, thread local, access
        ServerFlags.Flags              = SourceServerInstance->ServerBuiltFlags.Flags;
        DeferredAODTasksHandlingGroups = SourceServerInstance->DeferredTasksHandlingGroups;
    }
}