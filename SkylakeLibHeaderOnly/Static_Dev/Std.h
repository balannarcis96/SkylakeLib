static_assert( sizeof( void* ) == 8, "64bit build Only!" );

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
#include <utility>
#include <cstdint>
#include <type_traits>
#include <memory>
#include <chrono>
#include <atomic>
#include <concepts>
#include <latch>
#include <bit>
#include <span>

namespace SKL
{
    [[noreturn]] inline void unreachable() noexcept
    {
#if defined(__GNUC__) // GCC, Clang, ICC
        __builtin_unreachable();
#elif defined(_MSC_VER) // MSVC
        __assume( false );
#endif
    }
}