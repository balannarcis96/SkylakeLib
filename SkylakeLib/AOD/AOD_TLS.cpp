//!
//! \file AOD_TLS.cpp
//! 
//! \brief Async Object bound Dispatcher thread local state 
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 

#if !defined(SKL_STANDALONE)
#include "SkylakeLib.h"

namespace SKL
{
    AODTLSContext::AODTLSContext( ServerInstance* InServerInstance, WorkerGroupTag InWorkerGroupTag ) noexcept
        : bScheduleAODDelayedTasks{ false == InWorkerGroupTag.bIsActive }
        , SourceServerInstance{ InServerInstance }
        , ParentWorkerGroup{ InWorkerGroupTag }
    {
        SKL_ASSERT( nullptr != InServerInstance );
        SKL_ASSERT( InWorkerGroupTag.IsValid() );
    }

    AODTLSContext::~AODTLSContext() noexcept
    {
        Clear();
    }

    RStatus AODTLSContext::Initialize() noexcept 
    {
        Reset();

        // Build name
        ( void )snprintf( NameBuffer, 512, "[%ws AODTLSContext]", SourceServerInstance->GetName() );

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
        
        while( false == DelayedCustomObjectTasks.empty() )
        {
            TSharedPtr<IAODCustomObjectTask>::Static_Reset( DelayedCustomObjectTasks.top() );
            DelayedCustomObjectTasks.pop();
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
            SKLL_WRN( "AODTLSContext::Reset() no server instance specified!" );
            return;
        }

        //Cache for fast, thread local, access
        ServerFlags.Flags              = SourceServerInstance->ServerBuiltFlags.Flags;
        DeferredAODTasksHandlingGroups = SourceServerInstance->DeferredTasksHandlingGroups;
    }
}
#endif
