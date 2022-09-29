//!
//! \file SMathUtils.h
//! 
//! \brief Math utils for SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    //! Snaps a value to the nearest grid multiple.
    template< std::TFloat T >
    SKL_FORCEINLINE T SnapToGrid( T Location, T Grid ) noexcept
    {
        if( Grid == static_cast< T >( 0.0 ) )
            return Location;

        return FFloor( ( ( Location + static_cast< T >( 0.5 ) * Grid ) / Grid ) * Grid );
    }

    //! Find the smallest angle between two headings (in radians).
    template< std::TFloat T >
    SKL_FORCEINLINE constexpr T FindDeltaAngle( T Angle1, T Angle2 ) noexcept
    {
        // Find the difference
        T Delta = Angle2 - Angle1;

        // If change is larger than PI
        if( Delta > static_cast< T >( static_cast< T >( PI ) ) )
        {
            // Flip to negative equivalent
            Delta = Delta - static_cast< T >( static_cast< T >( PI ) * static_cast< T >( 2.0 ) );
        }
        else if( Delta < -static_cast< T >( static_cast< T >( PI ) ) )
        {
            // Otherwise, if change is smaller than -PI
            // Flip to positive equivalent
            Delta = Delta + static_cast< T >( static_cast< T >( PI ) * static_cast< T >( 2.0 ) );
        }

        // Return delta in [-PI,PI] range
        return Delta;
    }

    //! Utility to ensure angle is between +/- 180 degrees by unwinding.
    template< typename T >
    void UnwindDegreeComponent( T &A ) requires( std::is_same_v< T, float > || std::is_same_v< T, double > )
    {
        while( A > static_cast< T >( 180.0 ) )
        {
            A -= static_cast< T >( 360.0 );
        }

        while( A < -static_cast< T >( 180.0 ) )
        {
            A += static_cast< T >( 360.0 );
        }
    }
}