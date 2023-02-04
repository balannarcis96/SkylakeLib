//!
//! \file StdDependencies.h
//! 
//! \brief Std library dependencies
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//!
#pragma once

using TString = std::wstring;

static_assert( sizeof( int ) == sizeof( long ), "Unsupported platform!" );

namespace std
{
    template<>
    struct hash<std::pair<int32_t, int32_t>>
    {
        SKL_FORCEINLINE SKL_NODISCARD size_t operator()( const std::pair<int32_t, int32_t>& Val ) const noexcept
        {
            return reinterpret_cast<const hash<uint64_t>*>( this )->operator ()( *reinterpret_cast<const uint64_t*>( &Val ) );
        }
    };
}
