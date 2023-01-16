//!
//! \file Task.cpp
//! 
//! \brief Task abstractions based on ASD plus allocation strategies
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#include "SkylakeLib.h"

namespace SKL
{
    void ScheduleTask( ServerInstanceTLSContext& TLSContext, ITask* InTask ) noexcept
    {
        const std::vector<WorkerGroup*>& TaskHandlingWGs{ TLSContext.GetDeferredTasksHandlingGroups() };
        SKL_ASSERT( false == TaskHandlingWGs.empty() );

        //Select target worker group(round-robin)
        WorkerGroup* TargetWG;
        if constexpr( CTaskScheduling_AssumeThatTaskHandlingWorkerGroupCountIsPowerOfTwo )
        {
            const size_t TargetWGIndexMask{ TaskHandlingWGs.size() - 1 };
            SKL_ASSERT( IsPowerOfTwo( TargetWGIndexMask + 1 ) );
            TargetWG = TaskHandlingWGs[static_cast<size_t>( TLSContext.RRLastIndex++ ) & TargetWGIndexMask];
        }
        else 
        {
            if constexpr( CTaskScheduling_UseIfInsteadOfModulo )
            {
                size_t TargetWGIndex{ static_cast<size_t>( TLSContext.RRLastIndex++ ) };
                if( TargetWGIndex >= TaskHandlingWGs.size() )
                {
                    TargetWGIndex = 0U;
                }

                TargetWG = TaskHandlingWGs[TargetWGIndex];
            }
            else
            {
                TargetWG = TaskHandlingWGs[static_cast<size_t>( TLSContext.RRLastIndex++ ) % TaskHandlingWGs.size()];
            }
        }

        SKL_ASSERT( true == TargetWG->GetTag().bHandlesTimerTasks );
        SKL_ASSERT( nullptr != TargetWG );
        SKL_ASSERT( 0 < TargetWG->GetNumberOfRunningWorkers() );
        
        auto&        Workers                   { TargetWG->GetWorkers() };
        const size_t WorkersCount              { Workers.size() };
        const size_t WorkersCountWithoutInvalid{ WorkersCount - 1U };

        // 1 more than the invalid slot must be present
        SKL_ASSERT( 1U < WorkersCount );

        //Select target worker(round-robin) (we offset by one because index 0 is reserved for invalid worker and is nullptr)
        Worker* TargetW;
        if constexpr( CTaskScheduling_AssumeThatWorkersCountIsPowerOfTwo )
        {
            // Fast

            const size_t TargetWIndexMask{ WorkersCountWithoutInvalid - 1 };
            SKL_ASSERT( IsPowerOfTwo( TargetWIndexMask + 1 ) );
            TargetW = Workers[( static_cast<size_t>( TLSContext.RRLastIndex2++ ) & TargetWIndexMask ) + 1U].get();
        }
        else
        {
            if constexpr( CTaskScheduling_UseIfInsteadOfModulo )
            {
                // Potentially fastest (if correct branch is predicted)
                size_t TargetWIndex{ static_cast<size_t>( TLSContext.RRLastIndex2++ ) };
                if( TargetWIndex >= WorkersCountWithoutInvalid )
                {
                    TargetWIndex = 0U;
                }

                TargetW = Workers[TargetWIndex].get();
            }
            else
            {
                // Slowest (beats branch miss-predict)
                TargetW = Workers[( static_cast<size_t>( TLSContext.RRLastIndex2++ ) % WorkersCountWithoutInvalid ) + 1U].get();        
            }
        }

        SKL_ASSERT( nullptr != TargetW );

        //Defer task to worker
        TargetW->Defer( InTask );
    }

    void DeferTask( ITask* InTask ) noexcept
    {
        SKL_ASSERT( nullptr != InTask );
        SKL_ASSERT( 1U == TSharedPtr<ITask>::Static_GetReferenceCount( InTask ) );

        ServerInstanceTLSContext& TLSContext{ *ServerInstanceTLSContext::GetInstance() };

        if constexpr( CTaskScheduling_AssumeAllWorkerGroupsHandleTimerTasks )
        {
            SKL_ASSERT( TLSContext.GetCurrentWorkerGroupTag().bHandlesTimerTasks );
            TLSContext.DelayedTasks.push( InTask );
        }
        else
        {
            if( TLSContext.GetCurrentWorkerGroupTag().bHandlesTimerTasks )
            {
                TLSContext.DelayedTasks.push( InTask );
            }
            else
            {
                return ScheduleTask( TLSContext, InTask );
            }
        }
    }

    //! Called from withing the handler of a deferred task to defer the same task again 
    void DeferTaskAgain( ITask* InTask ) noexcept
    {
        SKL_ASSERT( nullptr != InTask );
        SKL_ASSERT( 0U < TSharedPtr<ITask>::Static_GetReferenceCount( InTask ) );

        // Add reference
        TSharedPtr<ITask>::Static_IncrementReference( InTask );

        ServerInstanceTLSContext& TLSContext{ *ServerInstanceTLSContext::GetInstance() };
        SKL_ASSERT( TLSContext.GetCurrentWorkerGroupTag().bHandlesTimerTasks );

        TLSContext.PendingDelayedTasks.push( InTask );
    }

    //! Called from withing the handler of a deferred task to defer the same task again 
    void DeferTaskAgain( TDuration AfterMilliseconds, ITask* InTask ) noexcept
    {
        SKL_ASSERT( nullptr != InTask );
        SKL_ASSERT( 0U < TSharedPtr<ITask>::Static_GetReferenceCount( InTask ) );

        InTask->SetDue( AfterMilliseconds );

        // Add reference
        TSharedPtr<ITask>::Static_IncrementReference( InTask );

        auto& TLSContext{ *ServerInstanceTLSContext::GetInstance() };
        SKL_ASSERT( TLSContext.GetCurrentWorkerGroupTag().bHandlesTimerTasks );

        TLSContext.PendingDelayedTasks.push( InTask );
    }
}
