//!
//! \file Memory.h
//! 
//! \brief Global allocation strategies through the MemoryManager
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    namespace DeallocationStrategy
    {
        template<typename T>
        struct DeconstructAndDeallocateStrategy
        {
            SKL_FORCEINLINE void operator()( T* InPtr ) const noexcept
            {
                GDestructNothrow<T>( InPtr );                  
                MemoryManager::Deallocate<sizeof( T )>( InPtr );
            }
        };

        template<typename T>
        struct JustDeallocateStrategy
        {
            SKL_FORCEINLINE void operator()( T* InPtr ) const noexcept
            {
                MemoryManager::Deallocate<sizeof( T )>( InPtr );
            }
        };
    }

    template<typename T>
    using skl_unique_ptr = std::unique_ptr<T, DeallocationStrategy::DeconstructAndDeallocateStrategy<T>>;

    template<typename T>
    using skl_unique_nd_ptr = std::unique_ptr<T, DeallocationStrategy::JustDeallocateStrategy<T>>;

    //!
    //! \brief Allocate new unique object through the MemoryManager
    //!
    template<typename TObject, typename ...TArgs>
    skl_unique_ptr<TObject> MakeUnique( TArgs... Args ) noexcept 
    {
        TObject* Result { nullptr };

        MemoryManager::AllocResult AllocationResult = MemoryManager::Allocate<sizeof(TObject)>();
        if( true == AllocationResult.IsValid() ) SKL_LIKELY
        {
            GConstructNothrow<TObject>( AllocationResult.MemoryBlock, std::forward<TArgs>( Args )... );
            Result = reinterpret_cast<TObject*>( AllocationResult.MemoryBlock );
        }

        return skl_unique_ptr<TObject>{ Result };
    }

    //!
    //! \brief Allocate new unique object through the MemoryManager
    //!
    //! \remarks Destructor will not be called when destroyed
    //!
    template<typename TObject, typename ...TArgs>
    skl_unique_nd_ptr<TObject> MakeUniqueNoDeconstruct( TArgs... Args ) noexcept 
    {
        TObject* Result { nullptr };

        MemoryManager::AllocResult AllocationResult = MemoryManager::Allocate<sizeof(TObject)>();
        if( true == AllocationResult.IsValid() ) SKL_LIKELY
        {
            GConstructNothrow<TObject>( AllocationResult.MemoryBlock, std::forward<TArgs>( Args )... );
            Result = reinterpret_cast<TObject*>( AllocationResult.MemoryBlock );
        }

        return skl_unique_nd_ptr<TObject>{ Result };
    }
    
    //!
    //! \brief Allocate new shared object (raw ptr) through the MemoryManager
    //!
    template<typename TObject, typename ...TArgs>
    TObject* MakeSharedRaw( TArgs... Args ) noexcept 
    {
        using TResultType                  = TSharedPtr<TObject>;
        using TControlBlock                = ControlBlock;
        constexpr size_t CControlBlockSize = sizeof( TControlBlock );
        static_assert( sizeof( TControlBlock ) % 8 == 0 );
        constexpr size_t ToAllocateSize = sizeof( TControlBlock ) + sizeof( TObject );

        TObject* Result { nullptr };

        auto AllocationResult { MemoryManager::Allocate<ToAllocateSize>() };
        if( true == AllocationResult.IsValid() ) SKL_LIKELY
        {
            //Construct the control block
            GConstructNothrow<TControlBlock>( 
                  AllocationResult.MemoryBlock
                , 1 //Start Reference count
                , static_cast<uint32_t>( AllocationResult.MemoryBlockSize ) );

            //Construct the object
            GConstructNothrow<TObject>( 
                    reinterpret_cast<uint8_t*>( AllocationResult.MemoryBlock ) + sizeof( TControlBlock )
                  , std::forward<TArgs>( Args )... );

            Result = reinterpret_cast<TObject*>( reinterpret_cast<uint8_t*>( AllocationResult.MemoryBlock ) + sizeof( TControlBlock ) );
        }

        return Result;
    }

    //!
    //! \brief Allocate new shared object through the MemoryManager
    //!
    template<typename TObject, typename ...TArgs>
    SKL_FORCEINLINE TSharedPtr<TObject> MakeShared( TArgs... Args ) noexcept 
    {
        return { MakeSharedRaw<TObject>( std::forward<TArgs>( Args )... ) };
    }
}