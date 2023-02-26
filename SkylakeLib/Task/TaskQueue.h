//!
//! \file TaskQueue.h
//! 
//! \brief ITask thread safe queue for SkylakeLib
//! 
//! \reference https://github.com/balannarcis96/Dispatcher (G.O.D: Grand Object-bound Dispatcher)
//!
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    //! Single consumer multiple producers intrusive singly-linked list based lock free queue
    struct TaskQueue
    {
        TaskQueue() noexcept: Head{ reinterpret_cast<ITask*>( &Stub ) }, Tail{ reinterpret_cast<ITask*>( &Stub ) }, Stub{} {}
        ~TaskQueue() noexcept = default;

        // Can't copy or move
        TaskQueue( const TaskQueue & ) = delete;
        TaskQueue &operator=( const TaskQueue & ) = delete;
        TaskQueue( TaskQueue && ) = delete;
        TaskQueue &operator=( TaskQueue && ) = delete;

        //! Is the pointer pointing to the stub 
        SKL_FORCEINLINE bool IsStub( void* InPtr ) const noexcept { return InPtr == &Stub; }

        //! Multiple producers push
        SKL_FORCEINLINE void Push( ITask* InTask ) noexcept
        {
            SKL_ASSERT( nullptr == InTask->Next );
            auto* PrevNode{ std::atomic_exchange_explicit( &Head, InTask, std::memory_order_acq_rel ) };
            PrevNode->Next = InTask;
        }

        //! Single consumer pop
        SKL_NODISCARD ITask* Pop() noexcept  
        {
            ITask* LocalTail{ Tail };
            ITask* LocalNext{ LocalTail->Next };

            if( reinterpret_cast<ITask*>( &Stub ) == LocalTail ) SKL_UNLIKELY
            {
                if( nullptr == LocalNext )
                {       
                    // Empty
                    return nullptr;
                }

                // First pop
                Tail      = LocalNext;
                LocalTail = LocalNext;
                LocalNext = LocalNext->Next;
            }

            // Most cases
            if( nullptr != LocalNext ) SKL_LIKELY
            {
                Tail = LocalNext;

                SKL_ASSERT( false == IsStub( LocalTail ) );
                return LocalTail;
            }

            // sq-consistent load
            const ITask* LocalHead{ Head };
            if( LocalTail != LocalHead )
            {
                return nullptr;
            }

            //Last pop
            Stub.Next = nullptr;
            Push( reinterpret_cast<ITask*>( &Stub ) );

            LocalNext = Tail->Next;
            if( nullptr != LocalNext )
            {
                Tail = LocalNext;

                SKL_ASSERT_ALLWAYS( false == IsStub( LocalTail ) );
                return LocalTail;
            }

            return nullptr;
        }

    private:
        std::atomic<ITask*>       Head; //!< Head of the queue
        ITask*                    Tail; //!< Tail of the queue
        ITaskBase                 Stub; //!< Stub item
    };
}
