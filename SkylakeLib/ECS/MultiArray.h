//!
//! \file MultiArray.h
//! 
//! \brief Multi Array abstraction
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
	template<size_t Count, typename... Types>
	class MultiArray;

	template<size_t Count, typename Type>
	class MultiArrayBase
	{
	protected:
		Type alignas( SKL_CACHE_LINE_SIZE ) Array[ Count ]{};

		template<size_t TCount, typename... TTypes>
		friend class MultiArray;
	};

	template<size_t Count, typename... Types>
	class MultiArray: public MultiArrayBase<Count, Types>...
	{
	protected:
		template<typename Type>
		SKL_FORCEINLINE Type *GetArray( ) noexcept
		{
			using TargetType = MultiArrayBase<Count, Type>;
			auto *MyBase{ static_cast<TargetType*>( this ) };
			return MyBase->Array;
		}

		template<typename Type>
		SKL_FORCEINLINE const Type *GetArray( ) const noexcept
		{
			using TargetType = MultiArrayBase<Count, Type>;
			auto *MyBase{ static_cast<const TargetType*>( this ) };
			return MyBase->Array;
		}
	};
}