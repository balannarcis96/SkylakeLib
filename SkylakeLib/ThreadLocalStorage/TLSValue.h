//!
//! \file TLSValue.h
//! 
//! \brief TLS value wrapper abstractions for the SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    template< typename T, uint32_t TypeIndex = 0, typename TDummy = void, size_t TSizeDummy = 0 >
    struct TLSValue
    {
        static consteval bool GetIsValueType( )
        {
            return sizeof( T ) <= sizeof( void * );
        }
        static constexpr bool IsValueType = GetIsValueType( );

        struct TypeTraits
        {
            using MyT    = T;
            using MyType = TLSValue< T >;
        };

        struct TLSSlot
        {
            TLSSlot( ) noexcept
            {
                TLSIndex = PlatformTLS::AllocTlsSlot( );
                SKL_ASSERT( PlatformTLS::IsValidTlsSlot( TLSIndex ) );

                if constexpr( IsValueType )
                {
                    T Value = { };

                    PlatformTLS::SetTlsValue( TLSIndex, reinterpret_cast< void * >( static_cast< uint64_t >( Value ) ) );
                }
                else
                {
                    PlatformTLS::SetTlsValue( TLSIndex, nullptr );
                }
            }

            ~TLSSlot( ) noexcept
            {
                if( PlatformTLS::IsValidTlsSlot( TLSIndex ) )
                {
                    if constexpr( !IsValueType )
                    {
                        SKL_ASSERT( PlatformTLS::GetTlsValue( TLSIndex ) == nullptr );
                    }

                    PlatformTLS::FreeTlsSlot( TLSIndex );
                }
            }

            template< typename = std::enable_if< IsValueType > >
            SKL_FORCEINLINE T GetValue( ) noexcept
            {
                return static_cast< T >( reinterpret_cast< uint64_t >( PlatformTLS::GetTlsValue( TLSIndex ) ) );
            }

            template< typename = std::enable_if< !IsValueType > >
            SKL_FORCEINLINE T *GetValuePtr( ) noexcept
            {
                return static_cast< T * >( PlatformTLS::GetTlsValue( TLSIndex ) );
            }

            template< typename = std::enable_if< IsValueType > >
            SKL_FORCEINLINE void SetValue( T InValue ) noexcept
            {
                return PlatformTLS::SetTlsValue( TLSIndex, reinterpret_cast< void * >( static_cast< uint64_t >( InValue ) ) );
            }

            template< typename = std::enable_if< !IsValueType > >
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

        template< typename = std::enable_if< IsValueType > >
        SKL_FORCEINLINE static T GetValue( ) noexcept
        {
            return Slot.GetValue( );
        }

        template< typename = std::enable_if< !IsValueType > >
        SKL_FORCEINLINE static T *GetValuePtr( ) noexcept
        {
            return Slot.GetValuePtr( );
        }

        template< typename = std::enable_if< IsValueType > >
        SKL_FORCEINLINE static void SetValue( T InValue ) noexcept
        {
            return Slot.SetValue( InValue );
        }

        template< typename = std::enable_if< !IsValueType > >
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