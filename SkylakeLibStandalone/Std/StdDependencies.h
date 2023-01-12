//!
//! \file StdDependencies.h
//! 
//! \brief Std library dependencies
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//!
#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <array>
#include <cctype>
#include <cfloat>
#include <cinttypes>
#include <cmath>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cwctype>
#include <deque>
#include <fvec.h>
#include <iomanip>
#include <locale>
#include <variant>
#include <optional>
#include <queue>
#include <shared_mutex>
#include <sstream>
#include <istream>
#include <fstream>
#include <ostream>
#include <stack>
#include <thread>
#include <limits>
#include <filesystem>

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