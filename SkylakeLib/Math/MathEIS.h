//!
//! \file MathEIS.h
//! 
//! \brief Math functions implemented using EIS(Enhanced instruction set)
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

#if defined(SKL_USE_AVX2)
#   include <immintrin.h>
#endif

namespace SKL
{
#if defined(SKL_USE_AVX2)
    static SKL_FORCEINLINE float InvSqrtInternal( float InValue ) noexcept
    {
        const __m128 One = _mm_set_ss( 1.0f );
        const __m128 Y0     = _mm_set_ss( InValue );
        const __m128 X0     = _mm_sqrt_ss( Y0 );
        const __m128 R0     = _mm_div_ss( One, X0 );
        float         temp;
        _mm_store_ss( &temp, R0 );
        return temp;
    }

    static SKL_FORCEINLINE double InvSqrtInternal( double InValue ) noexcept
    {
        const __m128d One = _mm_set_sd( 1.0 );
        const __m128d Y0  = _mm_set_sd( InValue );
        const __m128d X0  = _mm_sqrt_sd( One, Y0 );
        const __m128d R0  = _mm_div_sd( One, X0 );
        double          temp;
        _mm_store_sd( &temp, R0 );
        return temp;
    }

    static SKL_FORCEINLINE float InvSqrtEstInternal( float F ) noexcept
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

    static SKL_FORCEINLINE double InvSqrtEstInternal( double InValue ) noexcept
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
        return _mm_cvtsd_si64( _mm_set_sd( static_cast<double>( Value ) + ( Value - 0.5 ) ) ) >> 1;
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
#elif defined(SKL_USE_AVX_512)
#   error "Implement using AVX 512"
#else
    #   error "Implement using no EIS"
#endif
}
