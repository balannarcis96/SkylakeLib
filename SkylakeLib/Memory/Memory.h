//!
//! \file Memory.h
//! 
//! \brief Memory abstractions
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

#include <mimalloc.h>

#define SKL_MALLOC( InSize ) mi_malloc( InSize )
#define SKL_MALLOC_ALIGNED( InSize, InAlignemnt ) mi_malloc_aligned( InSize, InAlignemnt )

#define SKL_FREE( InPtr ) mi_free( InPtr )
#define SKL_FREE_ALIGNED( InPtr, InAlignemnt ) mi_free_aligned( InPtr, InAlignemnt )
#define SKL_FREE_SIZE_ALIGNED( InPtr, InSize, InAlignemnt ) mi_free_size_aligned( InPtr, InSize, InAlignemnt )

namespace SKL
{
    template<size_t BlockSize>
    using MemoryBlock = std::array<uint8_t, BlockSize>;

	template<typename T>
	SKL_FORCEINLINE constexpr void GDestruct( T* Ptr ) noexcept
	{
		Ptr->~T( );
	}

	template<typename T>
	SKL_FORCEINLINE constexpr void GDestructNothrow( T* Ptr ) noexcept
	{
		static_assert( std::is_nothrow_destructible_v<T>, "T::~T() must be noexcept!" );
		Ptr->~T( );
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
