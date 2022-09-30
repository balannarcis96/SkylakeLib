//!
//! \file SMathTemplates.h
//! 
//! \brief Math template types for SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    static SKL_FORCEINLINE float InvSqrtInternal( float InValue )
    {
        const __m128 One = _mm_set_ss( 1.0f );
        const __m128 Y0     = _mm_set_ss( InValue );
        const __m128 X0     = _mm_sqrt_ss( Y0 );
        const __m128 R0     = _mm_div_ss( One, X0 );
        float         temp;
        _mm_store_ss( &temp, R0 );
        return temp;
    }

    static SKL_FORCEINLINE double InvSqrtInternal( double InValue )
    {
        const __m128d One = _mm_set_sd( 1.0 );
        const __m128d Y0  = _mm_set_sd( InValue );
        const __m128d X0  = _mm_sqrt_sd( One, Y0 );
        const __m128d R0  = _mm_div_sd( One, X0 );
        double          temp;
        _mm_store_sd( &temp, R0 );
        return temp;
    }

    static SKL_FORCEINLINE float InvSqrtEstInternal( float F )
    {
        // Performs one pass of Newton-Raphson iteration on the hardware estimate
        const __m128 fOneHalf = _mm_set_ss( 0.5f );
        __m128         Y0, X0, X1, FOver2;
        float         temp;

        Y0       = _mm_set_ss( F );
        X0       = _mm_rsqrt_ss( Y0 ); // 1/sqrt estimate (12 bits)
        FOver2 = _mm_mul_ss( Y0, fOneHalf );

        // 1st Newton-Raphson iteration
        X1 = _mm_mul_ss( X0, X0 );
        X1 = _mm_sub_ss( fOneHalf, _mm_mul_ss( FOver2, X1 ) );
        X1 = _mm_add_ss( X0, _mm_mul_ss( X0, X1 ) );

        _mm_store_ss( &temp, X1 );
        return temp;
    }

    static SKL_FORCEINLINE double InvSqrtEstInternal( double InValue )
    {
        return InvSqrtInternal( InValue );
    }

    //! Converts a float to an integer 32bit and truncates it towards zero.
    SKL_FORCEINLINE int32_t FTrunc( float Value ) noexcept
    {
        return _mm_cvtt_ss2si( _mm_set_ss( Value ) );
    }

    //! Converts a float to an integer 32bit and truncates it towards zero.
    SKL_FORCEINLINE int32_t FTrunc( double Value ) noexcept
    {
        return _mm_cvttsd_si32( _mm_set_sd( Value ) );
    }

    //! Converts a double to an integer 64bit and truncates it towards zero.
    SKL_FORCEINLINE int64_t FTrunc64( double Value ) noexcept
    {
        return _mm_cvttsd_si64( _mm_set_sd( Value ) );
    }

    //! Converts a float to the nearest integer. Rounds up when the fraction is .5
    SKL_FORCEINLINE int32_t FRound( float Value ) noexcept
    {
        return _mm_cvt_ss2si( _mm_set_ss( Value + Value + 0.5f ) ) >> 1;
    }

    //! Converts a double to the nearest integer. Rounds up when the fraction is .5
    SKL_FORCEINLINE int32_t FRound( double Value ) noexcept
    {
        return _mm_cvtsd_si32( _mm_set_sd( Value + Value + 0.5 ) ) >> 1;
    }

    //! Converts a double to the nearest integer. Rounds up when the fraction is .5
    SKL_FORCEINLINE int64_t FRound64( double Value ) noexcept
    {
        return _mm_cvtsd_si64( _mm_set_sd( Value + Value + 0.5 ) ) >> 1;
    }

    //! Converts a float to a less or equal integer.
    SKL_FORCEINLINE int32_t FFloorToInt( float Value ) noexcept
    {
        return _mm_cvt_ss2si( _mm_set_ss( Value + Value - 0.5f ) ) >> 1;
    }

    //! Converts a double to a less or equal integer.
    SKL_FORCEINLINE int32_t FFloorToInt( double Value ) noexcept
    {
        return _mm_cvtsd_si32( _mm_set_sd( Value + Value - 0.5 ) ) >> 1;
    }

    //! Converts a float to a less or equal integer.
    SKL_FORCEINLINE int64_t FFloorToInt64( float Value ) noexcept
    {
        return _mm_cvtsd_si64( _mm_set_sd( Value + Value - 0.5 ) ) >> 1;
    }

    //! Converts a float to a greater or equal integer 32.
    SKL_FORCEINLINE int32_t FCeilToInt( float Value ) noexcept
    {
        return -( _mm_cvt_ss2si( _mm_set_ss( -0.5f - ( Value + Value ) ) ) >> 1 );
    }

    //! Converts a double to a greater or equal integer 32.
    SKL_FORCEINLINE int32_t FCeilToInt( double Value ) noexcept
    {
        return -( _mm_cvtsd_si32( _mm_set_sd( -0.5 - ( Value + Value ) ) ) >> 1 );
    }

    //! Converts a double to a greater or equal integer 64.
    SKL_FORCEINLINE int64_t FCeilToInt64( double Value ) noexcept
    {
        return -( _mm_cvtsd_si64( _mm_set_sd( -0.5 - ( Value + Value ) ) ) >> 1 );
    }

    //! Composes a floating point value with the magnitude of Number and the sign of Sign.
    SKL_FORCEINLINE float FCopySign( float Number, float Sign ) noexcept
    {
        return std::copysign( Number, Sign );
    }

    //! Get the absolute value of A
    template< typename T >
    SKL_FORCEINLINE constexpr T Abs( const T A ) noexcept requires std::is_arithmetic_v< T >
    {
        if constexpr( std::is_floating_point_v< T > )
        {
            return fabs( A );
        }
        else if constexpr( std::is_unsigned_v< T > )
        {
            return A;
        }
        else
        {
            return ( A >= static_cast< T >( 0 ) ) ? A : -A;
        }
    }

    //! Get the Sign of A
    //! \returns (T)(-1) if negative , (T)(0) if zero, (T)(1) if positive
    template< typename T >
    SKL_FORCEINLINE constexpr T Sgn( const T A ) noexcept requires std::is_arithmetic_v< T >
    {
        if constexpr( std::is_unsigned_v< T > )
        {
            return static_cast< T >( A != 0 );
        }
        else if constexpr( std::is_integral_v< T > )
        {
            constexpr T ZeroValue    = static_cast< T >( 0 );
            constexpr T One            = static_cast< T >( 1 );
            constexpr T NegativeOne = static_cast< T >( -1 );

            return NegativeOne + ( One * ( A == ZeroValue ) ) + ( 2 * ( A > ZeroValue ) );
        }
        else
        {
            constexpr T ZeroValue    = static_cast< T >( 0 );
            constexpr T One            = static_cast< T >( 1 );
            constexpr T NegativeOne = static_cast< T >( -1 );

            return ( A > ZeroValue ) ? One : ( ( A < ZeroValue ) ? NegativeOne : ZeroValue );
        }
    }

    //! Min function using available self comparision operator <, >, <=, >=
    //! \remarks Parameters are passed by value and it returns by value, careful when using it with objects
    //! \remarks Accepts a maximum of 10 parameters
    template< std::TSelfComparableSmaller T, std::TSelfComparableSmaller... TOther >
    SKL_FORCEINLINE constexpr T Min( const T A, const T B, const TOther... C ) noexcept
    {
        static_assert( sizeof...( TOther ) < 8, "Please loop to get Min from more than 10 elements" );

        if constexpr( sizeof...( TOther ) == 0 )
        {
            return A < B ? A : B;
        }
        else
        {
            const auto Temp = Min( A, B );
            return Min( Temp, C... );
        }
    }

    //! Max function using available self comparision operator <, >, <=, >=
    //! \remarks Parameters are passed by value and it returns by value, careful when using it with objects
    //! \remarks Accepts a maximum of 10 parameters
    template< std::TSelfComparableSmaller T, std::TSelfComparableSmaller... TOther >
    SKL_FORCEINLINE constexpr T Max( const T A, const T B, const TOther... C ) noexcept
    {
        static_assert( sizeof...( TOther ) < 8, "Please loop to get Max from more than 10 elements" );

        if constexpr( sizeof...( TOther ) == 0 )
        {
            return A < B ? B : A;
        }
        else
        {
            const auto Temp = Max( A, B );
            return Max( Temp, C... );
        }
    }

    //! Calculate Square of A, implementation: A * A
    //! \remarks Parameters are passed by value and it returns by value, careful when using it with objects
    template< std::TSelfMultiplicable T >
    SKL_FORCEINLINE constexpr auto Square( const T A ) noexcept
    {
        return A * A;
    }

    //! Clamp value of X between Min and Max
    //! \remarks Parameters are passed by value and it returns by value, careful when using it with objects
    template< std::TSelfComparableSmaller T >
    SKL_FORCEINLINE constexpr T Clamp( const T X, const T Min, const T Max ) noexcept
    {
        return X < Min ? Min : X < Max ? X :
                                           Max;
    }

    //! Checks if two floating point numbers are nearly equal
    template< std::TFloat T >
    SKL_FORCEINLINE constexpr bool FIsNearlyEqual( T A, T B, T ErrorTolerance = static_cast< T >( SMALL_NUMBER ) ) noexcept
    {
        return Abs( A - B ) < ErrorTolerance;
    }

    //! Checks if a floating point number is nearly zero
    template< std::TFloat T >
    SKL_FORCEINLINE constexpr bool FIsNearlyZero( T Value, T ErrorTolerance = static_cast< T >( SMALL_NUMBER ) ) noexcept
    {
        return Abs( Value ) < ErrorTolerance;
    }

    //! Determines if the given floating point number @Value is a not-a-number (NaN) value
    template< std::TFloat T >
    SKL_FORCEINLINE bool FIsNaN( T Value ) noexcept
    {
        if constexpr( std::is_same_v< T, float > )
        {
            return static_cast< bool >( _isnanf( Value ) );
        }
        else
        {
            return static_cast< bool >( _isnan( Value ) );
        }
    }

    //! Returns true if @Value is either a normal or subnormal finite value. Returns false if @Value is infinite or a NaN.
    template< std::TFloat T >
    SKL_FORCEINLINE bool FIsFinite( T Value ) noexcept
    {
        if constexpr( std::is_same_v< T, float > )
        {
            return static_cast< bool >( _finitef( Value ) );
        }
        else
        {
            return static_cast< bool >( _finite( Value ) );
        }
    }

    //! Computes the e (Euler's number, 2.7182818) raised to the given @Power arg
    template< std::TFloat T >
    SKL_FORCEINLINE T FEulerToPower( T Power ) noexcept
    {
        return std::exp( Power );
    }

    //! Computes the natural (base e) logarithm of @Value
    template< std::TFloat T >
    SKL_FORCEINLINE T FLogE( T Value ) noexcept
    {
        return std::log( Value );
    }

    //! Computes the floating-point remainder of the division operation @Value/@ModByValue
    template< std::TFloat T >
    SKL_FORCEINLINE T FMod( T Value, T ModByValue ) noexcept
    {
        return std::fmod( Value, ModByValue );
    }

    //! Computes the sine of @Value (measured in radians)
    template< std::TFloat T >
    SKL_FORCEINLINE T FSin( T Value ) noexcept
    {
        return std::sin( Value );
    }

    //! Computes the cosine of @Value (measured in radians)
    template< std::TFloat T >
    SKL_FORCEINLINE T FCos( T Value ) noexcept
    {
        return std::cos( Value );
    }

    //! Computes the principal values of the arc sine of @Value (measured in radians)
    template< std::TFloat T >
    SKL_FORCEINLINE T FAsin( T Value ) noexcept
    {
        return std::asin( Value );
    }

    //! Computes the principal values of the arc cosine of @Value (measured in radians)
    template< std::TFloat T >
    SKL_FORCEINLINE T FAcos( T Value ) noexcept
    {
        return std::acos( Value );
    }

    //! Computes the tangent of @Value (measured in radians)
    template< std::TFloat T >
    SKL_FORCEINLINE T FTan( T Value ) noexcept
    {
        return std::tan( Value );
    }

    //! Computes the arc tangent of @Value (measured in radians)
    template< std::TFloat T >
    SKL_FORCEINLINE T FAtan( T Value ) noexcept
    {
        return std::atan( Value );
    }

    //! Computes the arc tangent of @Y/@X using the signs of arguments to determine the correct quadrant (measured in radians)
    template< std::TFloat T >
    SKL_FORCEINLINE T FAtan2( T X, T Y ) noexcept
    {
        return std::atan2( X, Y );
    }

    //! Computes the square root of @Value
    template< std::TArithmetic T >
    SKL_FORCEINLINE T Sqrt( T Value ) noexcept
    {
        return std::sqrt( Value );
    }

    //! Computes the inverse square root of @Value (1.0 / @Value)
    template< std::TFloat T >
    SKL_FORCEINLINE T InverseSqrt( T Value ) noexcept
    {
        return InvSqrtInternal( Value );
    }

    //! Computes(estimates) the inverse square root of @Value (1.0 / @Value)
    template< std::TFloat T >
    SKL_FORCEINLINE T InverseSqrtEst( T Value ) noexcept
    {
        return InvSqrtEstInternal( Value );
    }

    //! Computes the value of @Base raised to @Exp
    template< std::TArithmetic T >
    SKL_FORCEINLINE T Pow( T Base, T Exp ) noexcept
    {
        return std::pow( Base, Exp );
    }

    //! Floors @Value
    template< std::TFloat T >
    SKL_FORCEINLINE T FFloor( T Value ) noexcept
    {
        return std::floor( Value );
    }

    //! Gets the next whole value starting from @Value
    template< std::TFloat T >
    SKL_FORCEINLINE T FCeil( T Value ) noexcept
    {
        return std::ceil( Value );
    }

#pragma intrinsic( _BitScanReverse )
#pragma intrinsic( _BitScanReverse64 )
    //! Computes the base 2 logarithm for @Value. @Value must be greater than 0
    template< std::TUInt32Or64 T >
    SKL_FORCEINLINE uint32_t FloorLog2( T Value ) noexcept
    {
        unsigned long Result;

        if constexpr( std::_Is_any_of_v< std::remove_cv_t< T >, unsigned int, unsigned long > )
        {
            _BitScanReverse( &Result, static_cast< unsigned long >( Value ) );
        }
        else
        {
            _BitScanReverse64( &Result, Value );
        }

        return Result;
    }

    //! Counts the number of leading zeros in the bit representation of @Value
    template< std::TUInt32Or64 T >
    SKL_FORCEINLINE uint32_t CountLeadingZeros( T Value ) noexcept
    {
        if constexpr( std::_Is_any_of_v< std::remove_cv_t< T >, unsigned int, unsigned long > )
        {
            if( Value == 0 )
                return 32;
            return 31 - FloorLog2( Value );
        }
        else
        {
            if( Value == 0 )
                return 64;
            return 63 - FloorLog2( Value );
        }
    }

    //! Counts the number of leading zeros in the bit representation of @Value
    template< std::TInt32Or64 T >
    SKL_FORCEINLINE constexpr bool IsPowerOfTwo( T Value ) noexcept
    {
        return ( ( Value & ( Value - 1 ) ) == 0 );
    }

    //! Computes the linear interpolation between @Start and @End by @Alpha
    template< std::TBasicMathEnabled T, std::TFloat U >
    SKL_FORCEINLINE constexpr T Lerp( const T &Start, const T &End, const U &Alpha ) noexcept
    {
        return static_cast< T >( Start + ( ( End - Start ) * Alpha ) );
    }

    //! Computes the bi-linear interpolation between 4 points forming 2 lines
    template< std::TBasicMathEnabled T, std::TFloat U >
    SKL_FORCEINLINE constexpr T BiLerp(
        const T &Point_00, const T &Point_10, const T &Point_01, const T &Point_11, const U &Alpha_BetweenPoints, const U &Alpha_Of_Individual_Lerps ) noexcept
    {
        return Lerp(
            Lerp( Point_00, Point_10, Alpha_BetweenPoints ),
            Lerp( Point_01, Point_11, Alpha_BetweenPoints ),
            Alpha_Of_Individual_Lerps );
    }

    //! Computes the cubic interpolation
    template< std::TBasicMathEnabled T, std::TFloat U >
    SKL_FORCEINLINE constexpr T CubicInterp(
        const T &P0, const T &T0, const T &P1, const T &T1, const U &Alpha ) noexcept
    {
        const auto AlphaSquared = Square( Alpha );
        const auto AlphaCubed    = AlphaSquared * Alpha;

        return static_cast< T >(
            ( ( ( 2 * AlphaCubed ) - ( 3 * AlphaSquared ) + 1 ) * P0 ) +
            ( ( AlphaCubed - ( 2 * AlphaSquared ) + Alpha ) * T0 ) + ( ( AlphaCubed - AlphaSquared ) * T1 ) +
            ( ( ( -2 * AlphaCubed ) + ( 3 * AlphaSquared ) ) * P1 ) );
    }

    //! Computes the first derivative of the cubic interpolation function
    template< std::TBasicMathEnabled T, std::TFloat U >
    SKL_FORCEINLINE constexpr T CubicInterpDerivative(
        const T &P0, const T &T0, const T &P1, const T &T1, const U &Alpha ) noexcept
    {
        const auto A =
            ( static_cast< U >( 6.0 ) * P0 ) + ( static_cast< U >( 3.0 ) * T0 ) + ( static_cast< U >( 3.0 ) * T1 ) - ( static_cast< U >( 6.0 ) * P1 );

        const auto B =
            ( static_cast< U >( -6.0 ) * P0 ) - ( static_cast< U >( 4.0 ) * T0 ) - ( static_cast< U >( 2.0 ) * T1 ) + ( static_cast< U >( 6.0 ) * P1 );

        const auto C = T0;

        const auto AlphaSquared = Square( Alpha );

        return ( A * AlphaSquared ) + ( B * Alpha ) + C;
    }

    //! Computes the second derivative of the cubic interpolation function
    template< std::TBasicMathEnabled T, std::TFloat U >
    SKL_FORCEINLINE constexpr T CubicInterpSecondDerivative(
        const T &P0, const T &T0, const T &P1, const T &T1, const U &Alpha ) noexcept
    {
        const auto A =
            ( static_cast< U >( 12.0 ) * P0 ) + ( static_cast< U >( 6.0 ) * T0 ) + ( static_cast< U >( 6.0 ) * T1 ) - ( static_cast< U >( 12.0 ) * P1 );

        const auto B =
            ( static_cast< U >( -6.0 ) * P0 ) - ( static_cast< U >( 4.0 ) * T0 ) - ( static_cast< U >( 2.0 ) * T1 ) + ( static_cast< U >( 6.0 ) * P1 );

        return ( A * Alpha ) + B;
    }

    //! Computes ease-in-out linear interpolation. @Exp controls the degree of the curve.
    template< std::TBasicMathEnabled T, std::TFloat U >
    SKL_FORCEINLINE constexpr T InterpEaseInOut(
        const T &Start, const T &End, const U &Alpha, const U &Exp ) noexcept
    {
        const U NewAlpha = ( Alpha < static_cast< U >( 0.5 ) ?
                                 static_cast< U >( 0.5 ) * Pow( static_cast< U >( 2.0 ) * Alpha, Exp ) :
                                   static_cast< U >( 1.0 - 0.5 ) * Pow( static_cast< U >( 2.0 ) * ( static_cast< U >( 1.0 ) - Alpha ), Exp ) );

        return Lerp( Start, End, NewAlpha );
    }
} // namespace SMath
