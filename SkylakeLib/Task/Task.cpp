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
    bool ScheduleTask( ServerInstanceTLSContext& TLSContext, ITask* InTask ) noexcept
    {
        //Select target worker group
        auto TaskHandlingWGs{ TLSContext.GetDeferredTasksHandlingGroups() };
        SKL_ASSERT( false == TaskHandlingWGs.empty() );
        auto* TargetWG{ TaskHandlingWGs[ static_cast<size_t>( TLSContext.RRLastIndex++ ) % TaskHandlingWGs.size() ].get() };
        SKL_ASSERT( true == TargetWG->GetTag().bHandlesTimerTasks );
        SKL_ASSERT( nullptr != TargetWG );
        SKL_ASSERT( 0 < TargetWG->GetNumberOfRunningWorkers() );
        
        constexpr int32_t MaxTries   { 3 };
        int32_t           HandleTries{ 0 };
        
        while( MaxTries > HandleTries )
        {
            //Select target worker
            auto Workers{ TargetWG->GetWorkers() };
            auto* TargetW{ Workers[ static_cast<size_t>( TLSContext.RRLastIndex2++ ) % Workers.size() ].get() };
            if( nullptr != TargetW ) SKL_LIKELY
            {
                //Defer task to worker
                TargetW->Defer( InTask );
                break;
            }
        
            ++HandleTries;
        }
        
        if( HandleTries > MaxTries ) SKL_UNLIKELY
        {
            SKL_ERR( "::ScheduleTask() Failed to schedule task to workers!" );
            TSharedPtr<ITask>::Static_Reset( InTask );
            return false;
        }

        return true;
    }

    bool DeferTask( ITask* InTask ) noexcept
    {
        SKL_ASSERT( nullptr != InTask );
        SKL_ASSERT( 1 == TSharedPtr<ITask>::Static_GetReferenceCount( InTask ) );

        auto& TLSContext{ *ServerInstanceTLSContext::GetInstance() };
        if( true == TLSContext.GetCurrentWorkerGroupTag().bHandlesTimerTasks )
        {
            TLSContext.DelayedTasks.push( InTask );
            return true;
        }

        return ScheduleTask( TLSContext, InTask );
    }
}
