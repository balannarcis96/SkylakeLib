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
        AODTaskQueue() noexcept: Head{ &Stub }, Tail{ &Stub }, Stub{} {}
        ~AODTaskQueue() noexcept = default;

        // Can't copy or move
        AODTaskQueue( const AODTaskQueue & ) = delete;
        AODTaskQueue &operator=( const AODTaskQueue & ) = delete;
        AODTaskQueue( AODTaskQueue && )                 = delete;
        AODTaskQueue &operator=( AODTaskQueue && ) = delete;

        //! Multiple producers push
        SKL_FORCEINLINE void Push( IAODTask* InTask ) noexcept
        {
            auto* PrevNode{ Head.exchange( InTask ) };
            PrevNode->Next = InTask;
        }

        //! Single consumer pop
        SKL_NODISCARD IAODTask* Pop() noexcept  
        {
            IAODTask* LocalTail{ Tail };
            IAODTask* LocalNext{ LocalTail->Next };

            if( &Stub == LocalTail ) SKL_UNLIKELY
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
                return LocalTail;
            }

            const IAODTask* LocalHead{ Head.load_acquire() };
            if( LocalTail != LocalHead )
            {
                return nullptr;
            }

            //Last pop
            Stub.Next = nullptr;
            Push( &Stub );

            LocalNext = Tail->Next;
            if( nullptr != LocalNext )
            {
                Tail = LocalNext;
                return LocalTail;
            }

            return nullptr;
        }

    private:
        std::synced_value<IAODTask*> Head; //!< Head of the queue
        IAODTask*                    Tail; //!< Tail of the queue
        IAODTask                     Stub; //!< Stub item
    };
}
