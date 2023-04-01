#pragma once

#include <array>
#include <string_view>
#include <cassert>
#include <type_traits>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>

// Enum value must be greater or equals than MAGIC_ENUM_RANGE_MIN. By default MAGIC_ENUM_RANGE_MIN = -128.
// If need another min range for all enum types by default, redefine the macro MAGIC_ENUM_RANGE_MIN.
#if !defined(MAGIC_ENUM_RANGE_MIN)
#  define MAGIC_ENUM_RANGE_MIN -128
#endif

// Enum value must be less or equals than MAGIC_ENUM_RANGE_MAX. By default MAGIC_ENUM_RANGE_MAX = 2048.
// If need another max range for all enum types by default, redefine the macro MAGIC_ENUM_RANGE_MAX.
#if !defined(MAGIC_ENUM_RANGE_MAX)
#  define MAGIC_ENUM_RANGE_MAX 2048
#endif

// Improve ReSharper C++ intellisense performance with builtins, avoiding unnecessary template instantiations.
#if defined(__RESHARPER__)
#  undef MAGIC_ENUM_GET_ENUM_NAME_BUILTIN
#  undef MAGIC_ENUM_GET_TYPE_NAME_BUILTIN
#  if __RESHARPER__ >= 20230100
#    define MAGIC_ENUM_GET_ENUM_NAME_BUILTIN(V) __rscpp_enumerator_name(V)
#    define MAGIC_ENUM_GET_TYPE_NAME_BUILTIN(T) __rscpp_type_name<T>()
#  else
#    define MAGIC_ENUM_GET_ENUM_NAME_BUILTIN(V) nullptr
#    define MAGIC_ENUM_GET_TYPE_NAME_BUILTIN(T) nullptr
#  endif
#endif

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunknown-warning-option"
#  pragma clang diagnostic ignored "-Wenum-constexpr-conversion"
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wmaybe-uninitialized" // May be used uninitialized 'return {};'.
#elif defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable : 26495) // Variable 'static_string<N>::chars_' is uninitialized.
#  pragma warning(disable : 28020) // Arithmetic overflow: Using operator '-' on a 4 byte value and then casting the result to a 8 byte value.
#  pragma warning(disable : 26451) // The expression '0<=_Param_(1)&&_Param_(1)<=1-1' is not true at this call.
#  pragma warning(disable : 4514) // Unreferenced inline function has been removed.
#endif

// Checks magic_enum_ex compiler compatibility.
#if defined(__clang__) && __clang_major__ >= 5 || defined(__GNUC__) && __GNUC__ >= 9 || defined(_MSC_VER) && _MSC_VER >= 1910 || defined(__RESHARPER__)
#  undef  MAGIC_ENUM_SUPPORTED
#  define MAGIC_ENUM_SUPPORTED 1
#endif

#if defined(__GNUC__)
#define MAGIC_ENUM_EX_NOINLINE    [[gnu::noinline]]
#elif defined(__clang__)
#define MAGIC_ENUM_EX_NOINLINE    [[clang::noinline]]
#elif defined(_MSC_VER) || defined(__INTEL_COMPILER)
#define MAGIC_ENUM_EX_NOINLINE    [[msvc::noinline]]
#else
#define MAGIC_ENUM_EX_NOINLINE __attribute__((noinline))
#endif

namespace magic_enum_ex
{
    namespace details
    {
        template <auto V, typename E = std::decay_t<decltype( V )>, std::enable_if_t<std::is_enum_v<E>, int> = 0>
        using enum_constant = std::integral_constant<E, V>;

        // Enum value must be in range [MAGIC_ENUM_RANGE_MIN, MAGIC_ENUM_RANGE_MAX]. By default MAGIC_ENUM_RANGE_MIN = -128, MAGIC_ENUM_RANGE_MAX = 128.
        // If need another range for all enum types by default, redefine the macro MAGIC_ENUM_RANGE_MIN and MAGIC_ENUM_RANGE_MAX.
        // If need another range for specific enum type, add specialization enum_range for necessary enum type.
        template <typename E>
        struct enum_range {
            static_assert( std::is_enum_v<E>, "magic_enum::customize::enum_range requires enum type." );
            static constexpr int min = MAGIC_ENUM_RANGE_MIN;
            static constexpr int max = MAGIC_ENUM_RANGE_MAX;
            static_assert( max > min, "magic_enum::customize::enum_range requires max > min." );
        };

        static_assert( MAGIC_ENUM_RANGE_MAX > MAGIC_ENUM_RANGE_MIN, "MAGIC_ENUM_RANGE_MAX must be greater than MAGIC_ENUM_RANGE_MIN." );
        static_assert( ( MAGIC_ENUM_RANGE_MAX - MAGIC_ENUM_RANGE_MIN ) < ( std::numeric_limits<std::uint16_t>::max )( ), "MAGIC_ENUM_RANGE must be less than UINT16_MAX." );

        template <typename T>
        struct supported
            #if defined(MAGIC_ENUM_SUPPORTED) && MAGIC_ENUM_SUPPORTED || defined(MAGIC_ENUM_NO_CHECK_SUPPORT)
            : std::true_type {};
        #else
            : std::false_type {};
        #endif

        template <typename T, typename = void>
        struct has_is_flags : std::false_type {};

        template <typename T>
        struct has_is_flags<T, std::void_t<decltype( enum_range<T>::is_flags )>> : std::bool_constant<std::is_same_v<bool, std::decay_t<decltype( enum_range<T>::is_flags )>>> {};

        template <typename... T>
        inline constexpr bool always_false_v = false;

        template <typename T, typename = void>
        struct range_min : std::integral_constant<int, MAGIC_ENUM_RANGE_MIN> {};

        template <typename T>
        struct range_min<T, std::void_t<decltype( enum_range<T>::min )>> : std::integral_constant<decltype( enum_range<T>::min ), enum_range<T>::min> {};

        template <typename T, typename = void>
        struct range_max : std::integral_constant<int, MAGIC_ENUM_RANGE_MAX> {};

        template <typename T>
        struct range_max<T, std::void_t<decltype( enum_range<T>::max )>> : std::integral_constant<decltype( enum_range<T>::max ), enum_range<T>::max> {};

        enum class customize_tag
        {
            default_tag,
            invalid_tag,
            custom_tag
        };

        class customize_t : public std::pair<customize_tag, std::string_view>
        {
        public:
            constexpr customize_t(std::string_view srt)
                : std::pair<customize_tag, std::string_view> { customize_tag::custom_tag, srt } {}

            constexpr customize_t(const char * srt)
                : customize_t { std::string_view{ srt } } {}

            constexpr customize_t(customize_tag tag)
                : std::pair<customize_tag, std::string_view> { tag, std::string_view{} }
            {
                assert(tag != customize_tag::custom_tag);
            }
        };

        // Default customize.
        inline constexpr auto default_tag = customize_t { customize_tag::default_tag };
        // Invalid customize.
        inline constexpr auto invalid_tag = customize_t { customize_tag::invalid_tag };

        template <typename str_view, std::uint16_t N>
        class static_string_base {
        public:
            static_assert( std::is_same_v<str_view, std::string_view> || std::is_same_v<str_view, std::wstring_view> );
            using Elem = typename str_view::value_type;
            using other_sv = std::conditional_t<std::is_same_v<str_view, std::string_view>, std::wstring_view, std::string_view>;

            constexpr explicit static_string_base(str_view str) noexcept
                : static_string_base { str, std::make_integer_sequence<std::uint16_t, N>{} }
            {
                assert(str.size( ) == N);
            }

            constexpr explicit static_string_base(other_sv str) noexcept
                : static_string_base { str, std::make_integer_sequence<std::uint16_t, N>{} }
            {
                assert(str.size( ) == N);
            }

            constexpr const Elem* data( ) const noexcept { return chars_; }

            constexpr std::uint16_t size( ) const noexcept { return N; }

            constexpr operator str_view( ) const noexcept { return { data( ), size( ) }; }

            constexpr str_view view( ) const noexcept { return { data( ), size( ) }; }

        private:
            template <std::uint16_t... I>
            constexpr static_string_base(str_view str, std::integer_sequence<std::uint16_t, I...>) noexcept : chars_ { str[ I ]..., Elem{ '\0' } } {}

            template <std::uint16_t... I>
            constexpr static_string_base(other_sv str, std::integer_sequence<std::uint16_t, I...>) noexcept : chars_ { static_cast< Elem >( str[ I ] )..., Elem{ '\0' } } {}

            Elem chars_[ static_cast< std::size_t >( N ) + 1 ];
        };

        template <>
        class static_string_base<std::string_view, 0> {
        public:
            constexpr explicit static_string_base( ) = default;

            constexpr explicit static_string_base(std::string_view) noexcept {}

            constexpr const char* data( ) const noexcept { return nullptr; }

            constexpr std::uint16_t size( ) const noexcept { return 0; }

            constexpr operator std::string_view( ) const noexcept { return {}; }
        };

        template <>
        class static_string_base<std::wstring_view, 0> {
        public:
            constexpr explicit static_string_base( ) = default;

            constexpr explicit static_string_base(std::wstring_view) noexcept {}

            constexpr const wchar_t* data( ) const noexcept { return nullptr; }

            constexpr std::uint16_t size( ) const noexcept { return 0; }

            constexpr operator std::wstring_view( ) const noexcept { return {}; }
        };

        template <std::uint16_t N>
        using static_string = static_string_base<std::string_view, N>;

        template <std::uint16_t N>
        using wstatic_string = static_string_base<std::wstring_view, N>;

        template<typename str_view, typename Op = std::equal_to<>>
        class case_insensitive
        {
            static_assert( std::is_same_v<str_view, std::string_view> || std::is_same_v<str_view, std::wstring_view> );
            using Elem = typename str_view::value_type;

            static consteval Elem to_lower(Elem c) noexcept
            {
                return ( c >= Elem { 'A' } && c <= Elem { 'Z' } ) ? static_cast< Elem >( c + ( Elem { 'a' } - Elem { 'A' } ) ) : c;
            }

        public:
            template <typename L, typename R>
            consteval auto operator()([[maybe_unused]] L lhs, [[maybe_unused]] R rhs) const noexcept -> std::enable_if_t<std::is_same_v<std::decay_t<L>, Elem> && std::is_same_v<std::decay_t<R>, Elem>, bool>
            {
                #if defined(MAGIC_ENUM_ENABLE_NONASCII)
                static_assert( always_false_v<L, R>, "magic_enum_ex::case_insensitive not supported Non-ASCII feature." );
                return false;
                #else
                return Op {}( to_lower(lhs), to_lower(rhs) );
                #endif
            }
        };
    }

    // If need custom names for enum, add specialization enum_name for necessary enum type.
    template <typename E, E V>
    consteval details::customize_t customize_enum_value_name( ) noexcept { return details::default_tag; }

    // If need custom type name for enum, add specialization enum_type_name for necessary enum type.
    template <typename E>
    consteval details::customize_t customize_enum_type_name( ) noexcept { return details::default_tag; }

    // Checks is magic_enum supported compiler.
    inline constexpr bool is_magic_enum_ex_supported = details::supported<void>::value;

    namespace utils
    {
        template <typename T, bool = std::is_enum_v<std::decay_t<T>>>
        struct underlying_type {};

        template <typename T>
        struct underlying_type<T, true> : std::underlying_type<std::decay_t<T>> {};

        template <typename T, bool = std::is_enum_v<T>>
        struct is_scoped_enum : std::false_type {};

        template <typename T>
        struct is_scoped_enum<T, true> : std::bool_constant<!std::is_convertible_v<T, std::underlying_type_t<T>>> {};

        template <typename T, bool = std::is_enum_v<T>>
        struct is_unscoped_enum : std::false_type {};

        template <typename T>
        struct is_unscoped_enum<T, true> : std::bool_constant<std::is_convertible_v<T, std::underlying_type_t<T>>> {};

        template<typename str_view>
        consteval str_view pretty_name(str_view name) noexcept
        {
            using Elem = typename str_view::value_type;

            const Elem* str = name.data( );
            for ( std::size_t i = name.size( ); i > 0; --i ) {
                const Elem c = str[ i - 1 ];
                if ( !( ( c >= Elem { '0' } && c <= Elem { '9' } ) ||
                    ( c >= Elem { 'a' } && c <= Elem { 'z' } ) ||
                    ( c >= Elem { 'A' } && c <= Elem { 'Z' } ) ||
                    #if defined(MAGIC_ENUM_ENABLE_NONASCII)
                    ( c & Elem { 0x80 } ) ||
                    #endif
                    ( c == Elem { '_' } ) ) ) {
                    name.remove_prefix(i);
                    break;
                }
            }

            if ( name.size( ) > 0 ) {
                const Elem c = name[ 0 ];
                if ( ( c >= Elem { 'a' } && c <= Elem { 'z' } ) ||
                    ( c >= Elem { 'A' } && c <= Elem { 'Z' } ) ||
            #if defined(MAGIC_ENUM_ENABLE_NONASCII)
                    ( c & Elem { 0x80 } ) ||
            #endif
                    ( c == Elem { '_' } ) ) {
                    return name;
                }
            }

            return {}; // Invalid name.
        }

        template<typename str_view>
        consteval std::size_t find(str_view str, typename str_view::value_type c) noexcept
        {
            return str.find(c);
        }

        template <typename T, std::size_t N, std::size_t... I>
        consteval std::array<std::remove_cv_t<T>, N> to_array(T(&a)[ N ], std::index_sequence<I...>) noexcept
        {
            return { {a[ I ]...} };
        }

        template <typename str_view, typename BinaryPredicate>
        consteval bool is_default_predicate( ) noexcept
        {
            return std::is_same_v<std::decay_t<BinaryPredicate>, std::equal_to<typename str_view::value_type>> || std::is_same_v<std::decay_t<BinaryPredicate>, std::equal_to<>>;
        }

        template <typename str_view, typename BinaryPredicate = std::equal_to<>>
        constexpr bool cmp_equal(str_view lhs, str_view rhs, [[maybe_unused]] BinaryPredicate&& p = {}) noexcept
        {
            if constexpr ( !is_default_predicate<str_view, BinaryPredicate>( ) )
            {
                if ( lhs.size( ) != rhs.size( ) )
                {
                    return false;
                }

                const auto size = lhs.size( );
                for ( std::size_t i = 0; i < size; ++i )
                {
                    if ( !p(lhs[ i ], rhs[ i ]) )
                    {
                        return false;
                    }
                }

                return true;
            }
            else
            {
                return lhs == rhs;
            }
        }

        template <typename L, typename R>
        consteval bool cmp_less(L lhs, R rhs) noexcept
        {
            static_assert( std::is_integral_v<L> && std::is_integral_v<R>, "magic_enum_ex::detail::cmp_less requires integral type." );

            if constexpr ( std::is_signed_v<L> == std::is_signed_v<R> ) {
                // If same signedness (both signed or both unsigned).
                return lhs < rhs;
            }
            else if constexpr ( std::is_same_v<L, bool> ) { // bool special case
                return static_cast< R >( lhs ) < rhs;
            }
            else if constexpr ( std::is_same_v<R, bool> ) { // bool special case
                return lhs < static_cast< L >( rhs );
            }
            else if constexpr ( std::is_signed_v<R> ) {
                // If 'right' is negative, then result is 'false', otherwise cast & compare.
                return rhs > 0 && lhs < static_cast< std::make_unsigned_t<R> >( rhs );
            }
            else {
                // If 'left' is negative, then result is 'true', otherwise cast & compare.
                return lhs < 0 || static_cast< std::make_unsigned_t<L> >( lhs ) < rhs;
            }
        }

        template <typename I>
        consteval I log2(I value) noexcept
        {
            static_assert( std::is_integral_v<I>, "magic_enum_ex::detail::log2 requires integral type." );

            if constexpr ( std::is_same_v<I, bool> )
            {
            // bool special case
                return assert(false), value;
            }
            else
            {
                auto ret = I { 0 };
                for ( ; value > I { 1 }; value >>= I { 1 }, ++ret ) { }
                return ret;
            }
        }

        template <typename T>
        inline constexpr bool is_enum_v = std::is_enum_v<T> && std::is_same_v<T, std::decay_t<T>>;

        template <typename str_view, typename E>
        consteval auto n( ) noexcept
        {
            static_assert( is_enum_v<E>, "magic_enum_ex::detail::n requires enum type." );

            if constexpr ( details::supported<E>::value )
            {
                #if defined(MAGIC_ENUM_GET_TYPE_NAME_BUILTIN)
                constexpr auto name_ptr = MAGIC_ENUM_GET_TYPE_NAME_BUILTIN(E);
                constexpr auto name = name_ptr ? std::string_view { name_ptr } : std::string_view {};
                #elif defined(__clang__) || defined(__GNUC__)
                constexpr auto name = pretty_name<std::string_view>({ __PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__) - 2 });
                #elif defined(_MSC_VER)
                constexpr auto name = pretty_name<std::string_view>({ __FUNCSIG__, sizeof(__FUNCSIG__) - 17 });
                #else
                #pragma error "unsuported compiler T_T"
                #endif
                return details::static_string_base<str_view, static_cast< std::uint16_t >( name.size( ) )>{ name };
            }
            else {
                return details::static_string_base<str_view, 0>{};
            }
        }

        template <typename str_view, typename E>
        consteval auto type_name( ) noexcept
        {
            static_assert( is_enum_v<E>, "magic_enum_ex::detail::type_name requires enum type." );

            [[maybe_unused]] constexpr auto custom = customize_enum_type_name<E>( );
            static_assert( std::is_same_v<std::decay_t<decltype( custom )>, details::customize_t>, "magic_enum_ex::customize requires customize_t type." );

            if constexpr ( custom.first == details::customize_tag::custom_tag )
            {
                constexpr auto name = custom.second;
                static_assert( !name.empty( ), "magic_enum_ex::customize requires not empty string." );
                return details::static_string_base<str_view, name.size( )>{ name };
            }
            else if constexpr ( custom.first == details::customize_tag::invalid_tag )
            {
                return details::static_string_base<str_view, 0>{};
            }
            else if constexpr ( custom.first == details::customize_tag::default_tag )
            {
                return n<str_view, E>( );
            }
            else {
                static_assert( details::always_false_v<E>, "magic_enum_ex::customize invalid." );
            }
        }

        template <typename str_view, typename E>
        inline constexpr auto type_name_v = type_name<str_view, E>( );

        template <typename str_view, typename E, E V>
        consteval auto n( ) noexcept
        {
            static_assert( is_enum_v<E>, "magic_enum_ex::detail::n requires enum type." );

            if constexpr ( details::supported<E>::value )
            {
                #if defined(MAGIC_ENUM_GET_ENUM_NAME_BUILTIN)
                constexpr auto name_ptr = MAGIC_ENUM_GET_ENUM_NAME_BUILTIN(V);
                constexpr auto name = name_ptr ? std::string_view { name_ptr } : std::string_view {};
                #elif defined(__clang__) || defined(__GNUC__)
                constexpr auto name = pretty_name<std::string_view>({ __PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__) - 2 });
                #elif defined(_MSC_VER)
                constexpr auto name = pretty_name<std::string_view>({ __FUNCSIG__, sizeof(__FUNCSIG__) - 17 });
                #else
                #pragma error "unsuported compiler T_T"
                #endif
                return details::static_string_base<str_view, static_cast< std::uint16_t >( name.size( ) )>{ name };
            }
            else {
                return details::static_string_base<str_view, 0>{};
            }
        }

        template <typename str_view, typename E, E V>
        consteval auto enum_name( ) noexcept
        {
            static_assert( is_enum_v<E>, "magic_enum_ex::detail::enum_name requires enum type." );

            [[maybe_unused]] constexpr auto custom = customize_enum_value_name<E, V>( );
            static_assert( std::is_same_v<std::decay_t<decltype( custom )>, details::customize_t>, "magic_enum_ex::customize requires customize_t type." );

            if constexpr ( custom.first == details::customize_tag::custom_tag )
            {
                constexpr auto name = custom.second;
                static_assert( !name.empty( ), "magic_enum_ex::customize requires not empty string." );
                return details::static_string_base<str_view, name.size( )>{name};
            }
            else if constexpr ( custom.first == details::customize_tag::invalid_tag )
            {
                return details::static_string_base<str_view, 0>{};
            }
            else if constexpr ( custom.first == details::customize_tag::default_tag )
            {
                return n<str_view, E, V>( );
            }
            else
            {
                static_assert( details::always_false_v<E>, "magic_enum_ex::customize invalid." );
            }
        }

        template <typename str_view, typename E, E V>
        inline constexpr auto enum_name_v = enum_name<str_view, E, V>( );

        template <typename E, auto V>
        consteval bool is_valid( ) noexcept
        {
            static_assert( is_enum_v<E>, "magic_enum_ex::detail::is_valid requires enum type." );

            #if defined(__clang__) && __clang_major__ >= 16
              // https://reviews.llvm.org/D130058, https://reviews.llvm.org/D131307
            constexpr E v = __builtin_bit_cast(E, V);
            #else
            constexpr E v = static_cast< E >( V );
            #endif

            [[maybe_unused]] constexpr auto custom = customize_enum_value_name<E, v>( );
            static_assert( std::is_same_v<std::decay_t<decltype( custom )>, details::customize_t>, "magic_enum::customize requires customize_t type." );

            if constexpr ( custom.first == details::customize_tag::custom_tag )
            {
                constexpr auto name = custom.second;
                static_assert( !name.empty( ), "magic_enum_ex::customize requires not empty string." );
                return name.size( ) != 0;
            }
            else if constexpr ( custom.first == details::customize_tag::default_tag )
            {
                return n<std::string_view, E, v>( ).size( ) != 0;
            }
            else
            {
                return false;
            }
        }

        template <typename E, int O, typename U = std::underlying_type_t<E>>
        constexpr U ualue(std::size_t i) noexcept
        {
            static_assert( is_enum_v<E>, "magic_enum_ex::detail::ualue requires enum type." );

            if constexpr ( std::is_same_v<U, bool> )
            { // bool special case
                static_assert( O == 0, "magic_enum_ex::detail::ualue requires valid offset." );

                return static_cast< U >( i );
            }
            else
            {
                return static_cast< U >( static_cast< int >( i ) + O );
            }
        }

        template <typename E, int O, typename U = std::underlying_type_t<E>>
        constexpr E value(std::size_t i) noexcept
        {
            static_assert( is_enum_v<E>, "magic_enum_ex::detail::value requires enum type." );

            return static_cast< E >( ualue<E, O>(i) );
        }

        template <typename E, typename U = std::underlying_type_t<E>>
        consteval int reflected_min( ) noexcept
        {
            static_assert( is_enum_v<E>, "magic_enum_ex::detail::reflected_min requires enum type." );

            constexpr auto lhs = details::range_min<E>::value;
            constexpr auto rhs = ( std::numeric_limits<U>::min )( );

            if constexpr ( cmp_less(rhs, lhs) )
            {
                return lhs;
            }
            else {
                return rhs;
            }
        }

        template <typename E, typename U = std::underlying_type_t<E>>
        consteval int reflected_max( ) noexcept {
            static_assert( is_enum_v<E>, "magic_enum_ex::detail::reflected_max requires enum type." );

            constexpr auto lhs = details::range_max<E>::value;
            constexpr auto rhs = ( std::numeric_limits<U>::max )( );

            if constexpr ( cmp_less(lhs, rhs) ) {
                return lhs;
            }
            else {
                return rhs;
            }
        }

        template <typename E>
        inline constexpr auto reflected_min_v = reflected_min<E>( );

        template <typename E>
        inline constexpr auto reflected_max_v = reflected_max<E>( );

        template <std::size_t N>
        consteval std::size_t values_count(const bool(&valid)[ N ]) noexcept
        {
            auto count = std::size_t { 0 };
            for ( std::size_t i = 0; i < N; ++i )
            {
                if ( valid[ i ] )
                {
                    ++count;
                }
            }

            return count;
        }

        template <typename str_view, typename E, int Min, std::size_t... I>
        consteval auto values(std::index_sequence<I...>) noexcept
        {
            static_assert( is_enum_v<E>, "magic_enum_ex::detail::values requires enum type." );
            constexpr bool valid[ sizeof...( I ) ] = { is_valid<E, ualue<E, Min>(I)>( )... };
            constexpr std::size_t count = values_count(valid);

            if constexpr ( count > 0 )
            {
                E values[ count ] = {};

                for ( std::size_t i = 0, v = 0; v < count; ++i )
                {
                    if ( valid[ i ] )
                    {
                        values[ v++ ] = value<E, Min>(i);
                    }
                }

                return to_array(values, std::make_index_sequence<count>{});
            }
            else {
                return std::array<E, 0>{};
            }
        }

        template <typename str_view, typename E, typename U = std::underlying_type_t<E>>
        constexpr auto values( ) noexcept
        {
            static_assert( is_enum_v<E>, "magic_enum_ex::detail::values requires enum type." );

            constexpr auto min = reflected_min_v<E>;
            constexpr auto max = reflected_max_v<E>;
            constexpr auto range_size = max - min + 1;

            static_assert( range_size > 0, "magic_enum_ex::enum_range requires valid size." );
            static_assert( range_size < ( std::numeric_limits<std::uint16_t>::max )( ), "magic_enum_ex::enum_range requires valid size." );

            return values<str_view, E, min>(std::make_index_sequence<range_size>{});
        }

        template <typename str_view, typename E>
        inline constexpr std::array values_v = values<str_view, E>( );

        template <typename str_view, typename E, typename D = std::decay_t<E>>
        using values_t = decltype( ( values_v<str_view, D> ) );

        template <bool, typename R>
        struct enable_if_enum {};

        template <typename R>
        struct enable_if_enum<true, R>
        {
            using type = R;
            static_assert( details::supported<R>::value, "magic_enum_ex unsupported compiler (https://github.com/Neargye/magic_enum#compiler-compatibility)." );
        };

        template <typename T, typename R, typename BinaryPredicate = std::equal_to<>, typename D = std::decay_t<T>>
        using enable_if_t = typename enable_if_enum<std::is_enum_v<D> &&
            ( std::is_invocable_r_v<bool, BinaryPredicate, char, char> || std::is_invocable_r_v<bool, BinaryPredicate, wchar_t, wchar_t> ), R>::type;

        template <typename T, typename R, typename BinaryPredicate = std::equal_to<>, typename D = std::decay_t<T>>
        using enable_if_default_t = typename enable_if_enum<std::is_enum_v<D> &&
            ( std::is_invocable_r_v<bool, BinaryPredicate, char, char> || std::is_invocable_r_v<bool, BinaryPredicate, wchar_t, wchar_t> ), R>::type;

        template <typename T, std::enable_if_t<std::is_enum_v<std::decay_t<T>>, int> = 0>
        using enum_concept = T;

        template <typename str_view, typename E>
        inline constexpr auto count_v = values_v<str_view, E>.size( );

        template <typename str_view, typename E, typename U = std::underlying_type_t<E>>
        inline constexpr auto min_v = ( count_v<str_view, E> > 0 ) ? static_cast< U >( values_v<str_view, E>.front( ) ) : U { 0 };

        template <typename str_view, typename E, typename U = std::underlying_type_t<E>>
        inline constexpr auto max_v = ( count_v<str_view, E> > 0 ) ? static_cast< U >( values_v<str_view, E>.back( ) ) : U { 0 };

        template <typename str_view, typename E, std::size_t... I>
        consteval auto names(std::index_sequence<I...>) noexcept
        {
            static_assert( is_enum_v<E>, "magic_enum_ex::detail::names requires enum type." );

            return std::array<str_view, sizeof...( I )>{ {enum_name_v<str_view, E, values_v<str_view, E>[ I ]>...}};
        }

        template <typename str_view, typename E>
        inline constexpr std::array names_v = names<str_view, E>(std::make_index_sequence<count_v<str_view, E>>{});

        template <typename str_view, typename E, typename D = std::decay_t<E>>
        using names_t = decltype( ( names_v<str_view, D> ) );

        template <typename str_view, typename E, std::size_t... I>
        consteval auto entries(std::index_sequence<I...>) noexcept
        {
            static_assert( is_enum_v<E>, "magic_enum_ex::detail::entries requires enum type." );

            return std::array<std::pair<E, str_view>, sizeof...( I )>{ { {values_v<str_view, E>[ I ], enum_name_v<str_view, E, values_v<str_view, E>[ I ]>}...}};
        }

        template <typename str_view, typename E>
        inline constexpr std::array entries_v = entries<str_view, E>(std::make_index_sequence<count_v<str_view, E>>{});

        template <typename str_view, typename E, typename D = std::decay_t<E>>
        using entries_t = decltype( ( entries_v<str_view, D> ) );

        template <typename E, typename U = std::underlying_type_t<E>>
        consteval bool is_sparse( ) noexcept
        {
            static_assert( is_enum_v<E>, "magic_enum_ex::detail::is_sparse requires enum type." );

            if constexpr ( count_v<std::string_view, E> == 0 )
            {
                return false;
            }
            else if constexpr ( std::is_same_v<U, bool> )
            {
                // bool special case
                return false;
            }
            else
            {
                constexpr auto max = max_v<std::string_view, E>;
                constexpr auto min = min_v<std::string_view, E>;
                constexpr auto range_size = max - min + 1;
                return range_size != count_v<std::string_view, E>;
            }
        }

        template <typename E>
        inline constexpr bool is_sparse_v = is_sparse<E>( );

        template <typename T>
        inline constexpr bool has_hash = false;

        template <typename str_view, typename E, typename F, std::size_t... I>
        consteval auto for_each(F&& f, std::index_sequence<I...>)
        {
            static_assert( is_enum_v<E>, "magic_enum_ex::detail::for_each requires enum type." );
            constexpr bool has_void_return = ( std::is_void_v<std::invoke_result_t<F, details::enum_constant<values_v<str_view, E>[ I ]>>> || ... );
            constexpr bool all_same_return = ( std::is_same_v<std::invoke_result_t<F, details::enum_constant<values_v<str_view, E>[ 0 ]>>, std::invoke_result_t<F, details::enum_constant<values_v<str_view, E>[ I ]>>> && ... );

            if constexpr ( has_void_return ) {
                ( f(details::enum_constant<values_v<str_view, E>[ I ]>{}), ... );
            }
            else if constexpr ( all_same_return ) {
                return std::array { f(details::enum_constant<values_v<str_view, E>[ I ]>{})... };
            }
            else {
                return std::tuple { f(details::enum_constant<values_v<str_view, E>[ I ]>{})... };
            }
        }

        template <typename str_view, typename E, typename F, std::size_t... I>
        consteval bool all_invocable(std::index_sequence<I...>)
        {
            static_assert( is_enum_v<E>, "magic_enum_ex::detail::all_invocable requires enum type." );

            if constexpr ( count_v<str_view, E> == 0 )
            {
                return false;
            }
            else
            {
                return ( std::is_invocable_v<F, details::enum_constant<values_v<str_view, E>[ I ]>> && ... );
            }
        }
    }

    // If T is a complete enumeration type, provides a member typedef type that names the underlying type of T.
    // Otherwise, if T is not an enumeration type, there is no member type. Otherwise (T is an incomplete enumeration type), the program is ill-formed.
    template <typename T>
    struct underlying_type : utils::underlying_type<T> {};

    template <typename T>
    using underlying_type_t = typename underlying_type<T>::type;

    template <typename T>
    using Enum = utils::enum_concept<T>;

    template <typename T>
    inline constexpr bool is_unscoped_enum_v = utils::is_unscoped_enum<T>::value;

    // Checks whether T is an Scoped enumeration type.
    // Provides the member constant value which is equal to true, if T is an [Scoped enumeration](https://en.cppreference.com/w/cpp/language/enum#Scoped_enumerations) type. Otherwise, value is equal to false.
    template <typename T>
    struct is_scoped_enum : utils::is_scoped_enum<T> {};

    // Returns type name of enum.
    template <typename E>
    [[nodiscard]] consteval auto enum_type_name( ) noexcept -> utils::enable_if_t<E, std::string_view> {
        constexpr auto name = utils::type_name_v<std::string_view, std::decay_t<E>>;
        static_assert( !name.empty( ), "magic_enum_ex::enum_type_name enum type does not have a name." );

        return name;
    }

    // Returns type name of enum.
    template <typename E>
    [[nodiscard]] consteval auto enum_type_name_w( ) noexcept -> utils::enable_if_t<E, std::wstring_view> {
        constexpr auto name = utils::type_name_v<std::wstring_view, std::decay_t<E>>;
        static_assert( !name.empty( ), "magic_enum_ex::enum_type_name enum type does not have a name." );

        return name;
    }

    // Returns number of enum values.
    template <typename E>
    [[nodiscard]] consteval auto enum_count( ) noexcept -> utils::enable_if_t<E, std::size_t>
    {
        return utils::count_v<std::string_view, std::decay_t<E>>;
    }

    // Returns enum value at specified index.
    // No bounds checking is performed: the behavior is undefined if index >= number of enum values.
    template <typename E>
    [[nodiscard]] constexpr auto enum_value(std::size_t index) noexcept -> utils::enable_if_t<E, std::decay_t<E>>
    {
        using D = std::decay_t<E>;

        if constexpr ( utils::is_sparse_v<D> )
        {
            return assert(( index < utils::count_v<std::string_view, D> )), utils::values_v<std::string_view, D>[ index ];
        }
        else {
            constexpr auto min = utils::min_v<std::string_view, D>;
            return assert(( index < utils::count_v<std::string_view, D> )), utils::value<D, min>(index);
        }
    }

    // Returns enum value at specified index.
    template <typename E, std::size_t I>
    [[nodiscard]] constexpr auto enum_value( ) noexcept -> utils::enable_if_t<E, std::decay_t<E>> {
        using D = std::decay_t<E>;
        static_assert( I < utils::count_v<std::string_view, D>, "magic_enum::enum_value out of range." );

        return enum_value<D>(I);
    }

    // Returns std::array with enum values, sorted by enum value.
    template <typename E>
    [[nodiscard]] consteval auto enum_values( ) noexcept -> utils::enable_if_t<E, utils::values_t<std::string_view, E>>
    {
        return utils::values_v<std::string_view, std::decay_t<E>>;
    }

    // Returns integer value from enum value.
    template <typename E>
    [[nodiscard]] consteval auto enum_integer(E value) noexcept -> utils::enable_if_t<E, underlying_type_t<E>>
    {
        return static_cast< underlying_type_t<E> >( value );
    }

    // Returns underlying value from enum value.
    template <typename E>
    [[nodiscard]] consteval auto enum_underlying(E value) noexcept -> utils::enable_if_t<E, underlying_type_t<E>>
    {
        return static_cast< underlying_type_t<E> >( value );
    }

    // Obtains index in enum values from enum value.
    // Returns optional with index.
    template <typename E>
    [[nodiscard]] constexpr auto enum_index([[maybe_unused]] E value) noexcept -> utils::enable_if_t<E, std::optional<std::size_t>>
    {
        using D = std::decay_t<E>;
        using U = underlying_type_t<D>;

        if constexpr ( utils::count_v<std::string_view, D> == 0 )
        {
            return {}; // Empty enum.
        }
        else if constexpr ( utils::is_sparse_v<D> )
        {
            for ( std::size_t i = 0; i < utils::count_v<std::string_view, D>; ++i )
            {
                if ( enum_value<D>(i) == value )
                {
                    return i;
                }
            }

            return {}; // Invalid value or out of range.
        }
        else {
            const auto v = static_cast< U >( value );

            if ( v >= utils::min_v<std::string_view, D> && v <= utils::max_v<std::string_view, D> )
            {
                return static_cast< std::size_t >( v - utils::min_v<std::string_view, D> );
            }

            return {}; // Invalid value or out of range.
        }
    }

    // Obtains index in enum values from static storage enum variable.
    template <auto V>
    [[nodiscard]] consteval auto enum_index( ) noexcept -> utils::enable_if_t<decltype( V ), std::size_t>
    {
        constexpr auto index = enum_index<std::decay_t<decltype( V )>>(V);
        static_assert( index, "magic_enum_ex::enum_index enum value does not have a index." );

        return *index;
    }

    // Returns name from static storage enum variable.
    // This version is much lighter on the compile times and is not restricted to the enum_range limitation.
    template <auto V>
    [[nodiscard]] consteval auto enum_name( ) noexcept -> utils::enable_if_t<decltype( V ), std::string_view>
    {
        constexpr std::string_view name = utils::enum_name_v<std::string_view, std::decay_t<decltype( V )>, V>;
        static_assert( !name.empty( ), "magic_enum_ex::enum_name enum value does not have a name." );

        return name;
    }

    // Returns name from static storage enum variable.
    // This version is much lighter on the compile times and is not restricted to the enum_range limitation.
    template <auto V>
    [[nodiscard]] consteval auto enum_name_w( ) noexcept -> utils::enable_if_t<decltype( V ), std::wstring_view>
    {
        constexpr std::wstring_view name = utils::enum_name_v<std::wstring_view, std::decay_t<decltype( V )>, V>;
        static_assert( !name.empty( ), "magic_enum_ex::enum_name enum value does not have a name." );

        return name;
    }

    // Returns name from enum value.
    // If enum value does not have name or value out of range, returns empty string.
    template <typename E>
    [[nodiscard]] constexpr auto enum_name(E value) noexcept -> utils::enable_if_default_t<E, std::string_view>
    {
        using D = std::decay_t<E>;

        if ( const auto i = enum_index<D>(value) )
        {
            return utils::names_v<std::string_view, D>[ *i ];
        }

        return {};
    }
    
    // Returns name from enum value.
    // If enum value does not have name or value out of range, returns empty string.
    template <typename E>
    [[nodiscard]] constexpr auto enum_name_w(E value) noexcept -> utils::enable_if_default_t<E, std::wstring_view>
    {
        using D = std::decay_t<E>;

        if ( const auto i = enum_index<D>(value) )
        {
            return utils::names_v<std::wstring_view, D>[ *i ];
        }

        return {};
    }

    // Returns std::array with names, sorted by enum value.
    template <typename E>
    [[nodiscard]] constexpr auto enum_names( ) noexcept -> utils::enable_if_t<E, utils::names_t<std::string_view, E>>
    {
        return utils::names_v<std::string_view, std::decay_t<E>>;
    }

    // Returns std::array with names, sorted by enum value.
    template <typename E>
    [[nodiscard]] constexpr auto enum_names_w( ) noexcept -> utils::enable_if_t<E, utils::names_t<std::wstring_view, E>>
    {
        return utils::names_v<std::wstring_view, std::decay_t<E>>;
    }

    // Returns std::array with pairs (value, name), sorted by enum value.
    template <typename E>
    [[nodiscard]] constexpr auto enum_entries( ) noexcept -> utils::enable_if_t<E, utils::entries_t<std::string_view, E>>
    {
        return utils::entries_v<std::string_view, std::decay_t<E>>;
    }

    // Returns std::array with pairs (value, name), sorted by enum value.
    template <typename E>
    [[nodiscard]] constexpr auto enum_entries_w( ) noexcept -> utils::enable_if_t<E, utils::entries_t<std::wstring_view, E>>
    {
        return utils::entries_v<std::wstring_view, std::decay_t<E>>;
    }

    // Allows you to write magic_enum::enum_cast<foo>("bar", magic_enum::case_insensitive);
    inline constexpr auto case_insensitive = details::case_insensitive<std::string_view> {};

    // Allows you to write magic_enum::enum_cast_w<foo>(L"bar", magic_enum::case_insensitive_w);
    inline constexpr auto case_insensitive_w = details::case_insensitive<std::wstring_view> {};

    // Obtains enum value from integer value.
    // Returns optional with enum value.
    template <typename E>
    [[nodiscard]] consteval auto enum_cast(underlying_type_t<E> value) noexcept -> utils::enable_if_t<E, std::optional<std::decay_t<E>>>
    {
        using D = std::decay_t<E>;
        using U = underlying_type_t<D>;

        if constexpr ( utils::count_v<std::string_view, D> == 0 )
        {
            return {}; // Empty enum.
        }
        else
        {
            if constexpr ( utils::is_sparse_v<D> )
            {
                for ( std::size_t i = 0; i < utils::count_v<std::string_view, D>; ++i )
                {
                    if ( value == static_cast< U >( enum_value<std::string_view, D>(i) ) )
                    {
                        return static_cast< D >( value );
                    }
                }
            }
            else
            {
                if ( value >= utils::min_v<std::string_view, D> && value <= utils::max_v<std::string_view, D> )
                {
                    return static_cast< D >( value );
                }
            }

            return {}; // Invalid value or out of range.
        }
    }

    // Obtains enum value from name.
    // Returns optional with enum value.
    template <typename E, typename BinaryPredicate = std::equal_to<>>
    [[nodiscard]] MAGIC_ENUM_EX_NOINLINE constexpr auto enum_cast(std::string_view value, [[maybe_unused]] BinaryPredicate p = {}) noexcept -> utils::enable_if_t<E, std::optional<std::decay_t<E>>, BinaryPredicate>
    {
        using D = std::decay_t<E>;

        if constexpr ( utils::count_v<std::string_view, D> == 0 )
        {
            return {}; // Empty enum.
        }
        else
        {
            for ( std::size_t i = 0; i < utils::count_v<std::string_view, D>; ++i )
            {
                if ( utils::cmp_equal(value, utils::names_v<std::string_view, D>[ i ], p) )
                {
                    return enum_value<D>(i);
                }
            }

            return {}; // Invalid value or out of range.
        }
    }

    // Obtains enum value from name.
    // Returns optional with enum value.
    template <typename E, typename BinaryPredicate = std::equal_to<>>
    [[nodiscard]] MAGIC_ENUM_EX_NOINLINE constexpr auto enum_cast(std::wstring_view value, [[maybe_unused]] BinaryPredicate p = {}) noexcept -> utils::enable_if_t<E, std::optional<std::decay_t<E>>, BinaryPredicate>
    {
        using D = std::decay_t<E>;

        if constexpr ( utils::count_v<std::wstring_view, D> == 0 )
        {
            return {}; // Empty enum.
        }
        else
        {
            for ( std::size_t i = 0; i < utils::count_v<std::wstring_view, D>; ++i )
            {
                if ( utils::cmp_equal(value, utils::names_v<std::wstring_view, D>[ i ], p) )
                {
                    return enum_value<D>(i);
                }
            }

            return {}; // Invalid value or out of range.
        }
    }
}