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
    void ScheduleTask( AODTLSContext& TLSContext, TTask* InTask ) noexcept 
    {
        static_assert( std::is_same_v<TTask, IAODSharedObjectTask> 
               || std::is_same_v<TTask, IAODStaticObjectTask> 
               || std::is_same_v<TTask, IAODCustomObjectTask> );

        //Select target worker group
        const std::vector<WorkerGroup*>& TaskHandlingWGs{ TLSContext.GetDeferredAODTasksHandlingGroups() };
        SKL_ASSERT( false == TaskHandlingWGs.empty() );

        WorkerGroup* TargetWG;
        if constexpr( CTaskScheduling_AssumeThatTaskHandlingWorkerGroupCountIsPowerOfTwo )
        {
            // Fastest
            const size_t TargetWGIndexMask{ TaskHandlingWGs.size() - 1 };
            SKL_ASSERT( IsPowerOfTwo( TargetWGIndexMask + 1 ) );
            TargetWG = TaskHandlingWGs[static_cast<size_t>( TLSContext.RRLastIndex++ ) & TargetWGIndexMask];
        }
        else 
        {
            if constexpr( CTaskScheduling_UseIfInsteadOfModulo )
            {
                // Potentially faster than modulo (if correct branch is predicted)
                size_t TargetWGIndex{ static_cast<size_t>( TLSContext.RRLastIndex++ ) };
                if( TargetWGIndex >= TaskHandlingWGs.size() )
                {
                    TargetWGIndex = 0U;
                }

                TargetWG = TaskHandlingWGs[TargetWGIndex];
            }
            else
            {
                // Slowest (beats branch miss-predict though)
                TargetWG = TaskHandlingWGs[static_cast<size_t>( TLSContext.RRLastIndex++ ) % TaskHandlingWGs.size()];
            }
        }

        SKL_ASSERT( nullptr != TargetWG );
        SKL_ASSERT( true == TargetWG->GetTag().bSupportsAOD );
        SKL_ASSERT( 0U < TargetWG->GetNumberOfRunningWorkers() );
        
        auto&        Workers                   { TargetWG->GetWorkers() };
        const size_t WorkersCount              { Workers.size() };
        const size_t WorkersCountWithoutInvalid{ WorkersCount - 1U };
        
        // 1 more than the invalid slot must be present
        SKL_ASSERT( 1U < WorkersCount );

        //Select target worker(round-robin) (we offset by one because index 0 is reserved for invalid worker and is nullptr)
        Worker* TargetW;
        if constexpr( CTaskScheduling_AssumeThatWorkersCountIsPowerOfTwo )
        {
            // Fastest
            const size_t TargetWIndexMask{ WorkersCountWithoutInvalid - 1 };
            SKL_ASSERT( IsPowerOfTwo( TargetWIndexMask + 1 ) );
            TargetW = Workers[( static_cast<size_t>( TLSContext.RRLastIndex2++ ) & TargetWIndexMask ) + 1U].get();
        }
        else
        {
            if constexpr( CTaskScheduling_UseIfInsteadOfModulo )
            {
                // Potentially faster than modulo (if correct branch is predicted)
                size_t TargetWIndex{ static_cast<size_t>( TLSContext.RRLastIndex2++ ) };
                if( TargetWIndex >= WorkersCountWithoutInvalid )
                {
                    TargetWIndex = 0U;
                }

                TargetW = Workers[TargetWIndex].get();
            }
            else
            {
                // Slowest (beats branch miss-predict though)
                TargetW = Workers[( static_cast<size_t>( TLSContext.RRLastIndex2++ ) % WorkersCountWithoutInvalid ) + 1U].get();        
            }
        }

        SKL_ASSERT( nullptr != TargetW );

        //Defer task to worker
        TargetW->Defer( InTask );
    }
}

namespace SKL::AOD
{
    void StaticObject::Flush() noexcept
    {
        while( true )
        {
            auto* Task{ reinterpret_cast<IAODStaticObjectTask*>( TaskQueue.Pop() ) };
            if( nullptr != Task ) SKL_LIKELY
            {
                Task->Dispatch();

                SKL_IFNOTSHIPPING( SKL_ASSERT_ALLWAYS( nullptr != Task ) );

                TSharedPtr<IAODStaticObjectTask>::Static_Reset( Task );

                if( 1 == RemainingTasksCount.decrement() )
                {
                    break;
                }
            }
            else
            {
                SKLL_TRACE_MSG( "Flush SKIPP!" );
                //SKL_BREAK();
                // There is low a possiblility for a bit of spinning because of the gap between: case1{ RefPoint[0] and RefPoint[1] } or case2{ RefPoint[0] and RefPoint[2] }
            }
        }
    }

    bool StaticObject::Dispatch( IAODStaticObjectTask* InTask ) noexcept
    {
        SKL_ASSERT( nullptr != InTask );
        SKL_ASSERT( false == InTask->IsNull() );
        
        // reset next ptr 
        InTask->Next = nullptr;

        if( RemainingTasksCount.increment() != 0 ) // RefPoint [0]
        {
            // Queue the task (must be done only after the count increment
            TaskQueue.Push( InTask ); // RefPoint [1]

            // There is a consumer present, just bail
            return false;
        }

        // Queue the task (must be done only after the count increment
        TaskQueue.Push( InTask ); // RefPoint [2]

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

    void StaticObject::DelayTask( IAODStaticObjectTask* InTask ) noexcept
    {
        SKL_ASSERT( nullptr != AODTLSContext::GetInstance() );
        auto& TLSData{ *AODTLSContext::GetInstance() };

        if constexpr( CTaskScheduling_AssumeAllWorkerGroupsHandleAOD )
        {
            // Push task
            TLSData.DelayedStaticObjectTasks.push( InTask );
        }
        else
        {

            if( FALSE == TLSData.bScheduleAODDelayedTasks )
            {
                // Push task
                TLSData.DelayedStaticObjectTasks.push( InTask );
            }
            else
            {
                // Schedule task to other workers
                ScheduleTask( TLSData, InTask );        
            }
        }
    }
}

namespace SKL::AOD
{
    void SharedObject::Flush() noexcept
    {
        while( true )
        {
            auto* Task{ reinterpret_cast<IAODSharedObjectTask*>( TaskQueue.Pop() ) };
            if( nullptr != Task ) SKL_LIKELY
            {
                Task->Dispatch();

                SKL_IFNOTSHIPPING( SKL_ASSERT_ALLWAYS( nullptr != Task ) );

                TSharedPtr<IAODSharedObjectTask>::Static_Reset( Task );

                if( 1 == RemainingTasksCount.decrement() )
                {
                    break;
                }
            }
            else
            {
                SKLL_VER( "Flush SKIPP!" );
                //SKL_BREAK();
                // There is low a possiblility for a bit of spinning because of the gap between: case1{ RefPoint[0] and RefPoint[1] } or case2{ RefPoint[0] and RefPoint[2] }
            }
        }
    }

    bool SharedObject::Dispatch( IAODSharedObjectTask* InTask ) noexcept
    {
        SKL_ASSERT( nullptr != InTask );
        SKL_ASSERT( false == InTask->IsNull() );
        
        // reset next ptr 
        InTask->Next = nullptr;

        if( RemainingTasksCount.increment() != 0 ) // RefPoint [0]
        {
            // Queue the task (must be done only after the count increment)
            TaskQueue.Push( InTask ); // RefPoint [1]

            // There is a consumer present, just bail
            return false;
        }

        // Queue the task (must be done only after the count increment)
        TaskQueue.Push( InTask ); // RefPoint [2]

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

    void SharedObject::DelayTask( IAODSharedObjectTask* InTask ) noexcept
    {
        SKL_ASSERT( nullptr != AODTLSContext::GetInstance() );
        auto& TLSData{ *AODTLSContext::GetInstance() };
        
        if constexpr( CTaskScheduling_AssumeAllWorkerGroupsHandleAOD )
        {
            // Push task
            TLSData.DelayedSharedObjectTasks.push( InTask );
        }
        else
        {
            if( FALSE == TLSData.bScheduleAODDelayedTasks )
            {
                // Push task
                TLSData.DelayedSharedObjectTasks.push( InTask );
            }
            else
            {
                // Schedule task to other workers
                ScheduleTask( TLSData, InTask );
            }
        }
    }
}

namespace SKL::AOD
{
    void CustomObject::Flush() noexcept
    {
        while( true )
        {
            auto* Task{ reinterpret_cast<IAODCustomObjectTask*>( TaskQueue.Pop() ) };
            if( nullptr != Task ) SKL_LIKELY
            {
                Task->Dispatch();

                TSharedPtr<IAODCustomObjectTask>::Static_Reset( Task );

                if( 1 == RemainingTasksCount.decrement() )
                {
                    break;
                }
            }
            else
            {
                // There is low a possiblility for a bit of spinning because of the gap between: case1{ RefPoint[0] and RefPoint[1] } or case2{ RefPoint[0] and RefPoint[2] }
            }
        }
    }

    bool CustomObject::Dispatch( IAODCustomObjectTask* InTask ) noexcept
    {
        SKL_ASSERT( nullptr != InTask );
        SKL_ASSERT( false == InTask->IsNull() );
        
        // reset next ptr 
        InTask->Next = nullptr;

        if( 0 != RemainingTasksCount.increment() ) // RefPoint [0]
        {
            // Queue the task (must be done only after the count increment
            TaskQueue.Push( InTask ); // RefPoint [1]

            // There is a consumer present, just bail
            return false;
        }

        // Queue the task (must be done only after the count increment
        TaskQueue.Push( InTask ); // RefPoint [2]

        // This thread is the new consumer for this AODObject instance, dispatch all available tasks.

        // Increment ref count for self (the reinterpret_cast should not affect the end result, the pointer is used as base to jump to the control block only)
        TCustomObjectSharedPtr::Static_IncrementReference( this );

        auto *TLSContext = AODTLSContext::GetInstance();
        SKL_ASSERT( nullptr != TLSContext );

        if( true == TLSContext->Flags.bIsAnyCustomDispatchInProgress )
        {
            TLSContext->PendingAOD_CustomObjects.push( this );
        }
        else
        {
            TLSContext->Flags.bIsAnyCustomDispatchInProgress = true;

            Flush();

            while( false == TLSContext->PendingAOD_CustomObjects.empty() )
            {
                auto* PendingAODObject{ TLSContext->PendingAOD_CustomObjects.front() };
                TLSContext->PendingAOD_CustomObjects.pop();

                PendingAODObject->Flush();
                TCustomObjectSharedPtr::Static_Reset( PendingAODObject );
            }

            TLSContext->Flags.bIsAnyCustomDispatchInProgress = false;

            TCustomObjectSharedPtr::Static_Reset( this );
        }                

        return true;
    }

    void CustomObject::DelayTask( IAODCustomObjectTask* InTask ) noexcept
    {
        SKL_ASSERT( nullptr != AODTLSContext::GetInstance() );
        auto& TLSData{ *AODTLSContext::GetInstance() };
        
        if constexpr( CTaskScheduling_AssumeAllWorkerGroupsHandleAOD )
        {
            // Push task
            TLSData.DelayedCustomObjectTasks.push( InTask );
        }
        else
        {
            if( FALSE == TLSData.bScheduleAODDelayedTasks )
            {
                // Push task
                TLSData.DelayedCustomObjectTasks.push( InTask );
            }
            else
            {
                // Schedule task to other workers
                ScheduleTask( TLSData, InTask );
            }
        }
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

    void IAODCustomObjectTask::SetParent( AOD::CustomObject* InObject )noexcept
    {
        // Increment ref count for self (the reinterpret_cast should not affect the end result, the pointer is used as base to jump to the control block only)
        // The type here is used only to tell the shared ptr that we have an object not an array
        TCustomObjectSharedPtr::Static_IncrementReference( InObject );
        Parent.Pointer = InObject;
    }
}

