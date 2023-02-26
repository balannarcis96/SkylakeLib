#if defined(SKL_MATH)

#include "../SkylakeLib.h"

namespace SKL
{
    SVector VRand( ) noexcept
    {
        SVector Result;

        do
        {
            // Check random vectors in the unit sphere so result is statistically uniform.
            Result.X = ( static_cast< skReal >( RandomTypeToUse::NextRandomF( ) ) * SKL_REAL_VALUE( 2.0 ) ) - SK_REAL_ONE;
            Result.Y = ( static_cast< skReal >( RandomTypeToUse::NextRandomF( ) ) * SKL_REAL_VALUE( 2.0 ) ) - SK_REAL_ONE;
            Result.Z = ( static_cast< skReal >( RandomTypeToUse::NextRandomF( ) ) * SKL_REAL_VALUE( 2.0 ) ) - SK_REAL_ONE;
        } while( Result.SizeSquared( ) > SK_REAL_ONE );

        return Result.UnsafeNormal( );
    }

    SVector ClosestPointOnLine( const SVector &LineStart, const SVector &LineEnd, const SVector &Point ) noexcept
    {
        // Solve to find alpha along line that is closest point
        // Weisstein, Eric W. "Point-Line Distance--3-Dimensional." From MathWorld--A Wolfram Web Resource. http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html
        const skReal A = ( LineStart - Point ) | ( LineEnd - LineStart );
        const skReal B = ( LineEnd - LineStart ).SizeSquared( );
        const skReal T = Clamp( -A / B, SK_REAL_ZERO, SK_REAL_ONE );

        // Generate closest point
        return LineStart + ( ( LineEnd - LineStart ) * T );
    }

    SVector VInterpNormalConstantTo( const SVector Current, const SVector &Target, skReal DeltaTime, skReal InterpSpeed ) noexcept
    {
        const SVector Delta   = Target - Current;
        const skReal  DeltaM  = Delta.Size( );
        const skReal  MaxStep = InterpSpeed * DeltaTime;

        if( DeltaM > MaxStep )
        {
            if( MaxStep > SK_REAL_ZERO )
            {
                const SVector DeltaN = Delta / DeltaM;
                return ( Current + DeltaN * MaxStep ).SafeNormal( );
            }
            else
            {
                return Current;
            }
        }

        return Target;
    }

    SVector VInterpConstantTo( const SVector Current, const SVector &Target, skReal DeltaTime, skReal InterpSpeed ) noexcept
    {
        const SVector Delta   = Target - Current;
        const skReal  DeltaM  = Delta.Size( );
        const skReal  MaxStep = InterpSpeed * DeltaTime;

        if( DeltaM > MaxStep )
        {
            if( MaxStep > SK_REAL_ZERO )
            {
                const SVector DeltaN = Delta / DeltaM;
                return Current + DeltaN * MaxStep;
            }
            else
            {
                return Current;
            }
        }

        return Target;
    }

    SVector VInterpTo( const SVector &Current, const SVector &Target, const skReal &DeltaTime, skReal InterpSpeed ) noexcept
    {
        // If no interp speed, jump to target value
        if( InterpSpeed <= SK_REAL_ZERO )
        {
            return Target;
        }

        // Distance to reach
        const SVector Dist = Target - Current;

        // If distance is too small, just set the desired location
        if( Dist.SizeSquared( ) < KINDA_SMALL_NUMBER )
        {
            return Target;
        }

        // Delta Move, Clamp so we do not over shoot.
        const SVector DeltaMove = Dist * Clamp< skReal >( DeltaTime * InterpSpeed, SK_REAL_ZERO, SK_REAL_ONE );

        return Current + DeltaMove;
    }

    SVector VClamp( SVector A, SVector Min, SVector Max ) noexcept
    {
        A.X = Clamp( A.X, Min.X, Max.X );
        A.Y = Clamp( A.Y, Min.Y, Max.Y );
        A.Z = Clamp( A.Z, Min.Z, Max.Z );
        return A;
    }

    bool SPlaneAABBIntersects( const SPlane &P, const SBox &AABB ) noexcept
    {
        // find diagonal most closely aligned with normal of plane
        SVector VMin, VMax;

        // Bypass the slow FVector[] operator. Not RESTRICT because it won't update Vmin, Vmax
        auto *VMinPtr = reinterpret_cast< skReal * >( &VMin );
        auto *VMaxPtr = reinterpret_cast< skReal * >( &VMax );

        // Use restrict to get better instruction scheduling and to bypass the slow FVector[] operator
        auto *__restrict AABBMinPtr = reinterpret_cast< const skReal * >( &AABB.Min );
        auto *__restrict AABBMaxPtr = reinterpret_cast< const skReal * >( &AABB.Max );
        auto *__restrict PlanePtr   = reinterpret_cast< const skReal   *>( &P );

        for( int32_t Idx = 0; Idx < 3; ++Idx )
        {
            if( PlanePtr[ Idx ] >= SK_REAL_ZERO )
            {
                VMinPtr[ Idx ] = AABBMinPtr[ Idx ];
                VMaxPtr[ Idx ] = AABBMaxPtr[ Idx ];
            }
            else
            {
                VMinPtr[ Idx ] = AABBMaxPtr[ Idx ];
                VMaxPtr[ Idx ] = AABBMinPtr[ Idx ];
            }
        }

        // if either diagonal is right on the plane, or one is on either side we have an intersection
        const skReal dMax = P.PlaneDot( VMax );
        const skReal dMin = P.PlaneDot( VMin );

        // if Max is below plane, or Min is above we know there is no intersection.. otherwise there must be one
        return ( dMax >= SK_REAL_ZERO && dMin <= SK_REAL_ZERO );
    }

    bool SLineExtentBoxIntersection( const SBox &inBox, const SVector &Start, const SVector &End, const SVector &Extent, SVector &HitLocation, SVector &HitNormal, skReal &HitTime ) noexcept
    {
        SBox box = inBox;
        box.Max.X += Extent.X;
        box.Max.Y += Extent.Y;
        box.Max.Z += Extent.Z;

        box.Min.X -= Extent.X;
        box.Min.Y -= Extent.Y;
        box.Min.Z -= Extent.Z;

        const SVector Dir = ( End - Start );

        SVector Time;
        BOOL    Inside       = 1;
        skReal  faceDir[ 3 ] = { 1, 1, 1 };

        /////////////// X
        if( Start.X < box.Min.X )
        {
            if( Dir.X <= SK_REAL_ZERO )
                return false;
            else
            {
                Inside       = 0;
                faceDir[ 0 ] = -1;
                Time.X       = ( box.Min.X - Start.X ) / Dir.X;
            }
        }
        else if( Start.X > box.Max.X )
        {
            if( Dir.X >= SK_REAL_ZERO )
                return false;
            else
            {
                Inside = 0;
                Time.X = ( box.Max.X - Start.X ) / Dir.X;
            }
        }
        else
            Time.X = SK_REAL_ZERO;

        /////////////// Y
        if( Start.Y < box.Min.Y )
        {
            if( Dir.Y <= SK_REAL_ZERO )
                return false;
            else
            {
                Inside       = 0;
                faceDir[ 1 ] = -1;
                Time.Y       = ( box.Min.Y - Start.Y ) / Dir.Y;
            }
        }
        else if( Start.Y > box.Max.Y )
        {
            if( Dir.Y >= SK_REAL_ZERO )
                return false;
            else
            {
                Inside = 0;
                Time.Y = ( box.Max.Y - Start.Y ) / Dir.Y;
            }
        }
        else
            Time.Y = SK_REAL_ZERO;

        /////////////// Z
        if( Start.Z < box.Min.Z )
        {
            if( Dir.Z <= SK_REAL_ZERO )
                return false;
            else
            {
                Inside       = 0;
                faceDir[ 2 ] = -1;
                Time.Z       = ( box.Min.Z - Start.Z ) / Dir.Z;
            }
        }
        else if( Start.Z > box.Max.Z )
        {
            if( Dir.Z >= SK_REAL_ZERO )
                return false;
            else
            {
                Inside = 0;
                Time.Z = ( box.Max.Z - Start.Z ) / Dir.Z;
            }
        }
        else
            Time.Z = SK_REAL_ZERO;

        // If the line started inside the box (ie. player started in contact with the fluid)
        if( Inside )
        {
            HitLocation = Start;
            HitNormal   = SVector( 0, 0, 1 );
            HitTime     = 0;
            return true;
        }
        // Otherwise, calculate when hit occurred
        else
        {
            if( Time.Y > Time.Z )
            {
                HitTime   = Time.Y;
                HitNormal = SVector( 0, faceDir[ 1 ], 0 );
            }
            else
            {
                HitTime   = Time.Z;
                HitNormal = SVector( 0, 0, faceDir[ 2 ] );
            }

            if( Time.X > HitTime )
            {
                HitTime   = Time.X;
                HitNormal = SVector( faceDir[ 0 ], 0, 0 );
            }

            if( HitTime >= SK_REAL_ZERO && HitTime <= SK_REAL_ONE )
            {
                HitLocation                         = Start + Dir * HitTime;
                constexpr skReal BOX_SIDE_THRESHOLD = SKL_REAL_VALUE( 0.1 );
                if( HitLocation.X > box.Min.X - BOX_SIDE_THRESHOLD && HitLocation.X < box.Max.X + BOX_SIDE_THRESHOLD &&
                    HitLocation.Y > box.Min.Y - BOX_SIDE_THRESHOLD && HitLocation.Y < box.Max.Y + BOX_SIDE_THRESHOLD &&
                    HitLocation.Z > box.Min.Z - BOX_SIDE_THRESHOLD && HitLocation.Z < box.Max.Z + BOX_SIDE_THRESHOLD )
                {
                    return true;
                }
            }

            return false;
        }
    }

    SVectord ClosestPointOnSegment( const SVectord &Point, const SVectord &StartPoint, const SVectord &EndPoint ) noexcept
    {
        const SVectord Segment     = EndPoint - StartPoint;
        const SVectord VectToPoint = Point - StartPoint;

        // See if closest point is before StartPoint
        const double Dot1 = VectToPoint | Segment;
        if( Dot1 <= 0.0 )
        {
            return StartPoint;
        }

        // See if closest point is beyond EndPoint
        const double Dot2 = Segment | Segment;
        if( Dot2 <= Dot1 )
        {
            return EndPoint;
        }

        // Closest Point is within segment
        return StartPoint + Segment * ( Dot1 / Dot2 );
    }

    SVectorf ClosestPointOnSegment( const SVectorf &Point, const SVectorf &StartPoint, const SVectorf &EndPoint ) noexcept
    {
        const SVectorf Segment     = EndPoint - StartPoint;
        const SVectorf VectToPoint = Point - StartPoint;

        // See if closest point is before StartPoint
        const float Dot1 = VectToPoint | Segment;
        if( Dot1 <= 0.0f )
        {
            return StartPoint;
        }

        // See if closest point is beyond EndPoint
        const float Dot2 = Segment | Segment;
        if( Dot2 <= Dot1 )
        {
            return EndPoint;
        }

        // Closest Point is within segment
        return StartPoint + Segment * ( Dot1 / Dot2 );
    }

    SVectord ClosestPointOnTriangleToPoint( const SVectord &Point, const SVectord &A, const SVectord &B, const SVectord &C ) noexcept
    {
        // Figure out what region the point is in and compare against that "point" or "edge"
        const SVectord BA        = A - B;
        const SVectord AC        = C - A;
        const SVectord CB        = B - C;
        const SVectord TriNormal = BA ^ CB;

        // Get the planes that define this triangle
        // edges BA, AC, BC with normals perpendicular to the edges facing outward
        const SPlaned Planes[ 3 ]           = { SPlaned( B, TriNormal ^ BA ), SPlaned( A, TriNormal ^ AC ), SPlaned( C, TriNormal ^ CB ) };
        int32_t       PlaneHalfspaceBitmask = 0;

        // Determine which side of each plane the test point exists
        for( int32_t i = 0; i < 3; i++ )
        {
            if( Planes[ i ].PlaneDot( Point ) > 0.0 )
            {
                PlaneHalfspaceBitmask |= ( 1 << i );
            }
        }

        SVectord Result( Point.X, Point.Y, Point.Z );
        switch( PlaneHalfspaceBitmask )
        {
            case 0: // 000 Inside
                return SVectord::PointPlaneProject( Point, A, B, C );
            case 1: // 001 Segment BA
                Result = ClosestPointOnSegment( Point, B, A );
                break;
            case 2: // 010 Segment AC
                Result = ClosestPointOnSegment( Point, A, C );
                break;
            case 3: // 011 point A
                return A;
            case 4: // 100 Segment BC
                Result = ClosestPointOnSegment( Point, B, C );
                break;
            case 5: // 101 point B
                return B;
            case 6: // 110 point C
                return C;
            default:
                printf_s( "[SMath]::Impossible result in SMath::ClosestPointOnTriangleToPoint(double)\n" );
                break;
        }

        return Result;
    }

    SVectorf ClosestPointOnTriangleToPoint( const SVectorf &Point, const SVectorf &A, const SVectorf &B, const SVectorf &C ) noexcept
    {
        // Figure out what region the point is in and compare against that "point" or "edge"
        const SVectorf BA        = A - B;
        const SVectorf AC        = C - A;
        const SVectorf CB        = B - C;
        const SVectorf TriNormal = BA ^ CB;

        // Get the planes that define this triangle
        // edges BA, AC, BC with normals perpendicular to the edges facing outward
        const SPlanef Planes[ 3 ]           = { SPlanef( B, TriNormal ^ BA ), SPlanef( A, TriNormal ^ AC ), SPlanef( C, TriNormal ^ CB ) };
        int32_t       PlaneHalfspaceBitmask = 0;

        // Determine which side of each plane the test point exists
        for( int32_t i = 0; i < 3; i++ )
        {
            if( Planes[ i ].PlaneDot( Point ) > 0.0f )
            {
                PlaneHalfspaceBitmask |= ( 1 << i );
            }
        }

        SVectorf Result( Point.X, Point.Y, Point.Z );
        switch( PlaneHalfspaceBitmask )
        {
            case 0: // 000 Inside
                return SVectorf::PointPlaneProject( Point, A, B, C );
            case 1: // 001 Segment BA
                Result = ClosestPointOnSegment( Point, B, A );
                break;
            case 2: // 010 Segment AC
                Result = ClosestPointOnSegment( Point, A, C );
                break;
            case 3: // 011 point A
                return A;
            case 4: // 100 Segment BC
                Result = ClosestPointOnSegment( Point, B, C );
                break;
            case 5: // 101 point B
                return B;
            case 6: // 110 point C
                return C;
            default:
                printf_s( "[SMath]::Impossible result in SMath::ClosestPointOnTriangleToPoint(double)\n" );
                break;
        }

        return Result;
    }
} // namespace SMath

#endif
