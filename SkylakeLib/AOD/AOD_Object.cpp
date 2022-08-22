//!
//! \file AOD_Object.cpp
//! 
//! \brief Async Object bound Dispatcher Task abstraction for SkylakeLib
//! 
//! \reference https://github.com/balannarcis96/Dispatcher (G.O.D: Grand Object-bound Dispatcher)
//!
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 

#include "SkylakeLib.h"

namespace SKL
{
    bool ScheduleTask( AODTLSContext& TLSContext, IAODTask* InTask ) noexcept
    {
        //Select target worker group
        auto TaskHandlingWGs{ TLSContext.GetDeferredAODTasksHandlingGroups() };
        SKL_ASSERT( false == TaskHandlingWGs.empty() );
        auto* TargetWG{ TaskHandlingWGs[ static_cast<size_t>( TLSContext.RRLastIndex++ ) % TaskHandlingWGs.size() ].get() };
        SKL_ASSERT( true == TargetWG->GetTag().bHandlesTasks );
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
            SKL_ERR( "AODObject::ScheduleTask() Failed to schedule task to workers!" );
            TSharedPtr<IAODTask>::Static_Reset( InTask );
            return false;
        }

        return true;
    }

    void AODObject::Flush() noexcept
    {
        while( true )
        {
            auto* Task{ this->TaskQueue.Pop() };
            SKL_IFNOTSHIPPING( SKL_ASSERT_ALLWAYS( nullptr != Task ) );
           
            Task->Dispatch();

            SKL_IFNOTSHIPPING( SKL_ASSERT_ALLWAYS( nullptr != Task ) );

            TSharedPtr<IAODTask>::Static_Reset( Task );

            if( 1 == RemainingTasksCount.decrement() )
            {
                break;
            }
        }
    }

    bool AODObject::Dispatch( IAODTask* InTask ) noexcept
    {
        SKL_ASSERT( nullptr != InTask );
        SKL_ASSERT( false == InTask->IsNull() );

        TaskQueue.Push( InTask );
        
        if( RemainingTasksCount.increment() != 0 )
        {
            // There is a consumer present, just bail
            return false;
        }

        // This thread is the new consumer for this AODObject instance, dispatch all available tasks.

        // Increment ref count for self
        TSharedPtr<AODObject>::Static_IncrementReference( this );

        auto *TLSContext = AODTLSContext::GetInstance();
        SKL_ASSERT( nullptr != TLSContext );

        if( true == TLSContext->Flags.bIsAnyDispatchInProgress )
        {
            TLSContext->PendingAODObjects.push( this );
        }
        else
        {
            TLSContext->Flags.bIsAnyDispatchInProgress = true;

            Flush();

            while( false == TLSContext->PendingAODObjects.empty() )
            {
                auto* PendingAODObject{ TLSContext->PendingAODObjects.front() };
                TLSContext->PendingAODObjects.pop();

                PendingAODObject->Flush();
                TSharedPtr<AODObject>::Static_Reset( PendingAODObject );
            }

            TLSContext->Flags.bIsAnyDispatchInProgress = false;

            TSharedPtr<AODObject>::Static_Reset( this );
        }                

        return true;
    }

    bool AODObject::DelayTask( IAODTask* InTask ) noexcept
    {
        SKL_ASSERT( nullptr != AODTLSContext::GetInstance() );
        auto& TLSData{ *AODTLSContext::GetInstance() };

        if( FALSE == TLSData.bScheduleAODDelayedTasks )
        {
            // Push task
            TLSData.DelayedTasks.push( InTask );
            return true;
        }

        return ScheduleTask( TLSData, InTask );
    }
}