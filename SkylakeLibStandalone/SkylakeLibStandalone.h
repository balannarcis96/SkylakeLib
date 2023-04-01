//!
//! \file SkylakeLibStandalone.h
//! 
//! \brief Standalone part of SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#ifndef SKYLAKELIB_STANDALONG_H
#define SKYLAKELIB_STANDALONG_H

//! The standalone part
#include "../SkylakeLibHeaderOnly/Static_Dev/SkylakeLibHeaderOnly.h"

//#define SKL_MEM_MANAGER_DECAY_TO_GLOBAL

//! Std
#include "Std/Std.h"

//! Log
#include "Diagnostics/Log.h"

//! Tuning
#include "Tuning/Tuning.h"

//! Skylake Random
#include "Utils/SRand.h"

//! Skylake SpinLock
#include "Utils/SpinLock.h"

//! String StringUtils
#include "Utils/StringUtils.h"

//! ECS
#include "ECS/ECS.h"

//! magic_enum_ex library
#if defined(SKL_MAGIC_ENUM)
#include "Utils/magic_enum_ex.hpp"
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

#endif