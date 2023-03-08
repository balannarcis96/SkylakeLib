//!
//! \file ServerInstanceTLSContext.cpp
//! 
//! \brief Thread local context for all workers in a server instance
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#if !defined(SKL_STANDALONE)

#include "SkylakeLib.h"

namespace SKL
{
    ServerInstanceTLSContext::ServerInstanceTLSContext( ServerInstance* InServerInstance, WorkerGroupTag InWorkerGroupTag ) noexcept
        : ParentWorkerGroup{ InWorkerGroupTag }
        , SourceServerInstance{ InServerInstance }
    {
        SKL_ASSERT( nullptr != InServerInstance );
        SKL_ASSERT( InWorkerGroupTag.IsValid() );
    }

    ServerInstanceTLSContext::~ServerInstanceTLSContext() noexcept
    {
        Clear();
    }

    RStatus ServerInstanceTLSContext::Initialize() noexcept 
    {
        Reset();

        // Build name
        ( void )snprintf( NameBuffer, 512, "[%ws ServerInstanceTLSContext]", SourceServerInstance->GetName() );

        return RSuccess;
    }

    void ServerInstanceTLSContext::Clear() noexcept
    {
        DeferredTasksHandlingGroups.clear();
        ServerFlags.Flags = 0;

        while( false == DelayedTasks.empty() )
        {
            if constexpr( CTaskScheduling_AssumeAllWorkerGroupsHandleTimerTasks 
                       && CTaskScheduling_AssumeAllWorkerGroupsHaveTLSMemoryManagement )
            {
                // allocated from the thread local memory manager
                TLSSharedPtr<ITask>::Static_Reset( DelayedTasks.top() );
            }
            else
            {
                // allocated from the global memory manager
                TSharedPtr<ITask>::Static_Reset( DelayedTasks.top() );
            }

            DelayedTasks.pop();
        }
    }

    void ServerInstanceTLSContext::Reset() noexcept
    {   
        DeferredTasksHandlingGroups.clear();
        ServerFlags.Flags = 0;

        while( false == DelayedTasks.empty() )
        {
            if constexpr( CTaskScheduling_AssumeAllWorkerGroupsHandleTimerTasks 
                       && CTaskScheduling_AssumeAllWorkerGroupsHaveTLSMemoryManagement )
            {
                // allocated from the thread local memory manager
                TLSSharedPtr<ITask>::Static_Reset( DelayedTasks.top() );
            }
            else
            {
                // allocated from the global memory manager
                TSharedPtr<ITask>::Static_Reset( DelayedTasks.top() );
            }

            DelayedTasks.pop();
        }

        if( nullptr == SourceServerInstance )   
        {
            SKLL_WRN( "ServerInstanceTLSContext::Reset() no server instance specified!" );
            return;
        }

        //Cache for fast, thread local, access
        ServerFlags.Flags           = SourceServerInstance->ServerBuiltFlags.Flags;
        DeferredTasksHandlingGroups = SourceServerInstance->DeferredTasksHandlingGroups;
    }
}

#endif
