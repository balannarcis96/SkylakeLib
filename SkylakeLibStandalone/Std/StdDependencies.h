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

using TString = std::wstring;

static_assert( sizeof( int ) == sizeof( long ), "Unsupported platform!" );
