//!
//! \file SMathTemplates.h
//! 
//! \brief Math template types for SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

#include "MathEIS.h"

namespace SKL
{
    //! Composes a floating point value with the magnitude of Number and the sign of Sign.
    SKL_FORCEINLINE SKL_NODISCARD inline float FCopySign( float Number, float Sign ) noexcept
    {
        return std::copysign( Number, Sign );
    }

    //! Get the absolute value of A
    template<typename T>
    SKL_FORCEINLINE SKL_NODISCARD inline constexpr T Abs( const T A ) noexcept requires std::is_arithmetic_v<T>
    {
        if constexpr( std::is_floating_point_v<T> )
        {
            return fabs( A );
        }
        else if constexpr( std::is_unsigned_v<T> )
        {
            return A;
        }
        else
        {
            return ( A >= static_cast<T>( 0 ) ) ? A : -A;
        }
    }

    //! Get the Sign of A
    //! \returns (T)(-1) if negative , (T)(0) if zero, (T)(1) if positive
    template<typename T>
    SKL_FORCEINLINE SKL_NODISCARD inline constexpr T Sgn( const T A ) noexcept requires std::is_arithmetic_v<T>
    {
        if constexpr( std::is_unsigned_v<T> )
        {
            return static_cast<T>( A != 0 );
        }
        else if constexpr( std::is_integral_v<T> )
        {
            constexpr T ZeroValue  { static_cast<T>( 0 ) };
            constexpr T One        { static_cast<T>( 1 ) };
            constexpr T NegativeOne{ static_cast<T>( -1 ) };

            return NegativeOne + ( One * ( A == ZeroValue ) ) + ( 2 * ( A > ZeroValue ) );
        }
        else
        {
            constexpr T ZeroValue  { static_cast<T>( 0 ) };
            constexpr T One        { static_cast<T>( 1 ) };
            constexpr T NegativeOne{ static_cast<T>( -1 ) };

            return ( A > ZeroValue ) ? One : ( ( A < ZeroValue ) ? NegativeOne : ZeroValue );
        }
    }

    //! Min function using available self comparison operator <, >, <=, >=
    //! \remarks Parameters are passed by value and it returns by value, careful when using it with objects
    //! \remarks Accepts a maximum of 10 parameters
    template<std::TSelfComparableSmaller T, std::TSelfComparableSmaller... TOther>
    SKL_FORCEINLINE SKL_NODISCARD inline constexpr T Min( const T A, const T B, const TOther... C ) noexcept
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

    //! Max function using available self comparison operator <, >, <=, >=
    //! \remarks Parameters are passed by value and it returns by value, careful when using it with objects
    //! \remarks Accepts a maximum of 10 parameters
    template<std::TSelfComparableSmaller T, std::TSelfComparableSmaller... TOther>
    SKL_FORCEINLINE SKL_NODISCARD inline constexpr T Max( const T A, const T B, const TOther... C ) noexcept
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
    template<std::TSelfMultiplicable T>
    SKL_FORCEINLINE SKL_NODISCARD inline constexpr auto Square( const T A ) noexcept
    {
        return A * A;
    }

    //! Clamp value of X between Min and Max
    //! \remarks Parameters are passed by value and it returns by value, careful when using it with objects
    template<std::TSelfComparableSmaller T>
    SKL_FORCEINLINE SKL_NODISCARD inline constexpr T Clamp( const T X, const T Min, const T Max ) noexcept
    {
        return X < Min ? Min : X < Max ? X : Max;
    }

    //! Checks if two floating point numbers are nearly equal
    template<std::TFloat T>
    SKL_FORCEINLINE SKL_NODISCARD inline constexpr bool FIsNearlyEqual( T A, T B, T ErrorTolerance = static_cast<T>( SMALL_NUMBER ) ) noexcept
    {
        return Abs( A - B ) < ErrorTolerance;
    }

    //! Checks if a floating point number is nearly zero
    template<std::TFloat T>
    SKL_FORCEINLINE SKL_NODISCARD inline constexpr bool FIsNearlyZero( T Value, T ErrorTolerance = static_cast<T>( SMALL_NUMBER ) ) noexcept
    {
        return Abs( Value ) < ErrorTolerance;
    }

    //! Determines if the given floating point number @Value is a not-a-number (NaN) value
    template<std::TFloat T>
    SKL_FORCEINLINE SKL_NODISCARD inline bool FIsNaN( T Value ) noexcept
    {
        if constexpr( std::is_same_v<T, float> )
        {
            return static_cast< bool >( _isnanf( Value ) );
        }
        else
        {
            return static_cast< bool >( _isnan( Value ) );
        }
    }

    //! Returns true if @Value is either a normal or subnormal finite value. Returns false if @Value is infinite or a NaN.
    template<std::TFloat T>
    SKL_FORCEINLINE SKL_NODISCARD inline bool FIsFinite( T Value ) noexcept
    {
        if constexpr( std::is_same_v<T, float> )
        {
            return static_cast< bool >( _finitef( Value ) );
        }
        else
        {
            return static_cast< bool >( _finite( Value ) );
        }
    }

    //! Computes the e (Euler's number, 2.7182818) raised to the given @Power arg
    template<std::TFloat T>
    SKL_FORCEINLINE SKL_NODISCARD inline T FEulerToPower( T Power ) noexcept
    {
        return std::exp( Power );
    }

    //! Computes the natural (base e) logarithm of @Value
    template<std::TFloat T>
    SKL_FORCEINLINE SKL_NODISCARD inline T FLogE( T Value ) noexcept
    {
        return std::log( Value );
    }

    //! Computes the floating-point remainder of the division operation @Value/@ModByValue
    template<std::TFloat T>
    SKL_FORCEINLINE SKL_NODISCARD inline T FMod( T Value, T ModByValue ) noexcept
    {
        return std::fmod( Value, ModByValue );
    }

    //! Computes the sine of @Value (measured in radians)
    template<std::TFloat T>
    SKL_FORCEINLINE SKL_NODISCARD inline T FSin( T Value ) noexcept
    {
        return std::sin( Value );
    }

    //! Computes the cosine of @Value (measured in radians)
    template<std::TFloat T>
    SKL_FORCEINLINE SKL_NODISCARD inline T FCos( T Value ) noexcept
    {
        return std::cos( Value );
    }

    //! Computes the principal values of the arc sine of @Value (measured in radians)
    template<std::TFloat T>
    SKL_FORCEINLINE SKL_NODISCARD inline T FAsin( T Value ) noexcept
    {
        return std::asin( Value );
    }

    //! Computes the principal values of the arc cosine of @Value (measured in radians)
    template<std::TFloat T>
    SKL_FORCEINLINE SKL_NODISCARD inline T FAcos( T Value ) noexcept
    {
        return std::acos( Value );
    }

    //! Computes the tangent of @Value (measured in radians)
    template<std::TFloat T>
    SKL_FORCEINLINE SKL_NODISCARD inline T FTan( T Value ) noexcept
    {
        return std::tan( Value );
    }

    //! Computes the arc tangent of @Value (measured in radians)
    template<std::TFloat T>
    SKL_FORCEINLINE SKL_NODISCARD inline T FAtan( T Value ) noexcept
    {
        return std::atan( Value );
    }

    //! Computes the arc tangent of @Y/@X using the signs of arguments to determine the correct quadrant (measured in radians)
    template<std::TFloat T>
    SKL_FORCEINLINE SKL_NODISCARD inline T FAtan2( T X, T Y ) noexcept
    {
        return std::atan2( X, Y );
    }

    //! Computes the square root of @Value
    template<std::TArithmetic T>
    SKL_FORCEINLINE SKL_NODISCARD inline T Sqrt( T Value ) noexcept
    {
        return std::sqrt( Value );
    }

    //! Computes the inverse square root of @Value (1.0 / @Value)
    template<std::TFloat T>
    SKL_FORCEINLINE SKL_NODISCARD inline T InverseSqrt( T Value ) noexcept
    {
        return InvSqrtInternal( Value );
    }

    //! Computes(estimates) the inverse square root of @Value (1.0 / @Value)
    template<std::TFloat T>
    SKL_FORCEINLINE SKL_NODISCARD inline T InverseSqrtEst( T Value ) noexcept
    {
        return InvSqrtEstInternal( Value );
    }

    //! Computes the value of @Base raised to @Exp
    template<std::TArithmetic T>
    SKL_FORCEINLINE SKL_NODISCARD inline T Pow( T Base, T Exp ) noexcept
    {
        return std::pow( Base, Exp );
    }

    //! Floors @Value
    template<std::TFloat T>
    SKL_FORCEINLINE SKL_NODISCARD inline T FFloor( T Value ) noexcept
    {
        return std::floor( Value );
    }

    //! Gets the next whole value starting from @Value
    template<std::TFloat T>
    SKL_FORCEINLINE SKL_NODISCARD inline T FCeil( T Value ) noexcept
    {
        return std::ceil( Value );
    }

#pragma intrinsic( _BitScanReverse )
#pragma intrinsic( _BitScanReverse64 )
    //! Computes the base 2 logarithm for @Value. @Value must be greater than 0
    template<std::TUInt32Or64 T>
    SKL_FORCEINLINE SKL_NODISCARD inline uint32_t FloorLog2( T Value ) noexcept
    {
        unsigned long Result;
        
        if constexpr( std::is_same_v<std::remove_cv_t<T>, unsigned int>
                   || std::is_same_v<std::remove_cv_t<T>, unsigned long> )
        {
            ( void )_BitScanReverse( &Result, static_cast<unsigned long>( Value ) );
        }
        else
        {
            ( void )_BitScanReverse64( &Result, Value );
        }

        return Result;
    }

    //! Counts the number of leading zeros in the bit representation of @Value
    template<std::TUInt32Or64 T>
    SKL_FORCEINLINE SKL_NODISCARD inline uint32_t CountLeadingZeros( T Value ) noexcept
    {
        if constexpr( std::is_same_v<std::remove_cv_t<T>, unsigned int>
                   || std::is_same_v<std::remove_cv_t<T>, unsigned long> )
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

    //! Computes the linear interpolation between @Start and @End by @Alpha
    template<std::TBasicMathEnabled T, std::TFloat U>
    SKL_FORCEINLINE SKL_NODISCARD inline constexpr T Lerp( const T &Start, const T &End, const U &Alpha ) noexcept
    {
        return static_cast<T>( Start + ( ( End - Start ) * Alpha ) );
    }

    //! Computes the bi-linear interpolation between 4 points forming 2 lines
    template<std::TBasicMathEnabled T, std::TFloat U>
    SKL_FORCEINLINE SKL_NODISCARD inline constexpr T BiLerp( const T &Point_00
                                      , const T &Point_10
                                      , const T &Point_01
                                      , const T &Point_11
                                      , const U &Alpha_BetweenPoints
                                      , const U &Alpha_Of_Individual_Lerps ) noexcept
    {
        return Lerp( Lerp( Point_00, Point_10, Alpha_BetweenPoints )
                   , Lerp( Point_01, Point_11, Alpha_BetweenPoints )
                   , Alpha_Of_Individual_Lerps );
    }

    //! Computes the cubic interpolation
    template<std::TBasicMathEnabled T, std::TFloat U>
    SKL_FORCEINLINE SKL_NODISCARD inline constexpr T CubicInterp( const T &P0
                                           , const T &T0
                                           , const T &P1
                                           , const T &T1
                                           , const U &Alpha ) noexcept
    {
        const U AlphaSquared{ Square( Alpha ) };
        const U AlphaCubed  { AlphaSquared * Alpha };

        return static_cast<T>(
            ( ( ( 2 * AlphaCubed ) - ( 3 * AlphaSquared ) + 1 ) * P0 ) +
            ( ( AlphaCubed - ( 2 * AlphaSquared ) + Alpha ) * T0 ) + ( ( AlphaCubed - AlphaSquared ) * T1 ) +
            ( ( ( -2 * AlphaCubed ) + ( 3 * AlphaSquared ) ) * P1 ) );
    }

    //! Computes the first derivative of the cubic interpolation function
    template<std::TBasicMathEnabled T, std::TFloat U>
    SKL_FORCEINLINE SKL_NODISCARD inline constexpr T CubicInterpDerivative( const T &P0
                                                     , const T &T0
                                                     , const T &P1
                                                     , const T &T1
                                                     , const U &Alpha ) noexcept
    {
        const U A { ( static_cast<U>( 6.0 ) * P0 ) 
                  + ( static_cast<U>( 3.0 ) * T0 ) 
                  + ( static_cast<U>( 3.0 ) * T1 ) 
                  - ( static_cast<U>( 6.0 ) * P1 ) };

        const U B { ( static_cast<U>( -6.0 ) * P0 ) 
                  - ( static_cast<U>( 4.0 ) * T0 ) 
                  - ( static_cast<U>( 2.0 ) * T1 ) 
                  + ( static_cast<U>( 6.0 ) * P1 ) };

        const T C            { T0 };
        const U AlphaSquared { Square( Alpha ) };

        return ( A * AlphaSquared ) + ( B * Alpha ) + C;
    }

    //! Computes the second derivative of the cubic interpolation function
    template<std::TBasicMathEnabled T, std::TFloat U>
    SKL_FORCEINLINE SKL_NODISCARD inline constexpr T CubicInterpSecondDerivative( const T &P0
                                                           , const T &T0
                                                           , const T &P1
                                                           , const T &T1
                                                           , const U &Alpha ) noexcept
    {
        const U A { ( static_cast<U>( 12.0 ) * P0 ) 
                  + ( static_cast<U>( 6.0 ) * T0 ) 
                  + ( static_cast<U>( 6.0 ) * T1 ) 
                  - ( static_cast<U>( 12.0 ) * P1 ) };

        const U B { ( static_cast<U>( -6.0 ) * P0 ) 
                  - ( static_cast<U>( 4.0 ) * T0 ) 
                  - ( static_cast<U>( 2.0 ) * T1 ) 
                  + ( static_cast<U>( 6.0 ) * P1 ) };

        return ( A * Alpha ) + B;
    }

    //! Computes ease-in-out linear interpolation. @Exp controls the degree of the curve.
    template<std::TBasicMathEnabled T, std::TFloat U>
    SKL_FORCEINLINE SKL_NODISCARD inline constexpr T InterpEaseInOut( const T &Start
                                                             , const T &End
                                                             , const U &Alpha
                                                             , const U &Exp ) noexcept
    {
        constexpr U CZeroPointFive = static_cast<U>( 0.5 );
        constexpr U CTwoPointZero  = static_cast<U>( 2.0 );
        constexpr U COnePointZero  = static_cast<U>( 1.0 );

        U NewAlpha;

        if( Alpha < CZeroPointFive )
        {
            NewAlpha = CZeroPointFive * Pow( CTwoPointZero * Alpha, Exp );
        }
        else
        {
            NewAlpha = ( COnePointZero - CZeroPointFive ) * Pow( CTwoPointZero * ( COnePointZero - Alpha ), Exp );
        }
        
        return Lerp( Start, End, NewAlpha );
    }
} // namespace SMath
