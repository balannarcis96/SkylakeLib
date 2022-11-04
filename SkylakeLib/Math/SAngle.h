//!
//! \file SAngle.h
//! 
//! \brief Angle math abstraction for SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    struct SAngle
    {
        int16_t Angle;

        SAngle() noexcept : Angle { 0 } { }
        SAngle( int16_t Angle ) noexcept :Angle( Angle ) { }

        SKL_FORCEINLINE SAngle operator+( const SAngle& Other ) const noexcept
        {
            return SAngle( Angle + Other.Angle );
        }
        SKL_FORCEINLINE SAngle operator-( const SAngle& Other ) const noexcept
        {
            return SAngle( Angle - Other.Angle );
        }
        SKL_FORCEINLINE SAngle operator*( const SAngle& Other ) const noexcept
        {
            return SAngle( Angle * Other.Angle );
        }
        SKL_FORCEINLINE SAngle operator/( const SAngle& Other ) const noexcept
        {
            return SAngle( Angle / Other.Angle );
        }
        SKL_FORCEINLINE SKL_NODISCARD bool operator<( const SAngle& Other ) const noexcept
        {
            return ( Angle < Other.Angle );
        }
        SKL_FORCEINLINE SKL_NODISCARD bool operator<=( const SAngle& Other ) const noexcept
        {
            return ( Angle <= Other.Angle );
        }
        SKL_FORCEINLINE SKL_NODISCARD bool operator>( const SAngle& Other ) const noexcept
        {
            return ( Angle > Other.Angle );
        }
        SKL_FORCEINLINE SKL_NODISCARD bool operator>=( const SAngle& Other ) const noexcept
        {
            return ( Angle >= Other.Angle );
        }
        SKL_FORCEINLINE SKL_NODISCARD bool operator==( const SAngle& Other ) const noexcept
        {
            return ( Angle == Other.Angle );
        }
        
        SKL_FORCEINLINE void SetValue( const int16_t Value ) noexcept
        {
            Angle = Value;
        }
        SKL_FORCEINLINE void SetFromRad( float Value ) noexcept
        {
            Angle = static_cast<int16_t>( Value * RadToUnit_f );
        }
        SKL_FORCEINLINE void SetFromDeg( float Value ) noexcept
        {
            Angle = static_cast<int16_t>( Value * DegToUnit_f );
        }
        SKL_FORCEINLINE SKL_NODISCARD float ToRad( ) const noexcept
        {
            return static_cast<float>( Angle ) * UnitToRad_f;
        }
        SKL_FORCEINLINE SKL_NODISCARD float ToDeg( ) const noexcept
        {
            return static_cast<float>( Angle ) * UnitToDeg_f;
        }

        SKL_FORCEINLINE SKL_NODISCARD static SAngle NewFromRad( float Value ) noexcept
        {
            return SAngle( static_cast<int16_t>( Value * RadToUnit_f ) );
        }
        SKL_FORCEINLINE SKL_NODISCARD static SAngle NewFromDeg( float Value ) noexcept
        {
            return SAngle( static_cast<int16_t>( Value * DegToUnit_f ) );
        }
    };
}