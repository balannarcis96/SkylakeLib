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

namespace SKL::AOD
{
    template<typename TTask>
    bool ScheduleTask( AODTLSContext& TLSContext, TTask* InTask ) noexcept requires( std::is_same_v<TTask, IAODSharedObjectTask> || std::is_same_v<TTask, IAODStaticObjectTask> )
    {
        //Select target worker group
        auto TaskHandlingWGs{ TLSContext.GetDeferredAODTasksHandlingGroups() };
        SKL_ASSERT( false == TaskHandlingWGs.empty() );
        auto* TargetWG{ TaskHandlingWGs[ static_cast<size_t>( TLSContext.RRLastIndex++ ) % TaskHandlingWGs.size() ] };
        SKL_ASSERT( true == TargetWG->GetTag().bSupportsAOD );
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
            TSharedPtr<TTask>::Static_Reset( InTask );
            return false;
        }

        return true;
    }
}

namespace SKL::AOD
{
    void StaticObject::Flush() noexcept
    {
        while( true )
        {
            auto* Task{ reinterpret_cast<IAODStaticObjectTask*>( TaskQueue.Pop() ) };
            SKL_IFNOTSHIPPING( SKL_ASSERT_ALLWAYS( nullptr != Task ) );
           
            Task->Dispatch();

            SKL_IFNOTSHIPPING( SKL_ASSERT_ALLWAYS( nullptr != Task ) );

            TSharedPtr<IAODStaticObjectTask>::Static_Reset( Task );

            if( 1 == RemainingTasksCount.decrement() )
            {
                break;
            }
        }
    }

    bool StaticObject::Dispatch( IAODStaticObjectTask* InTask ) noexcept
    {
        SKL_ASSERT( nullptr != InTask );
        SKL_ASSERT( false == InTask->IsNull() );

        TaskQueue.Push( InTask );
        
        if( RemainingTasksCount.increment() != 0 )
        {
            // There is a consumer present, just bail
            return false;
        }

        // This thread is the new consumer for this AODStaticObject instance, dispatch all available tasks.

        // Static object lifetime expected (no tasks should be issued before the object was destroyed)
        //TSharedPtr<AODObject>::Static_IncrementReference( this );

        auto *TLSContext = AODTLSContext::GetInstance();
        SKL_ASSERT( nullptr != TLSContext );

        if( true == TLSContext->Flags.bIsAnyStaticDispatchInProgress )
        {
            TLSContext->PendingAOD_StaticObjects.push( this );
        }
        else
        {
            TLSContext->Flags.bIsAnyStaticDispatchInProgress = true;

            Flush();

            while( false == TLSContext->PendingAOD_StaticObjects.empty() )
            {
                auto* PendingAODObject{ TLSContext->PendingAOD_StaticObjects.front() };
                TLSContext->PendingAOD_StaticObjects.pop();

                PendingAODObject->Flush();

                // Static object lifetime expected (no tasks should be issued before the object was destroyed)
                //TSharedPtr<AODObject>::Static_Reset( PendingAODObject );
            }

            TLSContext->Flags.bIsAnyStaticDispatchInProgress = false;

            // Static object lifetime expected (no tasks should be issued before the object was destroyed)
            //TSharedPtr<AODObject>::Static_Reset( this );
        }                

        return true;
    }

    bool StaticObject::DelayTask( IAODStaticObjectTask* InTask ) noexcept
    {
        SKL_ASSERT( nullptr != AODTLSContext::GetInstance() );
        auto& TLSData{ *AODTLSContext::GetInstance() };

        if( FALSE == TLSData.bScheduleAODDelayedTasks )
        {
            // Push task
            TLSData.DelayedStaticObjectTasks.push( InTask );
            return true;
        }

        return ScheduleTask( TLSData, InTask );
    }
}

namespace SKL::AOD
{
    void SharedObject::Flush() noexcept
    {
        while( true )
        {
            auto* Task{ reinterpret_cast<IAODSharedObjectTask*>( TaskQueue.Pop() ) };
            SKL_IFNOTSHIPPING( SKL_ASSERT_ALLWAYS( nullptr != Task ) );
           
            Task->Dispatch();

            SKL_IFNOTSHIPPING( SKL_ASSERT_ALLWAYS( nullptr != Task ) );

            TSharedPtr<IAODSharedObjectTask>::Static_Reset( Task );

            if( 1 == RemainingTasksCount.decrement() )
            {
                break;
            }
        }
    }

    bool SharedObject::Dispatch( IAODSharedObjectTask* InTask ) noexcept
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

        // Increment ref count for self (the reinterpret_cast should not affect the end result, the pointer is used as base to jump to the control block only)
        TSharedPtr<SharedObject>::Static_IncrementReference( reinterpret_cast<SharedObject*>( TargetSharedPointer ) );

        auto *TLSContext = AODTLSContext::GetInstance();
        SKL_ASSERT( nullptr != TLSContext );

        if( true == TLSContext->Flags.bIsAnySharedDispatchInProgress )
        {
            TLSContext->PendingAOD_SharedObjects.push( this );
        }
        else
        {
            TLSContext->Flags.bIsAnySharedDispatchInProgress = true;

            Flush();

            while( false == TLSContext->PendingAOD_SharedObjects.empty() )
            {
                auto* PendingAODObject{ TLSContext->PendingAOD_SharedObjects.front() };
                TLSContext->PendingAOD_SharedObjects.pop();

                PendingAODObject->Flush();
                TSharedPtr<SharedObject>::Static_Reset( reinterpret_cast<SharedObject*>( PendingAODObject->TargetSharedPointer ) );
            }

            TLSContext->Flags.bIsAnySharedDispatchInProgress = false;

            TSharedPtr<SharedObject>::Static_Reset( reinterpret_cast<SharedObject*>( TargetSharedPointer ) );
        }                

        return true;
    }

    bool SharedObject::DelayTask( IAODSharedObjectTask* InTask ) noexcept
    {
        SKL_ASSERT( nullptr != AODTLSContext::GetInstance() );
        auto& TLSData{ *AODTLSContext::GetInstance() };

        if( FALSE == TLSData.bScheduleAODDelayedTasks )
        {
            // Push task
            TLSData.DelayedSharedObjectTasks.push( InTask );
            return true;
        }

        return ScheduleTask( TLSData, InTask );
    }
}

namespace SKL
{
    void IAODSharedObjectTask::SetParent( AOD::SharedObject* InObject )noexcept
    {
        // Increment ref count for self (the reinterpret_cast should not affect the end result, the pointer is used as base to jump to the control block only)
        // The type here is used only to tell the shared ptr that we have an object not an array
        TSharedPtr<AOD::SharedObject>::Static_IncrementReference( reinterpret_cast<AOD::SharedObject*>( InObject->TargetSharedPointer ) );
        Parent.Pointer = InObject;
    }
}