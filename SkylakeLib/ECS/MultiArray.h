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
                GLOG_FATAL( "MultiArrayBase::MultiArrayBase() Failed to allocate array!" );
            }
            else
            {
                const void* MemsetResult{ memset( Array, 0, sizeof( Type ) * Count ) };
                SKL_ASSERT( Array == MemsetResult );
                ( void )MemsetResult;

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

        alignas( SKL_CACHE_LINE_SIZE ) Type* Array{ nullptr };

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

namespace SKL
{
    template<size_t Count, template<typename> typename TPaddingCondition, typename... Types>
    class MultiArrayWithConditionalPadding;

    template<size_t Count, template<typename> typename TPaddingCondition, typename BaseType>
    class MultiArrayWithConditionalPaddingBase
    {
    private:
        using TPCondition = TPaddingCondition<BaseType>;

        static constexpr bool   CIsThisPadded{ TPCondition::ShouldPadd() };
        static constexpr size_t CPaddingSize { CIsThisPadded ? TPCondition::GetPaddingSize() : 0U };

        struct alignas( alignof( BaseType ) ) Type_WithPadding
        {
            Type_WithPadding() noexcept
            {
                memset( PaddingBuffer, 0, CPaddingSize );
                memset( TypeBuffer, 0, sizeof( BaseType ) );
                
                // default construct the base type
                new ( TypeBuffer ) BaseType();
            }
            ~Type_WithPadding() noexcept
            {
                // call destructor of base type
                GetBaseType().~BaseType();
            }

            SKL_NODISCARD SKL_FORCEINLINE BaseType& GetBaseType() noexcept { return *reinterpret_cast<BaseType*>( TypeBuffer ); }
            SKL_NODISCARD SKL_FORCEINLINE const BaseType& GetBaseType() const noexcept { return *reinterpret_cast<const BaseType*>( TypeBuffer ); }

            template<typename T>
            SKL_NODISCARD SKL_FORCEINLINE T& GetPaddingAsType() noexcept 
            { 
                static_assert( sizeof( T ) <= CPaddingSize );
                return *reinterpret_cast<T*>( PaddingBuffer ); 
            }
            template<typename T>
            SKL_NODISCARD SKL_FORCEINLINE const T& GetPaddingAsType() const noexcept 
            { 
                static_assert( sizeof( T ) <= CPaddingSize );
                return *reinterpret_cast<const T*>( PaddingBuffer ); 
            }

        private:
            uint8_t PaddingBuffer[CPaddingSize];
            uint8_t TypeBuffer[sizeof( BaseType )];
        };
        struct alignas( alignof( BaseType ) ) Type_WithoutPadding
        {
            Type_WithoutPadding() noexcept = default;
            ~Type_WithoutPadding() noexcept = default;

            SKL_NODISCARD SKL_FORCEINLINE BaseType& GetBaseType() noexcept { return Type; }
            SKL_NODISCARD SKL_FORCEINLINE const BaseType& GetBaseType() const noexcept { return Type; }

        private:
            BaseType Type{};
        };

        using Type = std::conditional_t<CIsThisPadded, Type_WithPadding, Type_WithoutPadding>;

        SKL_NODISCARD SKL_FORCEINLINE BaseType& GetItem( size_t InIndex ) noexcept
        {
            SKL_ASSERT( InIndex < Count );
            return Array[InIndex].GetBaseType();    
        }
        
        SKL_NODISCARD SKL_FORCEINLINE const BaseType& GetItem( size_t InIndex ) const noexcept
        {
            SKL_ASSERT( InIndex < Count );
            return Array[InIndex].GetBaseType();    
        }
        
        template<typename T>
        SKL_NODISCARD SKL_FORCEINLINE T& GetPaddingAsType( size_t InIndex ) noexcept
        {
            static_assert( CIsThisPadded );
            SKL_ASSERT( InIndex < Count );
            return Array[InIndex].GetPaddingAsType<T>();    
        }
        
        template<typename T>
        SKL_NODISCARD SKL_FORCEINLINE const T& GetPaddingAsType( size_t InIndex ) const noexcept
        {
            static_assert( CIsThisPadded );
            SKL_ASSERT( InIndex < Count );
            return Array[InIndex].GetPaddingAsType<T>();    
        }

        SKL_FORCEINLINE SKL_NODISCARD bool IsValid() const noexcept { return nullptr != Array; }

    protected:
        MultiArrayWithConditionalPaddingBase() noexcept
        {
            Array = reinterpret_cast<Type*>(
                SKL_MALLOC_ALIGNED( sizeof( Type ) * Count, SKL_CACHE_LINE_SIZE )
            );
            if( nullptr == Array )
            {
                GLOG_FATAL( "MultiArrayWithConditionalPaddingBase::MultiArrayWithConditionalPaddingBase() Failed to allocate array!" );
            }
            else
            {
                const void* MemsetResult{ memset( Array, 0, sizeof( Type ) * Count ) };
                SKL_ASSERT( Array == MemsetResult );
                ( void )MemsetResult;

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

        alignas( SKL_CACHE_LINE_SIZE ) Type* Array{ nullptr };

        template<size_t TCount, template<typename> typename TTPaddingCondition, typename... TTypes>
        friend class MultiArrayWithConditionalPadding;
    };
    
    template<size_t Count, template<typename> typename TPaddingCondition, typename... Types>
    class MultiArrayWithConditionalPadding: public MultiArrayWithConditionalPaddingBase<Count, TPaddingCondition, Types>...
    {
    public:
        MultiArrayWithConditionalPadding() noexcept
            : MultiArrayWithConditionalPaddingBase<Count, TPaddingCondition, Types>()... 
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
        SKL_FORCEINLINE Type& GetArrayItem( size_t InIndex ) noexcept
        {
            using TargetType = MultiArrayWithConditionalPaddingBase<Count, TPaddingCondition, Type>;
            TargetType* MyBase{ static_cast<TargetType*>( this ) };
            return MyBase->Array[InIndex].GetBaseType();
        }

        template<typename Type>
        SKL_FORCEINLINE const Type& GetArrayItem( size_t InIndex  ) const noexcept
        {
            using TargetType = MultiArrayWithConditionalPaddingBase<Count, TPaddingCondition, Type>;
            const auto *MyBase{ static_cast<const TargetType*>( this ) };
            return MyBase->Array[InIndex].GetBaseType();
        }
        
        template<typename Type, typename T>
        SKL_FORCEINLINE T& GetArrayItemPaddingAsT( size_t InIndex ) noexcept
        {
            using TargetType = MultiArrayWithConditionalPaddingBase<Count, TPaddingCondition, Type>;
            static_assert( TargetType::CIsThisPadded );
            auto* MyBase{ static_cast<TargetType*>( this ) };
            return MyBase->Array[InIndex].GetPaddingAsType<T>();
        }

        template<typename Type, typename T>
        SKL_FORCEINLINE const T& GetArrayItemPaddingAsT( size_t InIndex ) const noexcept
        {
            using TargetType = MultiArrayWithConditionalPaddingBase<Count, TPaddingCondition, Type>;
            static_assert( TargetType::CIsThisPadded );
            const auto *MyBase{ static_cast<const TargetType*>( this ) };
            return MyBase->Array[InIndex].GetPaddingAsType<T>();
        }

    private:
        template<typename TType, typename ...TTypes>
        bool IsValidImpl() const noexcept
        {
            using TargetType = MultiArrayWithConditionalPaddingBase<Count, TPaddingCondition, TType>;
            const auto *MyBase{ static_cast<const TargetType*>( this ) };
            const auto bLocalIsValid{ MyBase->IsValid() };

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
