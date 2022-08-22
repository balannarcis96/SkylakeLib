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
        AODObject() noexcept = default;
        ~AODObject() noexcept = default;

        //! Execute the functor thread safe relative to the object [void(AODObject&)noexcept]
        template<typename TFunctor>
        SKL_FORCEINLINE RStatus DoAsync( TFunctor&& InFunctor ) noexcept
        {
            using TaskType = AODTask<sizeof(TFunctor)>;
            
            TaskType* NewTask{ MakeSharedRaw<TaskType>() };
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

        //! Execute the functor after [AfterMilliseconds], thread safe relative to the object [void(AODObject&)noexcept]
        template<typename TFunctor>
        SKL_FORCEINLINE RStatus DoAsyncAfter( TDuration AfterMilliseconds, TFunctor&& InFunctor ) noexcept
        {
            using TaskType = AODTask<sizeof(TFunctor)>;
            
            TaskType* NewTask{ MakeSharedRaw<TaskType>() };
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
    };
}
