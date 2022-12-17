//!
//! \file SGUID.h
//! 
//! \brief Short GUID abstraction
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    template<typename MyRand = GRand> 
    struct SGUID 
    {
        static_assert( std::is_same_v<MyRand, GRand> 
               || std::is_same_v<MyRand, TRand> 
               || std::is_base_of_v<Squirrel3Rand, MyRand> );

        union
        {
            uint32_t Value;
            struct
            {
                uint8_t B1;
                uint8_t B2;
                uint8_t B3;
                uint8_t B4;
            };
        };

        SGUID( ) noexcept :
            Value{ 0 }
        {
        }
        SGUID( const SGUID &Other ) noexcept :
            Value{ Other.Value }
        {
        }
        SGUID( uint32_t Value ) noexcept :
            Value{ Value }
        {
        }

        SKL_FORCEINLINE SGUID &operator=( const SGUID &Other ) noexcept
        {
            Value = Other.Value;

            return *this;
        }

        SKL_FORCEINLINE bool operator==( const SGUID &Other ) const noexcept
        {
            return Value == Other.Value;
        }

        SKL_FORCEINLINE bool operator!=( const SGUID &Other ) const noexcept
        {
            return Value != Other.Value;
        }

        SKL_FORCEINLINE SKL_NODISCARD bool IsNone() const noexcept
        {
            return Value == None.Value;
        }

        SKL_FORCEINLINE SKL_NODISCARD uint32_t GetRaw() const noexcept
        {
            return Value;
        }

        SKL_NODISCARD static SGUID New() noexcept
        {
            SGUID SGuid;

            SGuid.B1 = static_cast<uint8_t>( MyRand::NextRandom() % UINT8_MAX );
            SGuid.B2 = static_cast<uint8_t>( MyRand::NextRandom() % UINT8_MAX );
            SGuid.B3 = static_cast<uint8_t>( MyRand::NextRandom() % UINT8_MAX );
            SGuid.B4 = static_cast<uint8_t>( MyRand::NextRandom() % UINT8_MAX );

            return SGuid;
        }

        SKL_FORCEINLINE SKL_NODISCARD static SGUID NewSimple() noexcept
        {
            return static_cast< uint32_t >( MyRand::NextRandom() );
        }

        SKL_FORCEINLINE SKL_NODISCARD std::string ToString() const noexcept
        {
            char TempBuffer[ 32 ];

            if( !_snprintf_s( TempBuffer
                            , 32
                            , 32
                            , "%02X%02X_%02X%02X"
                            , static_cast<int32_t>( B1 )
                            , static_cast<int32_t>( B2 )
                            , static_cast<int32_t>( B3 )
                            , static_cast<int32_t>( B4 ) ) )
            {
                return L"[SGUID_STR_FAILED]";
            }

            return { TempBuffer };
        }

        SKL_FORCEINLINE SKL_NODISCARD std::wstring ToWString() const noexcept
        {
            wchar_t TempBuffer[ 32 ];

            if( !_snwprintf_s( TempBuffer
                             , 32
                             , 32
                             , L"%02X%02X_%02X%02X"
                             , static_cast<int32_t>( B1 )
                             , static_cast<int32_t>( B2 )
                             , static_cast<int32_t>( B3 )
                             , static_cast<int32_t>( B4 ) ) )
            {
                return L"[SGUID_WSTR_FAILED]";
            }

            return { TempBuffer };
        }

        static const SGUID None;
    };

    // Requires TRand to be a valid TLS singleton ( initialized )
    using TLSGUID = SGUID<TRand>;
}