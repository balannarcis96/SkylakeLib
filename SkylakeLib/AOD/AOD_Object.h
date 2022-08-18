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
        template<typename TFunctor>
        SKL_FORCEINLINE RStatus DoAsync( TFunctor&& InFunctor ) noexcept
        {
            using TaskType = AODTask<sizeof(TFunctor)>;
            
            TaskType* NewTask = MakeSharedRaw<TaskType>();
            if( nullptr == NewTask ) SKL_UNLIKELY
            {   
                SKL_ERR( "AODObject::DoAsync() Failed to allocate task!" );
                return RAllocationFailed;
            }

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
            //@TODO
        }

    private:
        void Flush() noexcept;
        bool Dispatch( IAODTask* InTask ) noexcept;

    protected:
        std::relaxed_value<uint16_t> RemainingTasksCount;
        uint16_t                     Padding1;
        uint32_t                     Padding2;
        AODTaskQueue                 TaskQueue;
        uint8_t                      CacheLinePayload[ SKL_CACHE_LINE_SIZE - sizeof( void* ) - sizeof( AODTaskQueue ) ];
    };
}