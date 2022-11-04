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
        MultiArrayBase() noexcept
        {
            Array = reinterpret_cast<Type*>(
                SKL_MALLOC_ALIGNED( sizeof( Type ) * Count, SKL_CACHE_LINE_SIZE )
            );
            if( nullptr == Array )
            {
                SKLL_WRN( "MultiArrayBase::MultiArrayBase() Failed to allocate array!" );
            }
            else
            {
                SKL_ASSERT( Array == memset( Array, 0, sizeof( Type ) * Count ) );

                for( size_t i = 0; i < Count; ++i )
                {
                    if constexpr( std::is_nothrow_default_constructible_v<Type> )
                    {
                        GConstructNothrow<Type>( &Array[i] );
                    }
                    else
                    {
                        GConstruct<Type>( &Array[i] );
                    }
                }
            }
        }

        Type* alignas( SKL_CACHE_LINE_SIZE ) Array{ nullptr };

        template<size_t TCount, typename... TTypes>
        friend class MultiArray;
    };
    
    template<size_t Count, typename... Types>
    class MultiArray: public MultiArrayBase<Count, Types>...
    {
    public:
        MultiArray() noexcept
            : MultiArrayBase<Count, Types>()... 
        {
            bIsValid = IsValidImpl<Types...>();
        }

        //! Are all arrays valid and ready to use
        SKL_FORCEINLINE bool IsValid() const noexcept
        {
            return bIsValid;
        }

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

    private:
        template<typename TType, typename ...TTypes>
        bool IsValidImpl() const noexcept
        {
            const auto bLocalIsValid{ nullptr != GetArray<TType>() };

            if constexpr( 0 == sizeof...( TTypes ) )
            {
                return bLocalIsValid;
            }
            else
            {
                if( false == bLocalIsValid )
                {
                    return false;
                }

                return IsValidImpl<TTypes...>();
            }
        }
    
        bool    bIsValid  { false };
        uint8_t Padding[7]{ 0 };
    };
}