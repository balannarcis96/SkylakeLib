//!
//! \file Math.h
//! 
//! \brief All math abstractions and constants for SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

#if SKL_USE_LARGE_WORLD_COORDS
    #define SKL_REAL_VALUE( x ) ( x )
#else
    #define SKL_REAL_VALUE( x ) ( x##f )
#endif

#if SKL_USE_LARGE_WORLD_COORDS
    using skReal = double;
#else
    using skReal = float;
#endif

#undef PI
#define PI SKL_REAL_VALUE( 3.1415926535897932 )
#define SMALL_NUMBER SKL_REAL_VALUE( 1.e-8 )
#define KINDA_SMALL_NUMBER SKL_REAL_VALUE( 1.e-4 )
#define BIG_NUMBER SKL_REAL_VALUE( 3.4e+38 )
#define EULERS_NUMBER SKL_REAL_VALUE( 2.71828182845904523536 )
#define RadToUnit ( SKL_REAL_VALUE( 32767.0 ) / PI )
#define UnitToRad ( PI / SKL_REAL_VALUE( 32767.0 ) )
#define DegToUnit ( SKL_REAL_VALUE( 32767.0 ) / SKL_REAL_VALUE( 180.0 ) )
#define UnitToDeg ( SKL_REAL_VALUE( 180.0 ) / SKL_REAL_VALUE( 32767.0 ) )
#define RadToDeg ( SKL_REAL_VALUE( 180 ) / PI )
#define DegToRad ( PI / SKL_REAL_VALUE( 180 ) )
#define SK_REAL_ZERO SKL_REAL_VALUE( 0.0 )
#define SK_REAL_ONE SKL_REAL_VALUE( 1.0 )

#ifndef INDEX_NONE
    #define INDEX_NONE ( -1 )
#endif

// Aux constants.
#define INV_PI ( SKL_REAL_VALUE( 0.31830988618 ) )
#define HALF_PI ( SKL_REAL_VALUE( 1.57079632679 ) )

#define DELTA ( SKL_REAL_VALUE( 0.00001 ) )

#define THRESH_POINT_ON_PLANE ( SKL_REAL_VALUE( 0.10 ) )        /* Thickness of plane for front/back/inside test */
#define THRESH_POINT_ON_SIDE ( SKL_REAL_VALUE( 0.20 ) )         /* Thickness of polygon side's side-plane for point-inside/outside/on side test */
#define THRESH_POINTS_ARE_SAME ( SKL_REAL_VALUE( 0.002 ) )      /* Two points are same if within this distance */
#define THRESH_POINTS_ARE_NEAR ( SKL_REAL_VALUE( 0.015 ) )      /* Two points are near if within this distance and can be combined if imprecise math is ok */
#define THRESH_NORMALS_ARE_SAME ( SKL_REAL_VALUE( 0.00002 ) )   /* Two normal points are same if within this distance */
#define THRESH_VECTORS_ARE_NEAR ( SKL_REAL_VALUE( 0.0004 ) )    /* Two vectors are near if within this distance and can be combined if imprecise math is ok */
#define THRESH_SPLIT_POLY_WITH_PLANE ( SKL_REAL_VALUE( 0.25 ) ) /* A plane splits a polygon in half */
#define THRESH_SPLIT_POLY_PRECISELY ( SKL_REAL_VALUE( 0.01 ) )  /* A plane exactly splits a polygon */
#define THRESH_ZERO_NORM_SQUARED ( SKL_REAL_VALUE( 0.0001 ) )   /* Size of a unit normal that is considered "zero", squared */
#define THRESH_VECTORS_ARE_PARALLEL ( SKL_REAL_VALUE( 0.02 ) )  /* Vectors are parallel if dot product varies less than this */

constexpr skReal CRealZero     = SK_REAL_ZERO;
constexpr skReal CRealUnit     = SK_REAL_ONE;
constexpr skReal CPI           = PI;
constexpr skReal CSmallNumber  = SMALL_NUMBER;
constexpr skReal CBigNumber    = BIG_NUMBER;
constexpr skReal CEulersNumber = EULERS_NUMBER;
constexpr skReal CRadToUnit    = RadToUnit;
constexpr skReal CUnitToRad    = UnitToRad;
constexpr skReal CDegToUnit    = DegToUnit;
constexpr skReal CUnitToDeg    = UnitToDeg;
constexpr skReal CRadToDeg     = RadToDeg;
constexpr skReal CDegToRad     = DegToRad;

#if defined(SKL_USE_SSE)
	#include <emmintrin.h>
#endif

namespace SKL
{
    using RandomTypeToUse = GRand;
}

#include "SAngle.h"
#include "SMathTemplates.h"
#include "SMathUtils.h"

namespace SKL
{
    struct SGlobalMath
    {
        // Constants.
        enum
        {
            ANGLE_SHIFT = 2
        }; // Bits to right-shift to get lookup value.
        enum
        {
            ANGLE_BITS = 14
        }; // Number of valid bits in angles.
        enum
        {
            NUM_ANGLES = 16384
        }; // Number of angles that are in lookup table.
        enum
        {
            ANGLE_MASK = ( ( ( 1 << ANGLE_BITS ) - 1 ) << ( 16 - ANGLE_BITS ) )
        };

        // Basic math functions.
        SKL_FORCEINLINE skReal SinTab( int i ) const noexcept
        {
            return TrigFLOAT[ ( ( i >> ANGLE_SHIFT ) & ( NUM_ANGLES - 1 ) ) ];
        }
        SKL_FORCEINLINE skReal CosTab( int i ) const noexcept
        {
            return TrigFLOAT[ ( ( ( i + 16384 ) >> ANGLE_SHIFT ) & ( NUM_ANGLES - 1 ) ) ];
        }
        SKL_FORCEINLINE skReal SinFloat( skReal F ) const noexcept
        {
            return SinTab( FTrunc( ( F * SKL_REAL_VALUE( 65536.0 ) ) / ( SKL_REAL_VALUE( 2.0 ) * PI ) ) );
        }
        SKL_FORCEINLINE skReal CosFloat( skReal F ) const noexcept
        {
            return CosTab( FTrunc( ( F * SKL_REAL_VALUE( 65536.0 ) ) / ( SKL_REAL_VALUE( 2.0 ) * PI ) ) );
        }

        // Constructor.
        SGlobalMath( ) noexcept;

        SKL_FORCEINLINE skReal GetCachedUnit( int32_t Index ) const noexcept
        {
            return CachedUnitAngles[ Index ];
        }
        SKL_FORCEINLINE skReal GetCachedCos( int16_t Angle ) const noexcept
        {
            return CachedUnitAngles[ 98304 + Angle ];
        }
        SKL_FORCEINLINE skReal GetCachedSin( int16_t Angle ) const noexcept
        {
            return CachedUnitAngles[ Angle + 32768 ];
        }

    private:
        // Tables.
        skReal TrigFLOAT[ NUM_ANGLES ]{ SK_REAL_ZERO };
        skReal CachedUnitAngles[ ( UINT16_MAX + 1 ) * 2 ]{ SK_REAL_ZERO };
    };
    extern SGlobalMath GSGlobalMath;

    template< std::TSInteger T >
    struct TIntPoint
    {
        using TType  = T;
        using MyType = TIntPoint< T >;

        TType X, Y;

        TIntPoint( ) noexcept :
            X{ 0 }, Y{ 0 }
        {
        }
        TIntPoint( TType InX, TType InY ) noexcept :
            X( InX ), Y( InY )
        {
        }
        TIntPoint( const skReal Vector[ 3 ] ) noexcept :
            X( static_cast< TType >( Vector[ 0 ] ) ), Y( static_cast< TType >( Vector[ 1 ] ) )
        {
        }

        SKL_FORCEINLINE static MyType ZeroValue( ) noexcept
        {
            return MyType{ 0, 0 };
        }
        SKL_FORCEINLINE static MyType NoneValue( ) noexcept
        {
            return MyType{ INDEX_NONE, INDEX_NONE };
        }

        SKL_FORCEINLINE bool operator==( const MyType &Other ) const noexcept
        {
            return X == Other.X && Y == Other.Y;
        }
        SKL_FORCEINLINE bool operator!=( const MyType &Other ) const noexcept
        {
            return X != Other.X || Y != Other.Y;
        }
        SKL_FORCEINLINE MyType &operator*=( TType Scale ) noexcept
        {
            X *= Scale;
            Y *= Scale;
            return *this;
        }
        SKL_FORCEINLINE MyType &operator+=( const MyType &Other ) noexcept
        {
            X += Other.X;
            Y += Other.Y;
            return *this;
        }
        SKL_FORCEINLINE MyType &operator-=( const MyType &Other ) noexcept
        {
            X -= Other.X;
            Y -= Other.Y;
            return *this;
        }
        SKL_FORCEINLINE MyType &operator+=( TType Value ) noexcept
        {
            X += Value;
            Y += Value;
            return *this;
        }
        SKL_FORCEINLINE MyType &operator-=( TType Value ) noexcept
        {
            X -= Value;
            Y -= Value;
            return *this;
        }
        SKL_FORCEINLINE MyType operator*( MyType Scale ) const noexcept
        {
            return MyType( *this ) *= Scale;
        }
        SKL_FORCEINLINE MyType operator+( const MyType &Other ) const noexcept
        {
            return MyType( *this ) += Other;
        }
        SKL_FORCEINLINE MyType operator-( const MyType &Other ) const noexcept
        {
            return MyType( *this ) -= Other;
        }
        SKL_FORCEINLINE MyType Size( ) const noexcept
        {
            return static_cast< MyType >( Sqrt( static_cast< skReal >( X * X + Y * Y ) ) );
        }
    };
    using SIntPoint   = TIntPoint< int32_t >;
    using SInt64Point = TIntPoint< int64_t >;

    template< std::TSInteger T >
    struct TIntRect
    {
        using TType    = T;
        using MyType   = TIntRect< T >;
        using MyTPoint = TIntPoint< TType >;

        MyTPoint Min, Max;

        TIntRect( ) noexcept = default;
        TIntRect( TType X0, TType Y0, TType X1, TType Y1 ) noexcept :
            Min( X0, Y0 ), Max( X1, Y1 )
        {
        }
        TIntRect( MyTPoint InMin, MyTPoint InMax ) noexcept :
            Min( InMin ), Max( InMax )
        {
        }

        SKL_FORCEINLINE bool operator==( const MyType &Other ) const noexcept
        {
            return Min == Other.Min && Max == Other.Max;
        }
        SKL_FORCEINLINE bool operator!=( const MyType &Other ) const noexcept
        {
            return Min != Other.Min || Max != Other.Max;
        }
        SKL_FORCEINLINE MyType Right( TType InWidth ) const noexcept
        {
            return MyType{ Max( Min.X, Max.X - InWidth ), Min.Y, Max.X, Max.Y };
        }
        SKL_FORCEINLINE MyType Bottom( TType InHeight ) const noexcept
        {
            return MyType{ Min.X, Max( Min.Y, Max.Y - InHeight ), Max.X, Max.Y };
        }
        SKL_FORCEINLINE MyTPoint Size( ) const noexcept
        {
            return MyTPoint{ Max.X - Min.X, Max.Y - Min.Y };
        }
        SKL_FORCEINLINE TType Width( ) const noexcept
        {
            return Max.X - Min.X;
        }
        SKL_FORCEINLINE TType Height( ) const noexcept
        {
            return Max.Y - Min.Y;
        }
        SKL_FORCEINLINE MyType Expand( TType Value ) const noexcept
        {
            return {
                { Min.X - Value, Min.Y - Value },
                { Max.X + Value, Max.Y + Value }
            };
        }
        SKL_FORCEINLINE MyType &operator*=( TType Scale ) noexcept
        {
            Min *= Scale;
            Max *= Scale;
            return *this;
        }
        SKL_FORCEINLINE MyType &operator+=( const MyTPoint &P ) noexcept
        {
            Min += P;
            Max += P;
            return *this;
        }
        SKL_FORCEINLINE MyType &operator-=( const MyTPoint &P ) noexcept
        {
            Min -= P;
            Max -= P;
            return *this;
        }
        SKL_FORCEINLINE MyType operator*( TType Scale ) const noexcept
        {
            return MyType{ Min * Scale, Max * Scale };
        }
        SKL_FORCEINLINE MyType operator+( const MyTPoint &P ) const noexcept
        {
            return MyType{ Min + P, Max + P };
        }
        SKL_FORCEINLINE MyType operator-( const MyTPoint &P ) const noexcept
        {
            return MyType{ Min - P, Max - P };
        }
        SKL_FORCEINLINE MyType operator+( const MyType &R ) const noexcept
        {
            return MyType{ Min + R.Min, Max + R.Max };
        }
        SKL_FORCEINLINE MyType operator-( const MyType &R ) const noexcept
        {
            return MyType{ Min - R.Min, Max - R.Max };
        }
        SKL_FORCEINLINE MyType Inner( MyTPoint P ) const noexcept
        {
            return MyType{ Min + P, Max - P };
        }
        SKL_FORCEINLINE bool Contains( MyTPoint P ) const noexcept
        {
            return ( P.X >= Min.X ) &&
                ( Max.X > P.X ) &&
                ( P.Y >= Min.Y ) &&
                ( Max.Y > P.Y );
        }
        SKL_FORCEINLINE TType Area( ) const noexcept
        {
            return ( Max.X - Min.X ) * ( Max.Y - Min.Y );
        }
        void GetCenterAndExtents( MyTPoint &Center, MyTPoint &Extent ) const noexcept
        {
            Extent.X = ( Max.X - Min.X ) / 2;
            Extent.Y = ( Max.Y - Min.Y ) / 2;

            Center.X = Min.X + Extent.X;
            Center.Y = Min.Y + Extent.Y;
        }
        void Clip( const MyType &R ) noexcept
        {
            Min.X = Max( Min.X, R.Min.X );
            Min.Y = Max( Min.Y, R.Min.Y );
            Max.X = Min( Max.X, R.Max.X );
            Max.Y = Min( Max.Y, R.Max.Y );

            // Adjust to zero area if the rects don't overlap.
            Max.X = Max( Min.X, Max.X );
            Max.Y = Max( Min.Y, Max.Y );
        }
        SKL_FORCEINLINE void SetWidth( TType width ) noexcept
        {
            Max.X = Min.X + width;
        }
        SKL_FORCEINLINE void SetHeight( TType height ) noexcept
        {
            Max.Y = Min.Y + height;
        }
        SKL_FORCEINLINE TType X( ) const noexcept
        {
            return Min.X;
        }
        SKL_FORCEINLINE TType Y( ) const noexcept
        {
            return Min.Y;
        }
        SKL_FORCEINLINE void ExpandFromCenter( TType Value ) noexcept
        {
            Max += Value;
            Min -= Value;
        }

        _NODISCARD bool Intersect( const MyType &Other ) const noexcept
        {
            if( Min.X > Other.Max.X || Other.Min.X > Max.X )
                return false;

            if( Min.Y > Other.Max.Y || Other.Min.Y > Max.Y )
                return false;

            return true;
        }
    };
    using SIntRect   = TIntRect< int32_t >;
    using SInt64Rect = TIntRect< int64_t >;

    template< typename TReal >
    requires( std::is_floating_point_v< TReal > ) struct TVector2D
    {
        template< typename TOtherReal >
        static consteval TReal ToMyRealType( TOtherReal InOtherReal ) noexcept
        {
            return static_cast< TReal >( InOtherReal );
        }

        // Zero vector.
        static TVector2D const ZeroVector;

        // Unit vector.
        static TVector2D const UnitVector;

        static constexpr TReal ZeroValue = ToMyRealType( 0.0 );
        static constexpr TReal UnitValue = ToMyRealType( 1.0 );

        TReal
            X,
            Y;

        // Constructors.
        TVector2D( ) noexcept :
            X{ ToMyRealType( 0.0 ) }, Y{ ToMyRealType( 0.0 ) }
        {
        }
        TVector2D( TReal InX, TReal InY ) noexcept :
            X( InX ), Y( InY )
        {
        }

        template< typename TOtherTReal >
        TVector2D( TOtherTReal InX, TOtherTReal InY ) noexcept requires( std::is_floating_point_v< TOtherTReal > && !std::is_same_v< TOtherTReal, TReal > ) :
            X{ ToMyRealType( InX ) }, Y{ ToMyRealType( InY ) }
        {
        }

        // Binary math operators.
        SKL_FORCEINLINE TVector2D operator+( const TVector2D &V ) const noexcept
        {
            return TVector2D{ X + V.X, Y + V.Y };
        }
        SKL_FORCEINLINE TVector2D operator-( const TVector2D &V ) const noexcept
        {
            return TVector2D{ X - V.X, Y - V.Y };
        }
        SKL_FORCEINLINE TVector2D operator*( TReal Scale ) const noexcept
        {
            return TVector2D{ X * Scale, Y * Scale };
        }
        SKL_FORCEINLINE TVector2D operator/( TReal Scale ) const noexcept
        {
            const TReal RScale = UnitValue / Scale;
            return TVector2D{ X * RScale, Y * RScale };
        }
        SKL_FORCEINLINE TVector2D operator*( const TVector2D &V ) const noexcept
        {
            return TVector2D{ X * V.X, Y * V.Y };
        }
        SKL_FORCEINLINE TReal operator|( const TVector2D &V ) const noexcept
        {
            return X * V.X + Y * V.Y;
        }
        SKL_FORCEINLINE TReal operator^( const TVector2D &V ) const noexcept
        {
            return X * V.Y - Y * V.X;
        }
        SKL_FORCEINLINE bool operator==( const TVector2D &V ) const noexcept
        {
            return X == V.X && Y == V.Y;
        }
        SKL_FORCEINLINE bool operator!=( const TVector2D &V ) const noexcept
        {
            return X != V.X || Y != V.Y;
        }
        SKL_FORCEINLINE bool operator<( const TVector2D &Other ) const noexcept
        {
            return X < Other.X && Y < Other.Y;
        }
        SKL_FORCEINLINE bool operator>( const TVector2D &Other ) const noexcept
        {
            return X > Other.X && Y > Other.Y;
        }
        SKL_FORCEINLINE bool operator<=( const TVector2D &Other ) const noexcept
        {
            return X <= Other.X && Y <= Other.Y;
        }
        SKL_FORCEINLINE bool operator>=( const TVector2D &Other ) const noexcept
        {
            return X >= Other.X && Y >= Other.Y;
        }
        // Error-tolerant comparison.
        _NODISCARD bool Equals( const TVector2D &V, TReal Tolerance = ToMyRealType( KINDA_SMALL_NUMBER ) ) const noexcept
        {
            return Abs( X - V.X ) < Tolerance && Abs( Y - V.Y ) < Tolerance;
        }

        // Unary operators.
        SKL_FORCEINLINE TVector2D operator-( ) const noexcept
        {
            return TVector2D{ -X, -Y };
        }
        // Assignment operators.
        SKL_FORCEINLINE TVector2D operator+=( const TVector2D &V ) noexcept
        {
            X += V.X;
            Y += V.Y;
            return *this;
        }
        SKL_FORCEINLINE TVector2D operator-=( const TVector2D &V ) noexcept
        {
            X -= V.X;
            Y -= V.Y;
            return *this;
        }
        SKL_FORCEINLINE TVector2D operator*=( TReal Scale ) noexcept
        {
            X *= Scale;
            Y *= Scale;
            return *this;
        }
        SKL_FORCEINLINE TVector2D operator/=( TReal V ) noexcept
        {
            const TReal RV = UnitValue / V;
            X *= RV;
            Y *= RV;
            return *this;
        }
        SKL_FORCEINLINE TVector2D operator*=( const TVector2D &V ) noexcept
        {
            X *= V.X;
            Y *= V.Y;
            return *this;
        }
        SKL_FORCEINLINE TVector2D operator/=( const TVector2D &V ) noexcept
        {
            X /= V.X;
            Y /= V.Y;
            return *this;
        }
        SKL_FORCEINLINE TReal &operator[]( int32_t i ) noexcept
        {
            if( i == 0 )
                return X;
            else
                return Y;
        }
        SKL_FORCEINLINE TReal operator[]( int32_t i ) const noexcept
        {
            return ( ( i == 0 ) ? X : Y );
        }
        // Simple functions.
        void Set( TReal InX, TReal InY ) noexcept
        {
            X = InX;
            Y = InY;
        }
        SKL_FORCEINLINE TReal GetMax( ) const noexcept
        {
            return Max( X, Y );
        }
        SKL_FORCEINLINE TReal GetAbsMax( ) const noexcept
        {
            return Max( Abs( X ), Abs( Y ) );
        }
        SKL_FORCEINLINE TReal GetMin( ) const noexcept
        {
            return Min( X, Y );
        }
        SKL_FORCEINLINE TReal Size( ) const noexcept
        {
            return Sqrt( X * X + Y * Y );
        }
        SKL_FORCEINLINE TReal SizeSquared( ) const noexcept
        {
            return X * X + Y * Y;
        }

        _NODISCARD TVector2D SafeNormal( TReal Tolerance = ToMyRealType( SMALL_NUMBER ) ) const noexcept
        {
            const TReal SquareSum = X * X + Y * Y;
            if( SquareSum > Tolerance )
            {
                const TReal Scale = InverseSqrt( SquareSum );
                return TVector2D{ X * Scale, Y * Scale };
            }
            return TVector2D{ ZeroValue, ZeroValue };
        }
        void Normalize( TReal Tolerance = ToMyRealType( SMALL_NUMBER ) ) noexcept
        {
            const TReal SquareSum = X * X + Y * Y;
            if( SquareSum > Tolerance )
            {
                const TReal Scale = InverseSqrt( SquareSum );
                X *= Scale;
                Y *= Scale;
                return;
            }

            X = ZeroValue;
            Y = ZeroValue;
        }
        _NODISCARD int32_t IsNearlyZero( TReal Tolerance = ToMyRealType( KINDA_SMALL_NUMBER ) ) const noexcept
        {
            return Abs( X ) < Tolerance && Abs( Y ) < Tolerance;
        }
        _NODISCARD SKL_FORCEINLINE bool IsZero( ) const noexcept
        {
            return X == ZeroValue && Y == ZeroValue;
        }
        SKL_FORCEINLINE skReal &Component( int32_t Index ) noexcept
        {
            return ( &X )[ Index ];
        }

        SKL_FORCEINLINE void Lerp( TVector2D &b, TReal t ) noexcept
        {
            X = ( ( b.X - X ) * t );
            Y = ( ( b.Y - Y ) * t );
        }
    };
    using SVector2D  = TVector2D< skReal >;
    using SVector2Df = TVector2D< float >;
    using SVector2Dd = TVector2D< double >;

    template< typename TReal >
    requires( std::is_floating_point_v< TReal > ) struct TVector
    {
        template< typename TOtherReal >
        static consteval TReal ToMyRealType( TOtherReal InOtherReal ) noexcept
        {
            return static_cast< TReal >( InOtherReal );
        }

        // Zero vector.
        static TVector const ZeroVector;

        // Unit vector.
        static TVector const UnitVector;

        static constexpr TReal ZeroValue = ToMyRealType( 0.0 );
        static constexpr TReal UnitValue = ToMyRealType( 1.0 );

        TReal
            X,
            Y,
            Z;

        // Constructors.
        TVector( ) noexcept :
            X( ZeroValue ), Y( ZeroValue ), Z( ZeroValue )
        {
        }

        explicit TVector( TReal InF ) noexcept :
            X( InF ), Y( InF ), Z( InF )
        {
        }

        TVector( TReal InX, TReal InY, TReal InZ ) noexcept :
            X( InX ), Y( InY ), Z( InZ )
        {
        }

        explicit TVector( const TReal *vec ) noexcept :
            X( vec[ 0 ] ), Y( vec[ 1 ] ), Z( vec[ 2 ] )
        {
        }

        template< typename TOtherTReal >
        TVector( TOtherTReal InX, TOtherTReal InY, TOtherTReal InZ ) noexcept requires( std::is_floating_point_v< TOtherTReal > && !std::is_same_v< TOtherTReal, TReal > ) :
            X{ ToMyRealType( InX ) }, Y{ ToMyRealType( InY ) }, Z{ ToMyRealType( InZ ) }
        {
        }

        template< typename TOtherTReal >
        explicit TVector( const TVector< TOtherTReal > &Other ) noexcept requires( std::is_floating_point_v< TOtherTReal > ) :
            X( static_cast< TReal >( Other.X ) ), Y( static_cast< TReal >( Other.Y ) ), Z( static_cast< TReal >( Other.Z ) )
        {
        }

        // Binary math operators.
        SKL_FORCEINLINE TVector operator^( const TVector &V ) const noexcept
        {
            return {
                Y * V.Z - Z * V.Y,
                Z * V.X - X * V.Z,
                X * V.Y - Y * V.X
            };
        }

        SKL_FORCEINLINE TReal operator|( const TVector &V ) const noexcept
        {
            return X * V.X + Y * V.Y + Z * V.Z;
        }
        SKL_FORCEINLINE TVector operator+( const TVector &V ) const noexcept
        {
            return TVector{ X + V.X, Y + V.Y, Z + V.Z };
        }
        SKL_FORCEINLINE TVector operator-( const TVector &V ) const noexcept
        {
            return TVector{ X - V.X, Y - V.Y, Z - V.Z };
        }
        SKL_FORCEINLINE TVector operator-( TReal Bias ) const noexcept
        {
            return TVector{ X - Bias, Y - Bias, Z - Bias };
        }
        SKL_FORCEINLINE TVector operator+( TReal Bias ) const noexcept
        {
            return TVector{ X + Bias, Y + Bias, Z + Bias };
        }
        SKL_FORCEINLINE TVector operator*( TReal Scale ) const noexcept
        {
            return TVector{ X * Scale, Y * Scale, Z * Scale };
        }
        TVector operator/( TReal Scale ) const noexcept
        {
            const TReal RScale = UnitValue / Scale;
            return TVector{ X * RScale, Y * RScale, Z * RScale };
        }
        SKL_FORCEINLINE TVector operator*( const TVector &V ) const noexcept
        {
            return TVector{ X * V.X, Y * V.Y, Z * V.Z };
        }
        SKL_FORCEINLINE TVector operator/( const TVector &V ) const noexcept
        {
            return TVector{ X / V.X, Y / V.Y, Z / V.Z };
        }

        SKL_FORCEINLINE bool operator==( const TVector &V ) const noexcept
        {
            return X == V.X && Y == V.Y && Z == V.Z;
        }
        SKL_FORCEINLINE bool operator!=( const TVector &V ) const noexcept
        {
            return X != V.X || Y != V.Y || Z != V.Z;
        }

        SKL_FORCEINLINE bool Equals( const TVector &V, TReal Tolerance = ToMyRealType( KINDA_SMALL_NUMBER ) ) const noexcept
        {
            return Abs( X - V.X ) < Tolerance && Abs( Y - V.Y ) < Tolerance && Abs( Z - V.Z ) < Tolerance;
        }

        SKL_FORCEINLINE bool AllComponentsEqual( TReal Tolerance = ToMyRealType( KINDA_SMALL_NUMBER ) ) const noexcept
        {
            return Abs( X - Y ) < Tolerance && Abs( X - Z ) < Tolerance && Abs( Y - Z ) < Tolerance;
        }

        SKL_FORCEINLINE TVector operator-( ) const noexcept
        {
            return TVector{ -X, -Y, -Z };
        }

        // Assignment operators.
        SKL_FORCEINLINE TVector operator+=( const TVector &V ) noexcept
        {
            X += V.X;
            Y += V.Y;
            Z += V.Z;
            return *this;
        }
        SKL_FORCEINLINE TVector operator-=( const TVector &V ) noexcept
        {
            X -= V.X;
            Y -= V.Y;
            Z -= V.Z;
            return *this;
        }
        SKL_FORCEINLINE TVector operator*=( TReal Scale ) noexcept
        {
            X *= Scale;
            Y *= Scale;
            Z *= Scale;
            return *this;
        }
        SKL_FORCEINLINE TVector operator/=( TReal V ) noexcept
        {
            const TReal RV = UnitValue / V;
            X *= RV;
            Y *= RV;
            Z *= RV;
            return *this;
        }
        SKL_FORCEINLINE TVector operator*=( const TVector &V ) noexcept
        {
            X *= V.X;
            Y *= V.Y;
            Z *= V.Z;
            return *this;
        }
        SKL_FORCEINLINE TVector operator/=( const TVector &V ) noexcept
        {
            X /= V.X;
            Y /= V.Y;
            Z /= V.Z;
            return *this;
        }
        SKL_FORCEINLINE TReal &operator[]( int32_t i ) noexcept
        {
            if( i == 0 )
                return X;
            if( i == 1 )
                return Y;
            return Z;
        }
        SKL_FORCEINLINE TReal operator[]( int32_t i ) const noexcept
        {
            if( i == 0 )
                return X;
            if( i == 1 )
                return Y;
            return Z;
        }
        // Simple functions.
        SKL_FORCEINLINE void Set( TReal InX, TReal InY, TReal InZ ) noexcept
        {
            X = InX;
            Y = InY;
            Z = InZ;
        }
        SKL_FORCEINLINE TReal GetMax( ) const noexcept
        {
            return Max( Max( X, Y ), Z );
        }
        SKL_FORCEINLINE TReal GetAbsMax( ) const noexcept
        {
            return Max( Max( Abs( X ), Abs( Y ) ), Abs( Z ) );
        }
        SKL_FORCEINLINE TReal GetMin( ) const noexcept
        {
            return Min( Min( X, Y ), Z );
        }
        SKL_FORCEINLINE TReal Size( ) const noexcept
        {
            return Sqrt( X * X + Y * Y + Z * Z );
        }
        SKL_FORCEINLINE TReal SizeSquared( ) const noexcept
        {
            return X * X + Y * Y + Z * Z;
        }
        _NODISCARD TReal Size2D( ) const noexcept
        {
            return Sqrt( X * X + Y * Y );
        }
        SKL_FORCEINLINE TReal SizeSquared2D( ) const noexcept
        {
            return X * X + Y * Y;
        }
        _NODISCARD bool IsNearlyZero( TReal Tolerance = ToMyRealType( KINDA_SMALL_NUMBER ) ) const noexcept
        {
            return Abs( X ) < Tolerance && Abs( Y ) < Tolerance && Abs( Z ) < Tolerance;
        }
        SKL_FORCEINLINE bool IsZero( ) const noexcept
        {
            return X == ZeroValue && Y == ZeroValue && Z == ZeroValue;
        }

        SKL_FORCEINLINE bool Normalize( TReal Tolerance = ToMyRealType( SMALL_NUMBER ) ) noexcept
        {
            const TReal SquareSum = X * X + Y * Y + Z * Z;
            if( SquareSum > Tolerance )
            {
                const TReal Scale = InverseSqrt( SquareSum );
                X *= Scale;
                Y *= Scale;
                Z *= Scale;
                return true;
            }

            return false;
        }
        /**
         * Returns TRUE if Normalized.
         */
        _NODISCARD bool IsNormalized( ) const noexcept
        {
            return ( Abs( UnitValue - SizeSquared( ) ) <= ToMyRealType( 0.01 ) );
        }

        void ToDirectionAndLength( TVector &OutDir, TReal &OutLength ) const noexcept
        {
            OutLength = Size( );
            if( OutLength > ToMyRealType( SMALL_NUMBER ) )
            {
                const TReal OneOverLength = UnitValue / OutLength;
                OutDir                    = TVector( X * OneOverLength, Y * OneOverLength, Z * OneOverLength );
            }
            else
            {
                OutDir = TVector( ZeroValue, ZeroValue, ZeroValue );
            }
        }

        _NODISCARD TVector Projection( ) const noexcept
        {
            const TReal RZ = UnitValue / Z;
            return TVector{ X * RZ, Y * RZ, 1 };
        }
        SKL_FORCEINLINE TVector UnsafeNormal( ) const noexcept
        {
            const TReal Scale = InverseSqrt( X * X + Y * Y + Z * Z );
            return TVector{ X * Scale, Y * Scale, Z * Scale };
        }
        _NODISCARD TVector GridSnap( const TReal &GridSz ) const noexcept
        {
            return TVector{ SnapToGrid( X, GridSz ), SnapToGrid( Y, GridSz ), SnapToGrid( Z, GridSz ) };
        }
        SKL_FORCEINLINE TVector Move2D( const TReal distance, TReal radAngle ) const noexcept
        {
            return TVector{
                ( X + ( distance * FCos( radAngle ) ) ),
                ( Y + ( distance * FSin( radAngle ) ) ),
                Z
            };
        }
        _NODISCARD TVector BoundToCube( TReal Radius ) const noexcept
        {
            return TVector{
                Clamp( X, -Radius, Radius ),
                Clamp( Y, -Radius, Radius ),
                Clamp( Z, -Radius, Radius )
            };
        }
        void AddBounded( const TVector &V, TReal Radius = ToMyRealType( INT16_MAX ) ) noexcept
        {
            *this = ( *this + V ).BoundToCube( Radius );
        }

        /** Convert a direction vector into a 'heading' angle between +/-PI. 0 is pointing down +X. */
        SKL_FORCEINLINE TReal ToHeadingAngle( ) const noexcept
        {
            // Project Dir into Z plane.
            TVector PlaneDir = *this;
            PlaneDir.Z       = ZeroValue;
            PlaneDir         = PlaneDir.SafeNormal( );

            TReal Angle = FAcos( PlaneDir.X );

            if( PlaneDir.Y < ZeroValue )
            {
                Angle *= -UnitValue;
            }

            return Angle;
        }

        _NODISCARD TReal &Component( int32_t Index ) noexcept
        {
            return ( &X )[ Index ];
        }

        _NODISCARD bool IsUniform( TReal Tolerance = ToMyRealType( KINDA_SMALL_NUMBER ) ) const noexcept
        {
            return ( Abs( X - Y ) < Tolerance ) && ( Abs( Y - Z ) < Tolerance );
        }

        _NODISCARD TVector MirrorByVector( const TVector &MirrorNormal ) const noexcept
        {
            return *this - MirrorNormal * ( ToMyRealType( 2.0 ) * ( *this | MirrorNormal ) );
        }

        _NODISCARD TVector RotateAngleAxis( const int32_t Angle, const TVector &Axis ) const noexcept
        {
            const TReal S = ToMyRealType( GSGlobalMath.SinTab( Angle ) );
            const TReal C = ToMyRealType( GSGlobalMath.CosTab( Angle ) );

            const TReal XX = Axis.X * Axis.X;
            const TReal YY = Axis.Y * Axis.Y;
            const TReal ZZ = Axis.Z * Axis.Z;

            const TReal XY = Axis.X * Axis.Y;
            const TReal YZ = Axis.Y * Axis.Z;
            const TReal ZX = Axis.Z * Axis.X;

            const TReal XS = Axis.X * S;
            const TReal YS = Axis.Y * S;
            const TReal ZS = Axis.Z * S;

            const TReal OMC = UnitValue - C;

            return TVector{
                ( OMC * XX + C ) * X + ( OMC * XY - ZS ) * Y + ( OMC * ZX + YS ) * Z,
                ( OMC * XY + ZS ) * X + ( OMC * YY + C ) * Y + ( OMC * YZ - XS ) * Z,
                ( OMC * ZX - YS ) * X + ( OMC * YZ + XS ) * Y + ( OMC * ZZ + C ) * Z
            };
        }

        /**
         * Find good arbitrary axis vectors to represent U and V axes of a plane,
         * given just the normal.
         */
        void FindBestAxisVectors( TVector &Axis1, TVector &Axis2 ) const noexcept
        {
            const TReal NX = Abs( X );
            const TReal NY = Abs( Y );
            const TReal NZ = Abs( Z );

            // Find best basis vectors.
            if( NZ > NX && NZ > NY )
            {
                Axis1 = TVector( 1, 0, 0 );
            }
            else
            {
                Axis1 = TVector( 0, 0, 1 );
            }

            Axis1 = ( Axis1 - *this * ( Axis1 | *this ) ).SafeNormal( );
            Axis2 = Axis1 ^ *this;
        }

        SKL_FORCEINLINE TVector SafeNormal( TReal Tolerance = ToMyRealType( SMALL_NUMBER ) ) const noexcept
        {
            const TReal SquareSum = X * X + Y * Y + Z * Z;

            // Not sure if it's safe to add tolerance in there. Might introduce too many errors
            if( SquareSum == UnitValue )
            {
                return *this;
            }
            else if( SquareSum < Tolerance )
            {
                return TVector( ZeroValue );
            }
            const TReal Scale = InverseSqrt( SquareSum );
            return TVector{ X * Scale, Y * Scale, Z * Scale };
        }

        SKL_FORCEINLINE TVector SafeNormal2D( TReal Tolerance = ToMyRealType( SMALL_NUMBER ) ) const noexcept
        {
            const TReal SquareSum = X * X + Y * Y;

            // Not sure if it's safe to add tolerance in there. Might introduce too many errors
            if( SquareSum == UnitValue )
            {
                if( Z == ZeroValue )
                {
                    return *this;
                }

                return TVector{ X, Y, ZeroValue };
            }

            if( SquareSum < Tolerance )
            {
                return TVector( ZeroValue );
            }

            const TReal Scale = InverseSqrt( SquareSum );
            return TVector{ X * Scale, Y * Scale, ZeroValue };
        }

        /**
         * \brief Performs a 2D dot product
         */
        SKL_FORCEINLINE TReal Dot2d( TVector B ) const noexcept
        {
            TVector A( *this );
            A.Z = ZeroValue;
            B.Z = ZeroValue;
            A.Normalize( );
            B.Normalize( );
            return A | B;
        }

        /**
         * \brief Projects this vector onto the input vector. Does not assume A is unnormalized.
         */
        SKL_FORCEINLINE TVector ProjectOnTo( const TVector &A ) const noexcept
        {
            return ( A * ( ( *this | A ) / ( A | A ) ) );
        }

        /**
         * \brief When this vector contains Euler angles (degrees), ensure that angles are between +/-180.
         */
        void UnwindEuler( ) noexcept
        {
            UnwindDegreeComponent( X );
            UnwindDegreeComponent( Y );
            UnwindDegreeComponent( Z );
        }

        /** Utility to check if there are any NaNs in this vector. */
        _NODISCARD bool ContainsNaN( ) const noexcept
        {
            return ( FIsNaN( X ) || !FIsFinite( X ) ||
                     FIsNaN( Y ) || !FIsFinite( Y ) ||
                     FIsNaN( Z ) || !FIsFinite( Z ) );
        }

        /**
         * Returns TRUE if the vector is a unit vector within the specified tolerance.
         */
        SKL_FORCEINLINE bool IsUnit( TReal LengthSquaredTolerance = ToMyRealType( KINDA_SMALL_NUMBER ) ) const noexcept
        {
            return Abs( ToMyRealType( 1.0 ) - SizeSquared( ) ) < LengthSquaredTolerance;
        }

        void LerpLocal( TVector &End, TReal Alpha ) noexcept
        {
            X = ( ( End.X - X ) * Alpha );
            Y = ( ( End.Y - Y ) * Alpha );
            Z = ( ( End.Z - Z ) * Alpha );
        }

        _NODISCARD SAngle FaceOther( const TVector &other ) const noexcept
        {
            return SAngle{
                static_cast< int16_t >( FAtan2( other.Y - Y, other.X - X ) * ToMyRealType( RadToUnit ) )
            };
        }

        void MinOnAllAxis( const TVector &other ) noexcept
        {
            X = fmin( X, other.X );
            Y = fmin( Y, other.Y );
            Z = fmin( Z, other.Z );
        }

        void MaxOnAllAxis( const TVector &other ) noexcept
        {
            X = fmax( X, other.X );
            Y = fmax( Y, other.Y );
            Z = fmax( Z, other.Z );
        }

        _NODISCARD TReal DistanceTo( const TVector &Other ) const noexcept
        {
            return Sqrt( Pow( Other.X - X, ToMyRealType( 2.0 ) ) + Pow( Other.Y - Y, ToMyRealType( 2.0 ) ) + Pow( Other.Z - Z, ToMyRealType( 2.0 ) ) );
        }

        _NODISCARD TReal Distance2DTo( const TVector &Other ) const noexcept
        {
            return Sqrt( Pow( Other.X - X, ToMyRealType( 2.0 ) ) + Pow( Other.Y - Y, ToMyRealType( 2.0 ) ) );
        }

        _NODISCARD SKL_FORCEINLINE TVector Absolute( ) const noexcept
        {
            return TVector{
                Abs( X ), Abs( Y ), Abs( Z )
            };
        }

        template< std::TSInteger TInt >
        _NODISCARD SKL_FORCEINLINE TIntPoint< TInt > ToIntPoint2D( ) const noexcept
        {
            return {
                static_cast< TInt >( X ), static_cast< TInt >( Y )
            };
        }

        _NODISCARD static TVector PointPlaneProject( const TVector &Point, const TVector &A, const TVector &B, const TVector &C ) noexcept;

        SKL_FORCEINLINE friend TVector operator*( TReal Scale, const TVector &V )
        {
            return V.operator*( Scale );
        }
    };
    using SVector  = TVector< skReal >;
    using SVectorf = TVector< float >;
    using SVectord = TVector< double >;

    template< typename TReal >
    requires( std::is_floating_point_v< TReal > ) struct alignas( 16 ) TVector4
    {
        template< typename TOtherReal >
        static consteval TReal ToMyRealType( TOtherReal InOtherReal ) noexcept
        {
            return static_cast< TReal >( InOtherReal );
        }

        // Zero vector.
        static TVector4 const ZeroVector;

        // Unit vector.
        static TVector4 const UnitVector;

        static constexpr TReal ZeroValue = ToMyRealType( 0.0 );
        static constexpr TReal UnitValue = ToMyRealType( 1.0 );

        TReal X, Y, Z, W;

        TVector4( const TVector< TReal > &InVector, TReal InW = UnitValue ) noexcept :
            X( InVector.X ), Y( InVector.Y ), Z( InVector.Z ), W( InW )
        {
        }

        explicit TVector4( TReal InX = ZeroValue, TReal InY = ZeroValue, TReal InZ = ZeroValue, TReal InW = UnitValue ) noexcept :
            X( InX ), Y( InY ), Z( InZ ), W( InW )
        {
        }

        ~TVector4( ) noexcept = default;

        SKL_FORCEINLINE TReal &operator[]( int32_t ComponentIndex ) noexcept
        {
            return ( &X )[ ComponentIndex ];
        }
        SKL_FORCEINLINE TReal operator[]( int32_t ComponentIndex ) const noexcept
        {
            return ( &X )[ ComponentIndex ];
        }
        SKL_FORCEINLINE void Set( TReal InX, TReal InY, TReal InZ, TReal InW ) noexcept
        {
            X = InX;
            Y = InY;
            Z = InZ;
            W = InW;
        }

        // Unary operators.
        SKL_FORCEINLINE TVector4 operator-( ) const noexcept
        {
            return TVector4{ -X, -Y, -Z, -W };
        }

        // Binary math operators.
        SKL_FORCEINLINE TVector4 operator+( const TVector4 &V ) const noexcept
        {
            return TVector4{ X + V.X, Y + V.Y, Z + V.Z, W + V.W };
        }
        SKL_FORCEINLINE TVector4 operator-( const TVector4 &V ) const noexcept
        {
            return TVector4{ X - V.X, Y - V.Y, Z - V.Z, W - V.W };
        }
        SKL_FORCEINLINE TVector4 operator*( TReal Scale ) const noexcept
        {
            return TVector4{ X * Scale, Y * Scale, Z * Scale, W * Scale };
        }
        TVector4 operator/( TReal Scale ) const noexcept
        {
            const TReal RScale = UnitValue / Scale;
            return TVector4{ X * RScale, Y * RScale, Z * RScale, W * RScale };
        }
        SKL_FORCEINLINE TVector4 operator*( const TVector4 &V ) const noexcept
        {
            return TVector4{ X * V.X, Y * V.Y, Z * V.Z, W * V.W };
        }

        // Simple functions.
        _NODISCARD TReal &Component( int32_t Index ) noexcept
        {
            return ( &X )[ Index ];
        }

        _NODISCARD friend SKL_FORCEINLINE TReal Dot3( const TVector4 &V1, const TVector4 &V2 ) noexcept
        {
            return V1.X * V2.X + V1.Y * V2.Y + V1.Z * V2.Z;
        }
        _NODISCARD friend SKL_FORCEINLINE TReal Dot4( const TVector4 &V1, const TVector4 &V2 ) noexcept
        {
            return V1.X * V2.X + V1.Y * V2.Y + V1.Z * V2.Z + V1.W * V2.W;
        }
        _NODISCARD friend SKL_FORCEINLINE TVector4 operator*( TReal Scale, const TVector4 &V ) noexcept
        {
            return V.operator*( Scale );
        }

        /**
         * Basic == or != operators for SQuat
         */
        _NODISCARD SKL_FORCEINLINE bool operator==( const TVector4 &V ) const noexcept
        {
            return ( ( X == V.X ) && ( Y == V.Y ) && ( Z == V.Z ) && ( W == V.W ) );
        }
        _NODISCARD SKL_FORCEINLINE bool operator!=( const TVector4 &V ) const noexcept
        {
            return ( ( X != V.X ) || ( Y != V.Y ) || ( Z != V.Z ) || ( W != V.W ) );
        }

        /**
         * Error tolerant comparison
         */
        _NODISCARD bool Equals( const TVector4 &V, TReal Tolerance = ToMyRealType( KINDA_SMALL_NUMBER ) ) const noexcept
        {
            return Abs( X - V.X ) < Tolerance && Abs( Y - V.Y ) < Tolerance && Abs( Z - V.Z ) < Tolerance && Abs( W - V.W ) < Tolerance;
        }

        /** Returns a normalized 3D SVector */
        _NODISCARD SKL_FORCEINLINE TVector4 SafeNormal( TReal Tolerance = ToMyRealType( SMALL_NUMBER ) ) const noexcept
        {
            const TReal SquareSum = X * X + Y * Y + Z * Z;
            if( SquareSum > Tolerance )
            {
                const TReal Scale = InverseSqrt( SquareSum );
                return TVector4( X * Scale, Y * Scale, Z * Scale, ZeroValue );
            }

            return TVector4( ZeroValue );
        }

        template< typename TOtherReal >
        SKL_FORCEINLINE TVector< TOtherReal > ToVector( ) const noexcept requires( std::is_floating_point_v< TOtherReal > )
        {
            return TVector< TOtherReal >{
                TVector< TOtherReal >::ToMyRealType( X ),
                TVector< TOtherReal >::ToMyRealType( Y ),
                TVector< TOtherReal >::ToMyRealType( Z )
            };
        }
    };
    using SVector4  = TVector4< skReal >;
    using SVector4f = TVector4< float >;
    using SVector4d = TVector4< double >;

    template< typename TReal >
    requires( std::is_floating_point_v< TReal > ) struct alignas( 16 ) TPlane: public TVector< TReal >
    {
        template< typename TOtherReal >
        static consteval TReal ToMyRealType( TOtherReal InOtherReal ) noexcept
        {
            return static_cast< TReal >( InOtherReal );
        }

        // Zero vector.
        static TPlane const ZeroPlane;

        // Unit vector.
        static TPlane const UnitPlane;

        static constexpr TReal ZeroValue = ToMyRealType( 0.0 );
        static constexpr TReal UnitValue = ToMyRealType( 1.0 );

        using TVec3 = TVector< TReal >;
        using TVec4 = TVector4< TReal >;

        TReal W;

        SKL_FORCEINLINE TPlane( ) noexcept :
            W{ ZeroValue }
        {}

        SKL_FORCEINLINE TPlane( const TPlane &P ) noexcept = default;

        SKL_FORCEINLINE TPlane( const TVec4 &V ) noexcept :
            TVector< TReal >( V.X, V.Y, V.Z ), W( V.W )
        {
        }
        SKL_FORCEINLINE TPlane( TReal InX, TReal InY, TReal InZ, TReal InW ) noexcept :
            TVec3( InX, InY, InZ ), W( InW )
        {
        }
        SKL_FORCEINLINE TPlane( TVector< TReal > InNormal, TReal InW ) noexcept :
            TVec3( InNormal ), W( InW )
        {
        }
        SKL_FORCEINLINE TPlane( TVector< TReal > InBase, const TVec3 &InNormal ) noexcept :
            TVec3( InNormal ), W( InBase | InNormal )
        {
        }
        TPlane( TVec3 A, TVec3 B, TVec3 C ) noexcept :
            TVec3( ( ( B - A ) ^ ( C - A ) ).SafeNormal( ) ), W( A | ( ( B - A ) ^ ( C - A ) ).SafeNormal( ) )
        {
        }

        SKL_FORCEINLINE TReal PlaneDot( const TVec3 &P ) const noexcept
        {
            return this->X * P.X + this->Y * P.Y + this->Z * P.Z - W;
        }
        _NODISCARD SKL_FORCEINLINE TPlane Flip( ) const noexcept
        {
            return TPlane{ -this->X, -this->Y, -this->Z, -W };
        }

        SKL_FORCEINLINE bool operator==( const TPlane &V ) const noexcept
        {
            return this->X == V.X && this->Y == V.Y && this->Z == V.Z && W == V.W;
        }

        SKL_FORCEINLINE bool operator!=( const TPlane &V ) const noexcept
        {
            return this->X != V.X || this->Y != V.Y || this->Z != V.Z || W != V.W;
        }

        // Error-tolerant comparison.
        _NODISCARD bool Equals( const TPlane &V, TReal Tolerance = ToMyRealType( KINDA_SMALL_NUMBER ) ) const noexcept
        {
            return Abs( this->X - V.X ) < Tolerance && Abs( this->Y - V.Y ) < Tolerance && Abs( this->Z - V.Z ) < Tolerance && Abs( W - V.W ) < Tolerance;
        }

        SKL_FORCEINLINE TReal operator|( const TPlane &V ) const noexcept
        {
            return this->X * V.X + this->Y * V.Y + this->Z * V.Z + W * V.W;
        }
        TPlane operator+( const TPlane &V ) const noexcept
        {
            return TPlane{ this->X + V.X, this->Y + V.Y, this->Z + V.Z, W + V.W };
        }
        TPlane operator-( const TPlane &V ) const noexcept
        {
            return TPlane{ this->X - V.X, this->Y - V.Y, this->Z - V.Z, W - V.W };
        }
        TPlane operator/( TReal Scale ) const noexcept
        {
            const TReal RScale = UnitValue / Scale;
            return TPlane{ this->X * RScale, this->Y * RScale, this->Z * RScale, W * RScale };
        }
        TPlane operator*( TReal Scale ) const noexcept
        {
            return TPlane{ this->X * Scale, this->Y * Scale, this->Z * Scale, W * Scale };
        }
        TPlane operator*( const TPlane &V ) const noexcept
        {
            return TPlane{ this->X * V.X, this->Y * V.Y, this->Z * V.Z, W * V.W };
        }
        TPlane operator+=( const TPlane &V ) noexcept
        {
            this->X += V.X;
            this->Y += V.Y;
            this->Z += V.Z;
            W += V.W;
            return *this;
        }
        TPlane operator-=( const TPlane &V ) noexcept
        {
            this->X -= V.X;
            this->Y -= V.Y;
            this->Z -= V.Z;
            W -= V.W;
            return *this;
        }
        TPlane operator*=( TReal Scale ) noexcept
        {
            this->X *= Scale;
            this->Y *= Scale;
            this->Z *= Scale;
            W *= Scale;
            return *this;
        }
        TPlane operator*=( const TPlane &V ) noexcept
        {
            this->X *= V.X;
            this->Y *= V.Y;
            this->Z *= V.Z;
            W *= V.W;
            return *this;
        }
        TPlane operator/=( TReal V ) noexcept
        {
            const TReal RV = UnitValue / V;
            this->X *= RV;
            this->Y *= RV;
            this->Z *= RV;
            W *= RV;
            return *this;
        }
    };
    using SPlane  = TPlane< skReal >;
    using SPlanef = TPlane< float >;
    using SPlaned = TPlane< double >;

    template< typename TReal >
    requires( std::is_floating_point_v< TReal > ) struct TSphere
    {
        template< typename TOtherReal >
        static consteval TReal ToMyRealType( TOtherReal InOtherReal ) noexcept
        {
            return static_cast< TReal >( InOtherReal );
        }

        // Zero vector.
        static TSphere const ZeroSphere;

        // Unit vector.
        static TSphere const UnitSphere;

        static constexpr TReal ZeroValue = ToMyRealType( 0.0 );
        static constexpr TReal UnitValue = ToMyRealType( 1.0 );

        using TVec3 = TVector< TReal >;

        TVec3 Center;
        TReal W;

        TSphere( ) noexcept :
            W{ ZeroValue }
        {}
        TSphere( int32_t ) noexcept :
            Center( ZeroValue, ZeroValue, ZeroValue ), W( ZeroValue )
        {}
        TSphere( TVec3 InV, TReal InW ) noexcept :
            Center( InV ), W( InW )
        {}
        TSphere( const TVec3 *Pts, int32_t Count ) noexcept;

        _NODISCARD bool Equals( const TSphere &Sphere, TReal Tolerance = ToMyRealType( KINDA_SMALL_NUMBER ) ) const noexcept
        {
            return Center.Equals( Sphere.Center, Tolerance ) && Abs( W - Sphere.W ) < Tolerance;
        }

        _NODISCARD bool IsInside( const TSphere &Other, TReal Tolerance = ToMyRealType( KINDA_SMALL_NUMBER ) ) const noexcept
        {
            if( W > Other.W - Tolerance )
            {
                return false;
            }

            return ( Center - Other.Center ).SizeSquared( ) <= Square( Other.W - Tolerance - W );
        }
    };
    using SSphere  = TSphere< skReal >;
    using SSpheref = TSphere< float >;
    using SSphered = TSphere< double >;

    template< typename TReal >
    requires( std::is_floating_point_v< TReal > ) struct TBox
    {
        template< typename TOtherReal >
        static consteval TReal ToMyRealType( TOtherReal InOtherReal ) noexcept
        {
            return static_cast< TReal >( InOtherReal );
        }

        using TVec3 = TVector< TReal >;
        using TBool = std::conditional_t< sizeof( TReal ) == sizeof( double ), uint64_t, uint32_t >;

        // Zero vector.
        static TBox const ZeroBox;

        // Unit vector.
        static TBox const UnitBox;

        static constexpr TReal ZeroValue = ToMyRealType( 0.0 );
        static constexpr TReal UnitValue = ToMyRealType( 1.0 );

        TVec3 Min;
        TVec3 Max;
        TBool IsValid;

        TBox( ) noexcept :
            IsValid( false )
        {}
        TBox( int32_t ) noexcept :
            IsValid( false )
        {
            Init( );
        }
        TBox( const TVec3 &InMin, const TVec3 &InMax ) noexcept :
            Min( InMin ), Max( InMax ), IsValid( true )
        {
        }
        TBox( const TVec3 *Points, int32_t Count ) noexcept;

        _NODISCARD static TBox BuildAABB( const TVec3 &Origin, const TVec3 &Extent ) noexcept
        {
            return { Origin - Extent, Origin + Extent };
        }

        // Accessors.
        _NODISCARD SKL_FORCEINLINE TVec3 &GetExtrema( int32_t i ) noexcept
        {
            return ( &Min )[ i ];
        }
        _NODISCARD SKL_FORCEINLINE const TVec3 &GetExtrema( int32_t i ) const noexcept
        {
            return ( &Min )[ i ];
        }

        _NODISCARD SKL_FORCEINLINE TVec3 GetUpperLeftCornerBottom( ) const noexcept
        {
            return Min;
        }

        _NODISCARD SKL_FORCEINLINE TVec3 GetUpperRightCornerBottom( ) const noexcept
        {
            return {
                Max.X, Min.Y, Min.Z
            };
        }

        _NODISCARD SKL_FORCEINLINE TVec3 GetLowerRightCornerBottom( ) const noexcept
        {
            return {
                Max.X, Max.Y, Min.Z
            };
        }

        _NODISCARD SKL_FORCEINLINE TVec3 GetLowerLeftCornerBottom( ) const noexcept
        {
            return {
                Min.X, Max.Y, Min.Z
            };
        }

        _NODISCARD SKL_FORCEINLINE TVec3 GetUpperLeftCornerTop( ) const noexcept
        {
            return {
                Min.X, Min.Y, Max.Z
            };
        }

        _NODISCARD SKL_FORCEINLINE TVec3 GetUpperRightCornerTop( ) const noexcept
        {
            return {
                Max.X, Min.Y, Max.Z
            };
        }

        _NODISCARD SKL_FORCEINLINE TVec3 GetLowerLeftCornerTop( ) const noexcept
        {
            return {
                Min.X, Max.Y, Max.Z
            };
        }

        _NODISCARD SKL_FORCEINLINE TVec3 GetLowerRightCornerTop( ) const noexcept
        {
            return Max;
        }

        SKL_FORCEINLINE void Init( ) noexcept
        {
            Min = Max = TVec3( );
            IsValid   = false;
        }
        SKL_FORCEINLINE TBox &operator+=( const TVec3 &Other ) noexcept
        {
            if( IsValid )
            {
                Min.X = Min( Min.X, Other.X );
                Min.Y = Min( Min.Y, Other.Y );
                Min.Z = Min( Min.Z, Other.Z );
                Max.X = Max( Max.X, Other.X );
                Max.Y = Max( Max.Y, Other.Y );
                Max.Z = Max( Max.Z, Other.Z );
            }
            else
            {
                Min     = Other;
                Max     = Other;
                IsValid = true;
            }

            return *this;
        }
        SKL_FORCEINLINE TBox operator+( const TVec3 &Other ) const noexcept
        {
            return TBox( *this ) += Other;
        }
        SKL_FORCEINLINE TBox &operator+=( const TBox &Other ) noexcept
        {
            if( IsValid && Other.IsValid )
            {
                Min.X = Min( Min.X, Other.Min.X );
                Min.Y = Min( Min.Y, Other.Min.Y );
                Min.Z = Min( Min.Z, Other.Min.Z );
                Max.X = Max( Max.X, Other.Max.X );
                Max.Y = Max( Max.Y, Other.Max.Y );
                Max.Z = Max( Max.Z, Other.Max.Z );
            }
            else if( Other.IsValid )
            {
                ( *this ) = Other;
            }

            return *this;
        }
        SKL_FORCEINLINE TBox operator+( const TBox &Other ) const noexcept
        {
            return TBox( *this ) += Other;
        }
        SKL_FORCEINLINE TVec3 &operator[]( int32_t i ) noexcept
        {
            if( i == 0 )
            {
                return Min;
            }

            return Max;
        }

        _NODISCARD SKL_FORCEINLINE TBox ExpandBy( TReal W ) const noexcept
        {
            return { Min - TVec3( W, W, W ), Max + TVec3( W, W, W ) };
        }

        _NODISCARD SKL_FORCEINLINE TVec3 GetCenter( ) const noexcept
        {
            // midpoint between the min and max points.
            return { ( Min + Max ) * ToMyRealType( 0.5 ) };
        }

        _NODISCARD SKL_FORCEINLINE TVec3 GetExtent( ) const noexcept
        {
            // extent around the center
            return { ( Max - Min ) * ToMyRealType( 0.5 ) };
        }

        _NODISCARD SKL_FORCEINLINE SIntRect GetXYRectangle( ) const noexcept
        {
            return {
                { static_cast< int32_t >( Min.X ), static_cast< int32_t >( Min.Y ) },
                { static_cast< int32_t >( Max.X ), static_cast< int32_t >( Max.Y ) }
            };
        }

        _NODISCARD SKL_FORCEINLINE SInt64Rect GetXYRectangle64( ) const noexcept
        {
            return {
                { static_cast< int64_t >( Min.X ), static_cast< int64_t >( Min.Y ) },
                { static_cast< int64_t >( Max.X ), static_cast< int64_t >( Max.Y ) }
            };
        }

        template< std::TSInteger TInt >
        _NODISCARD SKL_FORCEINLINE TIntRect< TInt > GetXYRectangle( ) const noexcept
        {
            return {
                { static_cast< TInt >( Min.X ), static_cast< TInt >( Min.Y ) },
                { static_cast< TInt >( Max.X ), static_cast< TInt >( Max.Y ) }
            };
        }

        void GetCenterAndExtents( TVec3 &center, TVec3 &Extents ) const noexcept
        {
            Extents = GetExtent( );
            center  = Min + Extents;
        }

        _NODISCARD bool Intersect( const TBox &other ) const noexcept
        {
            if( Min.X > other.Max.X || other.Min.X > Max.X )
                return FALSE;
            if( Min.Y > other.Max.Y || other.Min.Y > Max.Y )
                return FALSE;
            if( Min.Z > other.Max.Z || other.Min.Z > Max.Z )
                return FALSE;
            return TRUE;
        }

        _NODISCARD bool IntersectXY( const TBox &other ) const noexcept
        {
            if( Min.X > other.Max.X || other.Min.X > Max.X )
                return FALSE;
            if( Min.Y > other.Max.Y || other.Min.Y > Max.Y )
                return FALSE;
            return TRUE;
        }

        _NODISCARD bool IsInside( const TVec3 &In ) const noexcept
        {
            return ( In.X > Min.X && In.X < Max.X && In.Y > Min.Y && In.Y < Max.Y && In.Z > Min.Z && In.Z < Max.Z );
        }

        _NODISCARD TReal GetVolume( ) const noexcept
        {
            return ( ( Max.X - Min.X ) * ( Max.Y - Min.Y ) * ( Max.Z - Min.Z ) );
        }

        /** Util to calculate distance from a point to a bounding box */
        _NODISCARD TReal ComputeSquaredDistanceToPoint( const TVec3 &Point ) const noexcept
        {
            // Accumulates the distance as we iterate axis
            TReal DistSquared = ZeroValue;

            // Check each axis for min/max and add the distance accordingly
            // NOTE: Loop manually unrolled for > 2x speed up
            if( Point.X < Min.X )
            {
                DistSquared += Square( Point.X - Min.X );
            }
            else if( Point.X > Max.X )
            {
                DistSquared += Square( Point.X - Max.X );
            }

            if( Point.Y < Min.Y )
            {
                DistSquared += Square( Point.Y - Min.Y );
            }
            else if( Point.Y > Max.Y )
            {
                DistSquared += Square( Point.Y - Max.Y );
            }

            if( Point.Z < Min.Z )
            {
                DistSquared += Square( Point.Z - Min.Z );
            }
            else if( Point.Z > Max.Z )
            {
                DistSquared += Square( Point.Z - Max.Z );
            }

            return DistSquared;
        }

        /** Return closest point on or inside the box to the given point in space. */
        _NODISCARD TVec3 GetClosestPointTo( const TVec3 &Point ) const noexcept
        {
            // Start by considering the Point inside the Box.
            TVec3 ClosestPoint = Point;

            // Now clamp to inside box if it's outside.
            if( Point.X < Min.X )
            {
                ClosestPoint.X = Min.X;
            }
            else if( Point.X > Max.X )
            {
                ClosestPoint.X = Max.X;
            }

            // Now clamp to inside box if it's outside.
            if( Point.Y < Min.Y )
            {
                ClosestPoint.Y = Min.Y;
            }
            else if( Point.Y > Max.Y )
            {
                ClosestPoint.Y = Max.Y;
            }

            // Now clamp to inside box if it's outside.
            if( Point.Z < Min.Z )
            {
                ClosestPoint.Z = Min.Z;
            }
            else if( Point.Z > Max.Z )
            {
                ClosestPoint.Z = Max.Z;
            }

            return ClosestPoint;
        }
    };
    using SBox  = TBox< skReal >;
    using SBoxf = TBox< float >;
    using SBoxd = TBox< double >;

    extern SVectorf ClosestPointOnTriangleToPoint( const SVectorf &Point, const SVectorf &A, const SVectorf &B, const SVectorf &C ) noexcept;
    extern SVectord ClosestPointOnTriangleToPoint( const SVectord &Point, const SVectord &A, const SVectord &B, const SVectord &C ) noexcept;

    template< typename TReal >
    requires( std::is_floating_point_v< TReal > ) struct TTriangle
    {
        template< typename TOtherReal >
        static consteval TReal ToMyRealType( TOtherReal InOtherReal ) noexcept
        {
            return static_cast< TReal >( InOtherReal );
        }

        using TVec3    = TVector< TReal >;
        using TMyPlane = TPlane< TReal >;
        using TMyBox   = TBox< TReal >;
        using TBool    = std::conditional_t< sizeof( TReal ) == sizeof( double ), uint64_t, uint32_t >;

        static constexpr TReal ZeroValue = ToMyRealType( 0.0 );
        static constexpr TReal UnitValue = ToMyRealType( 1.0 );

        TVec3 Triangle[ 3 ];

        TTriangle( ) noexcept = default;
        TTriangle( TVec3 A, TVec3 B, TVec3 C ) noexcept :
            Triangle{ A, B, C }
        {}

        TMyBox GetBoundingBox( ) const noexcept
        {
            const TVec3 Min = {
                Min( Triangle[ 0 ].X, Triangle[ 1 ].X, Triangle[ 2 ].X ), Min( Triangle[ 0 ].Y, Triangle[ 1 ].Y, Triangle[ 2 ].Y ), Min( Triangle[ 0 ].Z, Triangle[ 1 ].Z, Triangle[ 2 ].Z )
            };

            const TVec3 Max = {
                Max( Triangle[ 0 ].X, Triangle[ 1 ].X, Triangle[ 2 ].X ), Max( Triangle[ 0 ].Y, Triangle[ 1 ].Y, Triangle[ 2 ].Y ), Max( Triangle[ 0 ].Z, Triangle[ 1 ].Z, Triangle[ 2 ].Z )
            };

            return { Min, Max };
        }

        TVec3 GetRandomPointOnTriangle( ) const noexcept
        {
            const auto Bounds = GetBoundingBox( );
            TVec3      Result = Bounds.Min;

            TReal RandOne;
            TReal RandTwo;

            if constexpr( std::is_same_v< TReal, float > )
            {
                RandOne = RandomTypeToUse::NextRandomF( );
                RandTwo = RandomTypeToUse::NextRandomF( );
            }
            else
            {
                RandOne = RandomTypeToUse::NextRandomD( );
                RandTwo = RandomTypeToUse::NextRandomD( );
            }

            Result = BiLerp( Bounds.Min,
                                    Bounds.Max,
                                    Bounds.GetUpperRightCornerTop( ),
                                    Bounds.GetLowerLeftCornerTop( ),
                                    RandOne,
                                    RandTwo );

            Result = ClosestPointOnTriangleToPoint( Result, Triangle[ 0 ], Triangle[ 1 ], Triangle[ 2 ] );

            return Result;
        }

        bool IsInside( TVec3 Point ) const noexcept
        {
            // Figure out what region the point is in and compare against that "point" or "edge"
            const TVec3 BA        = Triangle[ 0 ] - Triangle[ 1 ];
            const TVec3 AC        = Triangle[ 2 ] - Triangle[ 0 ];
            const TVec3 CB        = Triangle[ 1 ] - Triangle[ 2 ];
            const TVec3 TriNormal = BA ^ CB;

            // Get the planes that define this triangle
            // edges BA, AC, BC with normals perpendicular to the edges facing outward
            const TMyPlane Planes[ 3 ]           = { TMyPlane( Triangle[ 1 ], TriNormal ^ BA ), TMyPlane( Triangle[ 0 ], TriNormal ^ AC ), TMyPlane( Triangle[ 2 ], TriNormal ^ CB ) };
            int32_t        PlaneHalfspaceBitmask = 0;

            // Determine which side of each plane the test point exists
            for( int32_t i = 0; i < 3; i++ )
            {
                if( Planes[ i ].PlaneDot( Point ) > ZeroValue )
                {
                    PlaneHalfspaceBitmask |= ( 1 << i );
                }
            }

            return PlaneHalfspaceBitmask == 0;
        }
    };
    using STriangle  = TTriangle< skReal >;
    using STrianglef = TTriangle< float >;
    using STriangled = TTriangle< double >;

    template< typename TReal >
    requires( std::is_floating_point_v< TReal > )
        _NODISCARD TVector< TReal > TVector< TReal >::PointPlaneProject( const TVector< TReal > &Point, const TVector< TReal > &A, const TVector< TReal > &B, const TVector< TReal > &C )
    noexcept
    {
        // Compute the plane normal from ABC
        const TPlane< TReal > Plane( A, B, C );

        // Find the distance of X from the plane
        // Add the distance back along the normal from the point
        return Point - ( Plane.PlaneDot( Point ) * Plane );
    }

    template< typename TReal >
    requires( std::is_floating_point_v< TReal > )
        TBox< TReal >::TBox( const TVector< TReal > *Points, int32_t Count )
    noexcept :
        Min( 0, 0, 0 ), Max( 0, 0, 0 ), IsValid( false )
    {
        for( int32_t i = 0; i < Count; i++ )
        {
            *this += Points[ i ];
        }
    }

    template< typename TReal >
    requires( std::is_floating_point_v< TReal > )
        TSphere< TReal >::TSphere( const TVector< TReal > *Pts, int32_t Count ) noexcept :
        Center( 0, 0, 0 ), W( 0 )
    {
        if( Count )
        {
            const TBox< TReal > Box( Pts, Count );
            *this = TSphere< TReal >( ( Box.Min + Box.Max ) / ToMyRealType( 2.0 ), ZeroValue );
            for( int32_t i = 0; i < Count; i++ )
            {
                const TReal Dist = SDistSquared( Pts[ i ], Center );
                if( Dist > W )
                {
                    W = Dist;
                }
            }
            W = Sqrt( W ) * ToMyRealType( 1.001 );
        }
    }
}

#include "SMath.h"
