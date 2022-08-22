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
    struct AODTaskQueue
    {
        AODTaskQueue() noexcept: Head{ reinterpret_cast<IAODTask*>( &Stub ) }, Tail{ reinterpret_cast<IAODTask*>( &Stub ) }, Stub{} {}
        ~AODTaskQueue() noexcept = default;

        // Can't copy or move
        AODTaskQueue( const AODTaskQueue & ) = delete;
        AODTaskQueue &operator=( const AODTaskQueue & ) = delete;
        AODTaskQueue( AODTaskQueue && ) = delete;
        AODTaskQueue &operator=( AODTaskQueue && ) = delete;

        //! Multiple producers push
        SKL_FORCEINLINE void Push( IAODTask* InTask ) noexcept
        {
            auto* PrevNode{ std::atomic_exchange_explicit( &Head, InTask, std::memory_order_acq_rel ) };
            PrevNode->Next = InTask;
        }

        //! Is the pointer pointing to the stub 
        SKL_FORCEINLINE bool IsStub( void* InPtr ) const noexcept { return InPtr == &Stub; }

        //! Single consumer pop
        SKL_NODISCARD IAODTask* Pop() noexcept  
        {
            IAODTask* LocalTail{ Tail };
            IAODTask* LocalNext{ LocalTail->Next };

            if( reinterpret_cast<IAODTask*>( &Stub ) == LocalTail ) SKL_UNLIKELY
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

                SKL_IFNOTSHIPPING( SKL_ASSERT_ALLWAYS( false == IsStub( LocalTail ) ) );
                return LocalTail;
            }

            // sq-consistent load
            const IAODTask* LocalHead{ Head };
            if( LocalTail != LocalHead )
            {
                return nullptr;
            }

            //Last pop
            Stub.Next = nullptr;
            Push( reinterpret_cast<IAODTask*>( &Stub ) );

            LocalNext = Tail->Next;
            if( nullptr != LocalNext )
            {
                Tail = LocalNext;

                SKL_IFNOTSHIPPING( SKL_ASSERT_ALLWAYS( false == IsStub( LocalTail ) ) );
                return LocalTail;
            }

            return nullptr;
        }

    private:
        std::atomic<IAODTask*> Head; //!< Head of the queue
        IAODTask*              Tail; //!< Tail of the queue
        IAODTaskBase           Stub; //!< Stub item
    };
}
