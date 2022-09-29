//!
//! \file AOD_Queue.h
//! 
//! \brief Async Object bound Dispatcher Queue abstraction for SkylakeLib
//! 
//! \reference https://github.com/balannarcis96/Dispatcher (G.O.D: Grand Object-bound Dispatcher)
//!
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    //! Single consumer multiple producers intrusive singly-linked list based lock free queue
    struct alignas( SKL_ALIGNMENT ) AODTaskQueue
    {
        AODTaskQueue() noexcept: Head{ reinterpret_cast<IAODTaskBase*>( &Stub ) }, Tail{ reinterpret_cast<IAODTaskBase*>( &Stub ) }, Stub{} {}
        ~AODTaskQueue() noexcept = default;

        // Can't copy or move
        AODTaskQueue( const AODTaskQueue & ) = delete;
        AODTaskQueue &operator=( const AODTaskQueue & ) = delete;
        AODTaskQueue( AODTaskQueue && ) = delete;
        AODTaskQueue &operator=( AODTaskQueue && ) = delete;

        //! Multiple producers push
        SKL_FORCEINLINE void Push( IAODTaskBase* InTask ) noexcept
        {
            SKL_ASSERT( nullptr != InTask );
            SKL_ASSERT( nullptr == InTask->Next );
            auto* PrevNode{ std::atomic_exchange_explicit( &Head, InTask, std::memory_order_acq_rel ) };
            TaskCount.increment();
            PrevNode->Next = InTask;
        }

        //! Is the pointer pointing to the stub 
        SKL_FORCEINLINE bool IsStub( void* InPtr ) const noexcept { return InPtr == &Stub; }

        //! Single consumer pop
        SKL_NODISCARD IAODTaskBase* Pop() noexcept  
        {
            const auto IncResult { Guard.increment() };
            if( IncResult != 0 )
            {
                SKL_BREAK();
            }

            IAODTaskBase* LocalTail{ Tail };
            IAODTaskBase* LocalNext{ LocalTail->Next };

            if( reinterpret_cast<IAODTaskBase*>( &Stub ) == LocalTail ) SKL_UNLIKELY
            {
                if( nullptr == LocalNext )
                {       
                    const auto DecResult { Guard.decrement() };
                    if( DecResult != 1 )
                    {
                        SKL_BREAK();
                    }

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

                TaskCount.decrement();
                SKL_IFNOTSHIPPING( SKL_ASSERT_ALLWAYS( false == IsStub( LocalTail ) ) );
                
                const auto DecResult { Guard.decrement() };
                if( DecResult != 1 )
                {
                    SKL_BREAK();
                }

                return LocalTail;
            }

            // sq-consistent load
            const IAODTaskBase* LocalHead{ Head };
            if( LocalTail != LocalHead )
            {
                const auto DecResult { Guard.decrement() };
                if( DecResult != 1 )
                {
                    SKL_BREAK();
                }

                return nullptr;
            }

            //Last pop
            Stub.Next = nullptr;
            //Push( reinterpret_cast<IAODTaskBase*>( &Stub ) );

            {
                auto* PrevNode{ std::atomic_exchange_explicit( &Head, &Stub, std::memory_order_acq_rel ) };
                PrevNode->Next = &Stub;
            }

            LocalNext = LocalTail->Next;
            if( nullptr != LocalNext )
            {
                Tail = LocalNext;

                SKL_IFNOTSHIPPING( SKL_ASSERT_ALLWAYS( false == IsStub( LocalTail ) ) );
                TaskCount.decrement();

                const auto DecResult { Guard.decrement() };
                if( DecResult != 1 )
                {
                    SKL_BREAK();
                }

                return LocalTail;
            }

            const auto DecResult { Guard.decrement() };
            if( DecResult != 1 )
            {
                SKL_BREAK();
            }

            return nullptr;
        }

    private:
        std::atomic<IAODTaskBase*> Head; //!< Head of the queue
        IAODTaskBase*              Tail; //!< Tail of the queue
        IAODTaskBase               Stub; //!< Stub item

        std::relaxed_value<size_t> TaskCount{ 0 };
        std::relaxed_value<int32_t> Guard{ 0 };
    };
}
