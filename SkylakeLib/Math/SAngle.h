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

		SAngle( )noexcept : Angle { 0 } { }
		SAngle( int16_t _angle ) noexcept :Angle( _angle ) { }
		explicit SAngle( float _degAngle ) noexcept :Angle( 0 )
		{
			FromDeg( _degAngle );
		}
		explicit SAngle( float _radAngle, bool explicitDummy ) noexcept :Angle( 0 )
		{
			FromRad( _radAngle );
		}

		SKL_FORCEINLINE void SetValue( const uint16_t val ) noexcept
		{
			Angle = val;
		}

		SKL_FORCEINLINE int16_t Add( const SAngle& other ) noexcept
		{
			return ( Angle += other.Angle );
		}
		SKL_FORCEINLINE int16_t Sub( const SAngle& other ) noexcept
		{
			return ( Angle -= other.Angle );
		}
		SKL_FORCEINLINE SAngle operator+( const SAngle& other ) const noexcept
		{
			return int16_t( Angle + other.Angle );
		}
		SKL_FORCEINLINE SAngle operator-( const SAngle& other ) const noexcept
		{
			return int16_t( Angle - other.Angle );
		}
		SKL_FORCEINLINE SAngle operator*( const SAngle& other ) const noexcept
		{
			return int16_t( Angle * other.Angle );
		}
		SKL_FORCEINLINE SAngle operator/( const SAngle& other ) const noexcept
		{
			return int16_t( Angle / other.Angle );
		}
		SKL_FORCEINLINE double operator*( const double scale ) const noexcept
		{
			return Angle * scale;
		}
		SKL_FORCEINLINE bool operator<( const SAngle& other ) const noexcept
		{
			return ( Angle < other.Angle );
		}
		SKL_FORCEINLINE bool operator<=( const SAngle& other ) const noexcept
		{
			return ( Angle <= other.Angle );
		}
		SKL_FORCEINLINE bool operator>( const SAngle& other ) const noexcept
		{
			return ( Angle > other.Angle );
		}
		SKL_FORCEINLINE bool operator>=( const SAngle& other ) const noexcept
		{
			return ( Angle >= other.Angle );
		}
		SKL_FORCEINLINE bool operator==( const SAngle& other ) const noexcept
		{
			return ( Angle == other.Angle );
		}
		SKL_FORCEINLINE int16_t FromRad( float rad ) noexcept
		{
			return ( Angle = static_cast< int16_t >( rad * RadToUnit ) );
		}
		SKL_FORCEINLINE int16_t FromDeg( float deg ) noexcept
		{
			return ( Angle = static_cast< int16_t >( deg * DegToUnit ) );
		}
		SKL_FORCEINLINE float ToRad( ) const noexcept
		{
			return ( float )( Angle * UnitToRad );
		}
		SKL_FORCEINLINE float ToDeg( ) const noexcept
		{
			return ( float )( Angle * UnitToDeg );
		}
	};
}