//!
//! \file SkylakeLibHeaderOnly.h
//! 
//! \brief All in one SkylakeLibHeaderOnly
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

//! Flags
#include "Flags.h"

//! std dependencies
#include "Std.h"

//! ASD (Advanced Single Dispatch)
#include "AdvancedSingleDispatch.h"
#include "ASDForward.h"

//! Macros
#include "Macros.h"

//! Result Status abstraction
#include "RStatus.h"

//! Base Types
#include "SkylakeLibTypes.h"

#if !defined(SKL_BUILD_SHIPPING)
#include "DebugTrap.h"
#define SKL_BREAK() psnip_trap()
#else
#define SKL_BREAK() 
#endif

//! Assert 
#include "SkylakeAssert.h"

//! Atomic value
#include "AtomicValue.h"

//! Stream
#include "Stream.h"

//! EntityId
#include "EntityId.h"

//! Packet
#include "Packet.h"

//! Logger
#include "Logger.h"

//! Packet Builder
#include "PacketBuilder.h"

namespace SKL
{
    //! Get the current epoch time (milliseconds since 01/01/1970)
    SKL_FORCEINLINE SKL_NODISCARD inline TEpochTimePoint GetCurrentEpochTime() noexcept
    {
        return static_cast<TEpochTimePoint>( std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now().time_since_epoch() ).count() );
    }
    
    //! Returns true if Value is a power of two
    SKL_FORCEINLINE SKL_NODISCARD inline constexpr bool IsPowerOfTwo( auto Value ) noexcept
    {
        static_assert( std::is_integral_v<decltype( Value )> );
        return ( ( Value & ( Value - 1 ) ) == 0 );
    }
}

//! magic_enum_ex library
#if defined(SKL_MAGIC_ENUM)
#include "magic_enum_ex.h"
namespace SKL
{
    template<auto EnumValue>
    consteval const char* EnumToString() noexcept
    {
        return magic_enum_ex::enum_name( EnumValue ).data();
    }
    
    template<auto EnumValue>
    consteval const wchar_t* EnumToStringW() noexcept
    {
        return magic_enum_ex::enum_name_w( EnumValue ).data();
    }
    
    constexpr const char* EnumToString( auto EnumValue ) noexcept
    {
        return magic_enum_ex::enum_name( EnumValue ).data();
    }
    
    constexpr const wchar_t* EnumToStringW( auto EnumValue ) noexcept
    {
        return magic_enum_ex::enum_name_w( EnumValue ).data();
    }

    template<typename TEnum, TEnum Max = TEnum::Max>
    constexpr std::optional<TEnum> EnumFromStringSafe( const char* InString ) noexcept
    {
        return magic_enum_ex::enum_cast<TEnum>( InString );
    }

    template<typename TEnum, TEnum Max = TEnum::Max>
    constexpr TEnum EnumFromString( const char* InString ) noexcept
    {
        return magic_enum_ex::enum_cast<TEnum>( InString ).value_or( Max );
    }
    
    template<typename TEnum, TEnum Max = TEnum::Max>
    constexpr std::pair<TEnum, bool> EnumFromStringWSafe( const wchar_t* InString ) noexcept
    {
        return magic_enum_ex::enum_cast<TEnum>( InString );
    }

    template<typename TEnum, TEnum Max = TEnum::Max>
    constexpr TEnum EnumFromStringW( const wchar_t* InString ) noexcept
    {
        return magic_enum_ex::enum_cast<TEnum>( InString ).value_or( Max );
    }
}
#endif

#if defined(SKL_NO_NAMESPACE)
#ifndef SKL_NO_NAMESPACE_STATEMENT
#define SKL_NO_NAMESPACE_STATEMENT
    using namespace SKL;
#endif SKL_NO_NAMESPACE_STATEMENT
#endif
