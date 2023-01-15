//!
//! \file Memory.h
//! 
//! \brief Memory abstractions
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    template<size_t BlockSize>
    using MemoryBlock = std::array<uint8_t, BlockSize>;

    template<typename T>
    SKL_FORCEINLINE constexpr void GDestruct( T* Ptr ) noexcept
    {
        Ptr->~T();
    }

    template<typename T>
    SKL_FORCEINLINE constexpr void GDestructNothrow( T* Ptr ) noexcept
    {
        static_assert( std::is_nothrow_destructible_v<T>, "T::~T() must be noexcept!" );
        Ptr->~T();
    }

    template<typename T, typename ...TArgs>
    SKL_FORCEINLINE constexpr void GConstruct( void* Ptr, TArgs... Args ) noexcept
    {
        if constexpr( sizeof...( TArgs ) == 0 )
        {
            new ( Ptr ) T();
        }
        else
        {
            new ( Ptr ) T( std::forward<TArgs>( Args )... );
        }
    }

    template<typename T, typename ...TArgs>
    SKL_FORCEINLINE constexpr void GConstructNothrow( void* Ptr, TArgs... Args ) noexcept
    {
        if constexpr( sizeof...( TArgs ) == 0 )
        {
            static_assert( std::is_nothrow_default_constructible_v<T>, "T::T() must be noexcept!" );

            new ( Ptr ) T();
        }
        else
        {
            static_assert( std::is_nothrow_constructible_v<T, TArgs...>, "T::T(TArgs...) must be noexcept!" );

            new ( Ptr ) T( std::forward<TArgs>( Args )... );
        }
    }

    template<size_t Alignment, typename T, typename ...TArgs>
    SKL_FORCEINLINE SKL_NODISCARD T* GCppAllocAlignedNoThrow( TArgs ... Args ) noexcept
    {
        void* Block{ SKL_MALLOC_ALIGNED( sizeof( T ), Alignment ) };
        if( nullptr != Block ) SKL_LIKELY 
        {
            GConstructNothrow<T>( Block, std::forward<TArgs>( Args )... );
        }

        return reinterpret_cast<T*>( Block );
    }

    template<size_t Alignment, typename T, typename ...TArgs>
    SKL_FORCEINLINE SKL_NODISCARD T* GCppAllocAligned( TArgs ... Args )
    {
        void* Block{ SKL_MALLOC_ALIGNED( sizeof( T ), Alignment ) };
        if( nullptr != Block ) SKL_LIKELY 
        {
            GConstruct<T>( Block, std::forward<TArgs>( Args )... );
        }

        return reinterpret_cast<T*>( Block );
    }

    template<size_t Alignment, typename T>
    SKL_FORCEINLINE void GCppDeleteAlignedNoThrow( T* InObject ) noexcept
    {
        SKL_ASSERT( nullptr != InObject );

        GDestructNothrow( InObject );

        SKL_FREE_SIZE_ALIGNED( InObject, sizeof( T ), Alignment );
    }

    template<size_t Alignment, typename T>
    SKL_FORCEINLINE void GCppDeleteAligned( T* InObject )
    {
        SKL_ASSERT( nullptr != InObject );

        GDestruct( InObject );

        SKL_FREE_SIZE_ALIGNED( InObject, sizeof( T ), Alignment );
    }

    template<size_t Alignment, typename T>
    struct CppNoThrowAlignedGlobalDeleter
    {
        SKL_FORCEINLINE void operator()( T* InPtr ) const noexcept
        {
            GCppDeleteAlignedNoThrow<Alignment>( InPtr );
        }
    };
    
    template<size_t Alignment, typename T>
    struct CppAlignedGlobalDeleter
    {
        SKL_FORCEINLINE void operator()( T* InPtr ) const
        {
            GCppDeleteAligned<Alignment>( InPtr );
        }
    };
}

namespace std
{
    template<typename T, size_t Alignment = alignof( T ), bool bNoThrow = true>
    using aligned_unique_ptr = unique_ptr<T, std::conditional_t<bNoThrow, ::SKL::CppNoThrowAlignedGlobalDeleter<Alignment, T>, ::SKL::CppAlignedGlobalDeleter<Alignment, T>>>;
    
    template<typename T, bool bNoThrow = true>
    using cacheline_unique_ptr = aligned_unique_ptr<T, SKL_CACHE_LINE_SIZE, bNoThrow>; 
    
    template<typename T, size_t Alignment = alignof( T ), typename ...TArgs>
    SKL_FORCEINLINE SKL_NODISCARD aligned_unique_ptr<T, Alignment, true> make_unique_aligned( TArgs ... Args ) noexcept
    {
        return aligned_unique_ptr<T, Alignment, true>{ ::SKL::GCppAllocAlignedNoThrow<Alignment, T>( std::forward<TArgs>( Args )... ) };
    }
    
    template<typename T, size_t Alignment = alignof( T ), typename ...TArgs>
    SKL_FORCEINLINE SKL_NODISCARD aligned_unique_ptr<T, Alignment, false> make_unique_aligned_throw( TArgs ... Args )
    {
        return aligned_unique_ptr<T, Alignment, false>{ ::SKL::GCppAllocAligned<Alignment, T>( std::forward<TArgs>( Args )... ) };
    }

    template<typename T, typename ...TArgs>
    SKL_FORCEINLINE SKL_NODISCARD cacheline_unique_ptr<T, true> make_unique_cacheline( TArgs ... Args ) noexcept
    {
        return cacheline_unique_ptr<T, true>{ ::SKL::GCppAllocAlignedNoThrow<SKL_CACHE_LINE_SIZE, T>( std::forward<TArgs>( Args )... ) };
    }
    
    template<typename T, typename ...TArgs>
    SKL_FORCEINLINE SKL_NODISCARD cacheline_unique_ptr<T, false> make_unique_cacheline_throw( TArgs ... Args )
    {
        return cacheline_unique_ptr<T, false>{ ::SKL::GCppAllocAligned<SKL_CACHE_LINE_SIZE, T>( std::forward<TArgs>( Args )... ) };
    }
}

#include "StaticObjectPool.h"
#include "LocalObjectPool.h"
#include "LocalMemoryManager.h"
#include "GlobalMemoryManagement.h"
#include "ThreadMemoryManagement.h"

#include "MemoryPolicy.h"
#include "SharedPointer.h"
#include "AllocationStrategies.h"
#include "STLAllocator.h"
