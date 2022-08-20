//!
//! \file AOD_Object.h
//! 
//! \brief Async Object bound Dispatcher Task abstraction for SkylakeLib
//! 
//! \reference https://github.com/balannarcis96/Dispatcher (G.O.D: Grand Object-bound Dispatcher)
//!
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    struct AODObject
    {
        AODObject() noexcept
        {
            SKL_ASSERT_MSG( ( (uintptr_t)this ) % SKL_CACHE_LINE_SIZE == 0, "Instances of AODObject must be cache line aligned" );
        }
        ~AODObject() noexcept = default;

        template<typename TFunctor>
        SKL_FORCEINLINE RStatus DoAsync( TFunctor&& InFunctor ) noexcept
        {
            using TaskType = AODTask<sizeof(TFunctor)>;
            
            if( TRUE == bIsPendingDestroy ) SKL_UNLIKELY
            {
                return RAborted;
            }

            TaskType* NewTask = MakeSharedRaw<TaskType>();
            if( nullptr == NewTask ) SKL_UNLIKELY
            {   
                SKL_ERR( "AODObject::DoAsync() Failed to allocate task!" );
                return RAllocationFailed;
            }

            NewTask->SetParent( this );
            NewTask->SetDispatch( std::forward<TFunctor>( InFunctor ) );

            if( Dispatch( NewTask ) )
            {
                return RExecutedSync;
            }
        
            return RSuccess;
        }

        template<typename TFunctor>
        SKL_FORCEINLINE RStatus DoAsyncAfter( TDuration AfterMilliseconds, TFunctor&& InFunctor ) noexcept
        {
            using TaskType = AODTask<sizeof(TFunctor)>;
            
            if( TRUE == bIsPendingDestroy ) SKL_UNLIKELY
            {
                return RAborted;
            }

            TaskType* NewTask = MakeSharedRaw<TaskType>();
            if( nullptr == NewTask ) SKL_UNLIKELY
            {   
                SKL_ERR( "AODObject::DoAsyncAfter() Failed to allocate task!" );
                return RAllocationFailed;
            }

            NewTask->SetParent( this );
            NewTask->SetDue( AfterMilliseconds );
            NewTask->SetDispatch( std::forward<TFunctor>( InFunctor ) );

            if ( false == DelayTask( NewTask ) ) SKL_UNLIKELY
            {
                return RFail;
            }

            return RSuccess;
        }

    private:
        void Flush() noexcept;
        bool Dispatch( IAODTask* InTask ) noexcept;
        bool DelayTask( IAODTask* InTask ) noexcept;

    protected:
        //First cache line
        std::relaxed_value<uint64_t> RemainingTasksCount;
        AODTaskQueue                 TaskQueue;
        uint8_t                      CacheLinePayload[ SKL_CACHE_LINE_SIZE - sizeof( void* ) - sizeof( AODTaskQueue ) ];

        //Next cache line
        uint8_t                      bIsPendingDestroy{ FALSE };
    };
}