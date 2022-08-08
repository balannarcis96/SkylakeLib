//!
//! \file Concepts.h
//! 
//! \brief C++ 20 and later concepts extension to the std lib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 

namespace std
{
    static_assert( sizeof( int ) == sizeof( long ), "Unsupported platform!" );

    // Accepts: char, unsigned char or wchar_t, char8_t, char16_t, char32_t
    template< typename T >
    concept TChar =
        std::is_same_v< T, char > || std::is_same_v< T, char8_t > || std::is_same_v< T, unsigned char > || std::is_same_v< T, wchar_t > || std::is_same_v< T, char16_t > || std::is_same_v< T, char32_t >;

    // Accepts: float, double, long double
    template< typename T >
    concept TFloat = std::is_floating_point_v< T >;

    // Accepts: all integral types, char, short, int ,unsigned int etc
    template< typename T >
    concept TIntegral = std::is_integral_v< T >;

    // Accepts: short
    template< typename T >
    concept TSInt8 = std::is_same_v< T, char >;

    // Accepts: unsigned short
    template< typename T >
    concept TUInt8 = std::is_same_v< T, unsigned char >;

    // Accepts: short
    template< typename T >
    concept TSInt16 = std::is_same_v< T, short >;

    // Accepts: unsigned short
    template< typename T >
    concept TUInt16 = std::is_same_v< T, unsigned short >;

    // Accepts: short
    template< typename T >
    concept TSInt32 = std::is_same_v< T, int > || std::is_same_v< T, long >;

    // Accepts: unsigned short
    template< typename T >
    concept TUInt32 = std::is_same_v< T, unsigned int > || std::is_same_v< T, unsigned long >;

    // Accepts: short
    template< typename T >
    concept TSInt64 = std::is_same_v< T, long long >;

    // Accepts: unsigned short
    template< typename T >
    concept TUInt64 = std::is_same_v< T, unsigned long long >;

    // Accepts: int, long ,long long
    template< typename T >
    concept TSInt32Or64 = TSInt32< T > || TSInt64< T >;

    // Accepts: unsigned int, unsigned long ,unsigned long long
    template< typename T >
    concept TUInt32Or64 = TUInt32< T > || TUInt64< T >;

    // Accepts: char, short int, long ,long long
    template< typename T >
    concept TSInteger = TSInt8< T > || TSInt16< T > || TSInt32< T > || TSInt64< T >;

    // Accepts: unsigned char, unsigned short, unsigned int, unsigned long ,unsigned long long
    template< typename T >
    concept TUInteger = TUInt8< T > || TUInt16< T > || TUInt32< T > || TUInt64< T >;

    // Accepts: int, long ,long long
    template< typename T >
    concept TInt32Or64 = TSInt32Or64< T > || TUInt32Or64< T >;

    // Accepts: TFloat<T> == true or TIntegral<T> == true
    template< typename T >
    concept TArithmetic = std::is_arithmetic_v< T >;

    template< typename T1, typename T2, typename TResult >
    concept TComparableEqual = requires( const std::remove_reference_t< T1 > &Temp1, const std::remove_reference_t< T2 > &Temp2 )
    {
        {
            Temp1 == Temp2
            } -> std::convertible_to< TResult >;
    };

    template< typename T1, typename T2, typename TResult >
    concept TComparableSmaller = requires( const std::remove_reference_t< T1 > &Temp1, const std::remove_reference_t< T2 > &Temp2 )
    {
        {
            Temp1 < Temp2
            } -> std::convertible_to< TResult >;
    };

    template< typename T1, typename T2, typename TResult >
    concept TComparableSmallerOrEqual = requires( const std::remove_reference_t< T1 > &Temp1, const std::remove_reference_t< T2 > &Temp2 )
    {
        {
            Temp1 <= Temp2
            } -> std::convertible_to< TResult >;
    };

    template< typename T1, typename T2, typename TResult >
    concept TComparableBigger = requires( const std::remove_reference_t< T1 > &Temp1, const std::remove_reference_t< T2 > &Temp2 )
    {
        {
            Temp1 > Temp2
            } -> std::convertible_to< TResult >;
    };

    template< typename T1, typename T2, typename TResult >
    concept TComparableBiggerOrEqual = requires( const std::remove_reference_t< T1 > &Temp1, const std::remove_reference_t< T2 > &Temp2 )
    {
        {
            Temp1 >= Temp2
            } -> std::convertible_to< TResult >;
    };

    template< typename T1, typename T2, typename TResult >
    concept TMultiplicable = requires( const std::remove_reference_t< T1 > &Temp1, const std::remove_reference_t< T2 > &Temp2 )
    {
        {
            Temp1 *Temp2
            } -> std::convertible_to< TResult >;
    };

    template< typename T1, typename T2, typename TResult >
    concept TSummable = requires( const std::remove_reference_t< T1 > &Temp1, const std::remove_reference_t< T2 > &Temp2 )
    {
        {
            Temp1 + Temp2
            } -> std::convertible_to< TResult >;
    };

    template< typename T1, typename T2, typename TResult >
    concept TSubstractable = requires( const std::remove_reference_t< T1 > &Temp1, const std::remove_reference_t< T2 > &Temp2 )
    {
        {
            Temp1 - Temp2
            } -> std::convertible_to< TResult >;
    };

    template< typename T1, typename T2, typename TResult >
    concept TDivisible = requires( const std::remove_reference_t< T1 > &Temp1, const std::remove_reference_t< T2 > &Temp2 )
    {
        {
            Temp1 / Temp2
            } -> std::convertible_to< TResult >;
    };

    template< typename T >
    concept TSelfComparableEqual = TComparableEqual< T, T, bool >;

    template< typename T >
    concept TSelfComparableSmaller = TComparableSmaller< T, T, bool >;

    template< typename T >
    concept TSelfComparableSmallerOrEqual = TComparableSmallerOrEqual< T, T, bool >;

    template< typename T >
    concept TSelfComparableBigger = TComparableBigger< T, T, bool >;

    template< typename T >
    concept TSelfComparableBiggerOrEqual = TComparableBiggerOrEqual< T, T, bool >;

    template< typename T >
    concept TSelfMultiplicableValue = TMultiplicable< T, T, T >;
    template< typename T >
    concept TSelfMultiplicableObject = TMultiplicable< T, T, T & >;
    template< typename T >
    concept TSelfMultiplicable = TSelfMultiplicableValue< T > || TSelfMultiplicableObject< T >;

    template< typename T >
    concept TSelfSummableValue = TSummable< T, T, T >;
    template< typename T >
    concept TSelfSummableObject = TSummable< T, T, T & >;
    template< typename T >
    concept TSelfSummable = TSelfSummableValue< T > || TSelfSummableObject< T >;

    template< typename T >
    concept TSelfSubstractableValue = TSubstractable< T, T, T >;
    template< typename T >
    concept TSelfSubstractableObject = TSubstractable< T, T, T & >;
    template< typename T >
    concept TSelfSubstractable = TSelfSubstractableValue< T > || TSelfSubstractableObject< T >;

    template< typename T >
    concept TSelfDivisibleValue = TDivisible< T, T, T >;
    template< typename T >
    concept TSelfDivisibleObject = TDivisible< T, T, T & >;
    template< typename T >
    concept TSelfDivisible = TSelfDivisibleValue< T > || TSelfDivisibleObject< T >;

    template< typename T >
    concept TUsableForMinOrMax =
        TSelfComparableSmaller< T > ||
        TSelfComparableSmallerOrEqual< T > ||
        TSelfComparableBigger< T > ||
        TSelfComparableBiggerOrEqual< T >;

    template< typename T >
    concept TBasicMathEnabled =
        TSelfSummable< T > &&
        TSelfSubstractable< T > &&
        TSelfMultiplicable< T > &&
        TSelfDivisible< T >;
} // namespace std