//!
//! \file TLSSyncTask.h
//! 
//! \brief TLS sync system task queue abstraction
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    //! Lock free, fixed size circular queue TLSSyncSystem tasks queue
    struct TLSSyncQueue
    {
        using ThreadIndex = TLSValue<uint64_t, 0, TLSSyncQueue>;
        using TObject     = ITLSSyncTask;
        static constexpr size_t Size = CTLSSyncSystem_QueueSize;
        static constexpr size_t Mask = Size - 1;
        static_assert( ( Size & Mask ) == 0, "TLSSyncQueue size must be a power of 2" );

        TLSSyncQueue() noexcept
        {
            for( size_t i = 0; i < Size; ++i )    
            {
                Items[ i ].store_relaxed( nullptr );
            }
        }
        ~TLSSyncQueue() noexcept 
        {
            Clear();
        }

        //! Initialize queue for the calling thread
        static void TLSInitialize() noexcept
        {
            ThreadIndex::SetValue( 0 );
        }

        //! Clear all tasks
        void Clear() noexcept
        {
            for( size_t i = 0; i < Size; ++i )    
            {
                auto* TaskPtr{ Items[ i ].exchange( nullptr ) };
                if( nullptr != TaskPtr )
                {
                    TSharedPtr<TObject>::Static_Reset( TaskPtr );
                }
            }
        }

        //! Push new global task
        void Push( TObject* InTask ) noexcept
        {
            const uint64_t TaskIndex   { Head.increment() };
            auto*          ShouldBeNull{ Items[ TaskIndex & Mask ].exchange( InTask ) };
            SKL_ASSERT_MSG( nullptr == ShouldBeNull, "To many tasks at once, increase the TLSSync Tasks queue size!" );
        }

        //! Pop tail for the front element for the calling thread
        void TLSPop() noexcept
        {
            const uint64_t TaskIndex      { ThreadIndex::GetValue() };
            auto*          ShouldNotBeNull{ Items[ TaskIndex & Mask ].exchange( nullptr ) };
            SKL_ASSERT( nullptr != ShouldNotBeNull );
        }

        //! Get the front element for the calling thread
        SKL_NODISCARD TObject* TLSFront() noexcept
        {
            const uint64_t TaskIndex{ ThreadIndex::GetValue() };
            return Items[ TaskIndex & Mask ].load_acquire();
        }

        //! Pop next element for the calling thread
        SKL_NODISCARD TObject* TLSNext() noexcept
        {
            const uint64_t TaskIndex{ ThreadIndex::GetValue() + 1 };
            auto*          Result   { Items[ TaskIndex & Mask ].load_acquire() };
            ThreadIndex::SetValue( TaskIndex );
            return Result;
        }

    private:
        std::relaxed_value<uint64_t> Head       { 0 };       //!< Count
        std::relaxed_value<TObject*> Items[Size]{ nullptr }; //!< Items
    };
}