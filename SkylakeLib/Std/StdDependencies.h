//!
//! \file StdDependencies.h
//! 
//! \brief Std library dependencies
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//!
#pragma once

#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <array>
#include <barrier>
#include <bit>
#include <cctype>
#include <cfloat>
#include <cinttypes>
#include <cmath>
#include <concepts>
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
#include <span>
#include <sstream>
#include <stack>
#include <thread>

using TString = std::wstring;

static_assert( sizeof( int ) == sizeof( long ), "Unsupported platform!" );
