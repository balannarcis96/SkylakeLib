//!
//! \file TLSValue.h
//! 
//! \brief TLS value wrapper abstraction for the SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    template<typename T, uint32_t TypeIndex = 0, typename TDependentType = void, size_t TDependentSize = 0>
    struct TLSValue
    {
        static constexpr bool bFitsInTLSSlot = sizeof( T ) <= sizeof( void * );
        static constexpr bool bIsClassType   = std::is_class_v<T>;
        //static_assert( !bIsClassType || std::is_nothrow_default_constructible_v<T>, "Class types must be nothrow default constructible" );
        static_assert( bIsClassType || bFitsInTLSSlot, "Non class types must be at most 8bytes" );

        struct TypeTraits
        {
            using MyT    = T;
            using MyType = TLSValue< T >;

            struct WrappedValueType
            {
                MyT     DefaultInstance                                {};    //!< default constructed T
                uint8_t Buffer[ sizeof( void* ) - sizeof( MyT ) +  1 ] { 0 }; //!< buffer to account for sizeof( T ) < 8bytes
            };
        };

        struct TLSSlot
        {
            TLSSlot() noexcept
            {
                TLSIndex = PlatformTLS::AllocTlsSlot( );
                SKL_ASSERT( true == PlatformTLS::IsValidTlsSlot( TLSIndex ) );

                if constexpr( false == bIsClassType )
                {
                    typename TypeTraits::WrappedValueType Value {};

                    //! perform a raw copy
                    PlatformTLS::SetTlsValue( TLSIndex, *reinterpret_cast<void**>( &Value ) ); 
                }
                else
                {
                    PlatformTLS::SetTlsValue( TLSIndex, nullptr );
                }
            }

            ~TLSSlot() noexcept
            {
                if( PlatformTLS::IsValidTlsSlot( TLSIndex ) )
                {
                    if constexpr( bIsClassType )
                    {
                        const auto* TLSValue = reinterpret_cast<T*>( PlatformTLS::GetTlsValue( TLSIndex ) );
                        if( nullptr != TLSValue )
                        {
                            SKL_BREAK();
                        }
            
                        SKL_ASSERT( TLSValue == nullptr );
                    }

                    PlatformTLS::FreeTlsSlot( TLSIndex );
                }
            }

            template<typename = std::enable_if<false == bIsClassType>>
            SKL_FORCEINLINE T GetValue() noexcept
            {
                const void* TLSValue = PlatformTLS::GetTlsValue( TLSIndex );
                return *reinterpret_cast<T*>( &TLSValue );
            }

            template<typename = std::enable_if<true == bIsClassType>>
            SKL_FORCEINLINE T *GetValuePtr() noexcept
            {
                return reinterpret_cast<T*>( PlatformTLS::GetTlsValue( TLSIndex ) );
            }

            template<typename = std::enable_if<false == bIsClassType>>
            SKL_FORCEINLINE void SetValue( T InValue ) noexcept
            {
                if constexpr( sizeof( T ) < sizeof( void* ) )
                {
                    typename TypeTraits::WrappedValueType Temp{};
                    Temp.DefaultInstance = InValue;

                    return PlatformTLS::SetTlsValue( TLSIndex, *reinterpret_cast<void **>( &Temp ) );
                }
                else
                {
                    return PlatformTLS::SetTlsValue( TLSIndex, *reinterpret_cast<void **>( &InValue ) );
                }
            }

            template<typename = std::enable_if<true == bIsClassType>>
            SKL_FORCEINLINE void SetValuePtr( T *InValue ) noexcept
            {
                return PlatformTLS::SetTlsValue( TLSIndex, InValue );
            }

            uint32_t TLSIndex{ PlatformTLS::INVALID_SLOT_ID };

            TLSSlot( const TLSSlot & ) = delete;
            TLSSlot &operator=( const TLSSlot & ) = delete;
            TLSSlot( TLSSlot && )                  = delete;
            TLSSlot &operator=( TLSSlot && ) = delete;
        };

        template<typename = std::enable_if<true == bIsClassType>>
        SKL_FORCEINLINE static T GetValue( ) noexcept
        {
            return Slot.GetValue( );
        }

        template<typename = std::enable_if<false == bIsClassType>>
        SKL_FORCEINLINE static T *GetValuePtr( ) noexcept
        {
            return Slot.GetValuePtr( );
        }

        template<typename = std::enable_if<true == bIsClassType>>
        SKL_FORCEINLINE static void SetValue( T InValue ) noexcept
        {
            return Slot.SetValue( InValue );
        }

        template<typename = std::enable_if<false == bIsClassType>>
        SKL_FORCEINLINE static void SetValuePtr( T *InValue ) noexcept
        {
            return Slot.SetValuePtr( InValue );
        }

    private:
        static inline TLSSlot Slot;

    public:
        TLSValue( )  = delete;
        ~TLSValue( ) = delete;

        TLSValue( const TLSValue & ) = delete;
        TLSValue &operator=( const TLSValue & ) = delete;
        TLSValue( TLSValue && ) = delete;
        TLSValue &operator=( TLSValue && ) = delete;
    };
}