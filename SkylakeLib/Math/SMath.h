//!
//! \file SMath.h
//! 
//! \brief All math abstractions and constants for SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    /**
     * \brief Add to a word angle, constraining it within a min (not to cross) and a max (not to cross).  Accounts for funkyness of word angles.
     * \remarks Assumes that angle is initially in the desired range.
     */
    SKL_FORCEINLINE SKL_NODISCARD inline int16_t SAddAngleConfined( int32_t Angle, int32_t Delta, int32_t MinThresh, int32_t MaxThresh ) noexcept
    {
        if( Delta < 0 )
        {
            if( Delta <= -0x10000L || Delta <= -static_cast< int32_t >( static_cast< int16_t >( Angle - MinThresh ) ) )
                return static_cast< int16_t >( MinThresh );
        }
        else if( Delta > 0 )
        {
            if( Delta >= 0x10000L || Delta >= static_cast< int32_t >( static_cast< int16_t >( MaxThresh - Angle ) ) )
                return static_cast< int16_t >( MaxThresh );
        }

        return static_cast< int16_t >( Angle + Delta );
    }

    /**
     * \brief Find the point on line segment from LineStart to LineEnd which is closest to Point
     */
    SKL_NODISCARD SVector ClosestPointOnLine( const SVector &LineStart, const SVector &LineEnd, const SVector &Point ) noexcept;

    /**
     * \brief Converts given Cartesian coordinate pair to Polar coordinate system
     */
    template< typename TFloat >
    SKL_FORCEINLINE inline void CartesianToPolar( TFloat X, TFloat Y, TFloat &OutRad, TFloat &OutAng ) noexcept requires( std::is_same_v< TFloat, float > || std::is_same_v< TFloat, double > )
    {
        OutRad = Sqrt( Square( X ) + Square( Y ) );
        OutAng = FAtan2( Y, X );
    }

    /**
     * \brief Converts given Polar coordinate pair to Cartesian coordinate system
     */
    template< typename TFloat >
    SKL_FORCEINLINE inline void PolarToCartesian( TFloat Rad, TFloat Ang, TFloat &OutX, TFloat &OutY ) noexcept requires( std::is_same_v< TFloat, float > || std::is_same_v< TFloat, double > )
    {
        OutX = Rad * FCos( Ang );
        OutY = Rad * FSin( Ang );
    }

    /**
     * \brief Interpolate a normal vector from Current to Target with constant step
     */
    SKL_NODISCARD SVector VInterpNormalConstantTo( const SVector Current, const SVector &Target, skReal DeltaTime, skReal InterpSpeed ) noexcept;

    /**
     * \brief Interpolate vector from Current to Target with constant step
     */
    SKL_NODISCARD SVector VInterpConstantTo( const SVector Current, const SVector &Target, skReal DeltaTime, skReal InterpSpeed ) noexcept;

    /**
     * \brief Interpolate vector from Current to Target. Scaled by distance to Target, so it has a strong start speed and ease out
     */
    SKL_NODISCARD SVector VInterpTo( const SVector &Current, const SVector &Target, const skReal &DeltaTime, skReal InterpSpeed ) noexcept;

    template<typename T = float>
    SKL_NODISCARD inline T SInterpTo( T Current, T Target, T DeltaTime, T InterpSpeed ) noexcept requires( std::is_same_v< T, float > || std::is_same_v< T, double > )
    {
        // If no interp speed, jump to target value
        if( InterpSpeed == static_cast< T >( 0.0 ) )
        {
            return Target;
        }

        // Distance to reach
        const T Dist = Target - Current;

        // If distance is too small, just set the desired location
        if( Square( Dist ) < SMALL_NUMBER )
        {
            return Target;
        }

        // Delta Move, Clamp so we do not over shoot.
        const T DeltaMove = Dist * Clamp< T >( DeltaTime * InterpSpeed, static_cast< T >( 0.0 ), static_cast< T >( 1.0 ) );

        return Current + DeltaMove;
    }

    template<typename T = float>
    SKL_NODISCARD inline T SInterpConstantTo( T Current, T Target, T DeltaTime, T InterpSpeed ) noexcept requires( std::is_same_v< T, float > || std::is_same_v< T, double > )
    {
        const T Dist = Target - Current;

        // If distance is too small, just set the desired location
        if( Square( Dist ) < SMALL_NUMBER )
        {
            return Target;
        }

        const T Step = InterpSpeed * DeltaTime;
        return Current + Clamp< T >( Dist, -Step, Step );
    }

    template< typename T = float >
    SKL_NODISCARD inline T SInterpEaseInOut( T A, T B, T Alpha, T Exp ) noexcept requires( std::is_same_v< T, float > || std::is_same_v< T, double > )
    {
        T ModifiedAlpha;

        if( Alpha < static_cast< T >( 0.5 ) )
        {
            ModifiedAlpha = static_cast< T >( 0.5 ) * Pow( static_cast< T >( 2.0 ) * Alpha, Exp );
        }
        else
        {
            ModifiedAlpha = static_cast< T >( 1.0 ) - static_cast< T >( 0.5 ) * Pow( static_cast< T >( 2.0 ) * ( static_cast< T >( 1.0 ) - Alpha ), Exp );
        }

        return Lerp( A, B, ModifiedAlpha );
    }

    /**
     * \brief Clamp of Vector A From Min to Max of XYZ
     */
    SKL_NODISCARD SVector VClamp( SVector A, SVector Min, SVector Max ) noexcept;

    SKL_FORCEINLINE SKL_NODISCARD inline int32_t ReduceAngle( int32_t Angle ) noexcept
    {
        return Angle & SGlobalMath::ANGLE_MASK;
    }

    /**
     * \brief Convert a direction vector into a 'heading' angle between +/-PI. 0 is pointing down +X
     */
    SKL_FORCEINLINE SKL_NODISCARD inline skReal HeadingAngle( SVector Dir ) noexcept
    {
        // Project Dir into Z plane.
        SVector PlaneDir = Dir;
        PlaneDir.Z       = SK_REAL_ZERO;
        PlaneDir         = PlaneDir.SafeNormal( );

        skReal Angle = FAcos( PlaneDir.X );

        if( PlaneDir.Y < SK_REAL_ZERO )
        {
            Angle *= -SK_REAL_ONE;
        }

        return Angle;
    }

    /**
     * \brief Given a heading which may be outside the +/- PI range, 'unwind' it back into that range
     */
    template< typename T = float >
    SKL_FORCEINLINE SKL_NODISCARD inline T UnwindHeading( T A ) noexcept requires( std::is_same_v< T, float > || std::is_same_v< T, double > )
    {
        while( A > static_cast< T >( PI ) )
        {
            A -= ( static_cast< T >( PI ) * static_cast< T >( 2.0 ) );
        }

        while( A < -static_cast< T >( PI ) )
        {
            A += ( static_cast< T >( PI ) * static_cast< T >( 2.0 ) );
        }

        return A;
    }

    /**
     * \brief Compare two points and see if they're the same, using a threshold.
     * \returns Returns true=yes, false=no. Uses fast distance approximation.
     */
    SKL_FORCEINLINE SKL_NODISCARD inline bool SPointsAreSame( const SVector &P, const SVector &Q ) noexcept
    {
        skReal Temp = P.X - Q.X;
        if( ( Temp > -THRESH_POINTS_ARE_SAME ) && ( Temp < THRESH_POINTS_ARE_SAME ) )
        {
            Temp = P.Y - Q.Y;
            if( ( Temp > -THRESH_POINTS_ARE_SAME ) && ( Temp < THRESH_POINTS_ARE_SAME ) )
            {
                Temp = P.Z - Q.Z;
                if( ( Temp > -THRESH_POINTS_ARE_SAME ) && ( Temp < THRESH_POINTS_ARE_SAME ) )
                {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * \brief Compare two points and see if they're the same, using a threshold.
     * \returns Returns true=yes, false=no. Uses fast distance approximation.
     */
    SKL_FORCEINLINE SKL_NODISCARD inline bool SPointsAreNear( const SVector &Point1, const SVector &Point2, skReal Dist ) noexcept
    {
        skReal Temp = ( Point1.X - Point2.X );
        if( Abs( Temp ) >= Dist )
            return false;
        Temp = ( Point1.Y - Point2.Y );
        if( Abs( Temp ) >= Dist )
            return false;
        Temp = ( Point1.Z - Point2.Z );
        if( Abs( Temp ) >= Dist )
            return false;
        return true;
    }

    /**
     * \brief Calculate the signed distance (in the direction of the normal) between a point and a plane.
     */
    SKL_FORCEINLINE SKL_NODISCARD inline skReal SPointPlaneDist( const SVector &Point, const SVector &PlaneBase, const SVector &PlaneNormal ) noexcept
    {
        return ( Point - PlaneBase ) | PlaneNormal;
    }

    /**
     * \brief Calculate a the projection of a point on the plane defined by CCW points A,B,C
     * \param Point - the point to project onto the plane
     * \param A - three points in CCW order defining the plane
     * \param B - three points in CCW order defining the plane
     * \param C - three points in CCW order defining the plane
     * \returns Projection of Point onto plane ABC
     */
    SKL_FORCEINLINE SKL_NODISCARD inline SVector SPointPlaneProject( const SVector &Point, const SVector &A, const SVector &B, const SVector &C ) noexcept
    {
        // Compute the plane normal from ABC
        const SPlane Plane( A, B, C );

        // Find the distance of X from the plane
        // Add the distance back along the normal from the point
        return Point - Plane * Plane.PlaneDot( Point );
    }

    /**
     * \brief Calculate a the projection of a point on the plane defined by PlaneBase, and PlaneNormal
     * \param Point - the point to project onto the plane
     * \param PlaneBase - point on the plane
     * \param PlaneNorm - normal of the plane
     * \returns Projection of Point onto plane ABC
     */
    SKL_FORCEINLINE SKL_NODISCARD inline SVector SPointPlaneProject( const SVector &Point, const SVector &PlaneBase, const SVector &PlaneNorm ) noexcept
    {
        // Find the distance of X from the plane
        // Add the distance back along the normal from the point
        return Point - PlaneNorm * SPointPlaneDist( Point, PlaneBase, PlaneNorm );
    }

    /**
     * \brief Euclidean distance between two points.
     */
    SKL_FORCEINLINE SKL_NODISCARD inline skReal SDist( const SVector &V1, const SVector &V2 ) noexcept
    {
        return Sqrt( Square( V2.X - V1.X ) + Square( V2.Y - V1.Y ) + Square( V2.Z - V1.Z ) );
    }

    /**
     * \brief Squared distance between two points.
     */
    SKL_FORCEINLINE SKL_NODISCARD inline skReal SDistSquared( const SVector &V1, const SVector &V2 ) noexcept
    {
        return Square( V2.X - V1.X ) + Square( V2.Y - V1.Y ) + Square( V2.Z - V1.Z );
    }

    /**
     * \brief See if two normal vectors (or plane normals) are nearly parallel.
     */
    SKL_FORCEINLINE SKL_NODISCARD inline skReal SParallel( const SVector &Normal1, const SVector &Normal2 ) noexcept
    {
        const skReal NormalDot = Normal1 | Normal2;
        return ( Abs( NormalDot - SK_REAL_ONE ) <= THRESH_VECTORS_ARE_PARALLEL );
    }

    /**
     * \brief See if two planes are coplanar.
     */
    SKL_FORCEINLINE SKL_NODISCARD inline skReal SCoplanar( const SVector &Base1, const SVector &Normal1, const SVector &Base2, const SVector &Normal2 ) noexcept
    {
        if( SK_REAL_ZERO == SParallel( Normal1, Normal2 ) || SPointPlaneDist( Base2, Base1, Normal1 ) > THRESH_POINT_ON_PLANE )
        {
            return false;
        }

        return true;
    }

    /**
     * \brief Triple product of three vectors.
     */
    SKL_FORCEINLINE SKL_NODISCARD inline skReal STriple( const SVector &X, const SVector &Y, const SVector &Z ) noexcept
    {
        return ( ( X.X * ( Y.Y * Z.Z - Y.Z * Z.Y ) ) + ( X.Y * ( Y.Z * Z.X - Y.X * Z.Z ) ) + ( X.Z * ( Y.X * Z.Y - Y.Y * Z.X ) ) );
    }

    /**
     * \brief Compute pushout of a box from a plane.
     */
    SKL_FORCEINLINE SKL_NODISCARD inline skReal SBoxPushOut( const SVector &Normal, const SVector &Size ) noexcept
    {
        return Abs( Normal.X * Size.X ) + Abs( Normal.Y * Size.Y ) + Abs( Normal.Z * Size.Z );
    }

    /**
     * \brief Return a uniformly distributed random unit vector.
     */
    SKL_NODISCARD SVector VRand() noexcept;

    SKL_NODISCARD bool SPlaneAABBIntersects( const SPlane &P, const SBox &AABB ) noexcept;

    SKL_NODISCARD bool SLineExtentBoxIntersection( const SBox &inBox, const SVector &Start, const SVector &End, const SVector &Extent, SVector &HitLocation, SVector &HitNormal, skReal &HitTime ) noexcept;

    /**
     * \brief Find the intersection of an infinite line (defined by two points) and a plane.  Assumes that the line and plane do indeed intersect; you must make sure they're not parallel before calling
     */
    SKL_FORCEINLINE SKL_NODISCARD inline SVector SLinePlaneIntersection(
        const SVector &Point1,
        const SVector &Point2,
        const SVector &PlaneOrigin,
        const SVector &PlaneNormal ) noexcept
    {
        return Point1 + ( Point2 - Point1 ) * ( ( ( PlaneOrigin - Point1 ) | PlaneNormal ) / ( ( Point2 - Point1 ) | PlaneNormal ) );
    }

    SKL_FORCEINLINE SKL_NODISCARD inline SVector SLinePlaneIntersection(
        const SVector &Point1,
        const SVector &Point2,
        const SPlane  &Plane ) noexcept
    {
        return Point1 + ( Point2 - Point1 ) * ( ( Plane.W - ( Point1 | Plane ) ) / ( ( Point2 - Point1 ) | Plane ) );
    }

    /**
     * \brief Determines whether a point is inside a box
     */
    SKL_FORCEINLINE SKL_NODISCARD inline bool SPointBoxIntersection(
        const SVector &Point,
        const SBox    &Box ) noexcept
    {
        if( Point.X >= Box.Min.X && Point.X <= Box.Max.X &&
            Point.Y >= Box.Min.Y && Point.Y <= Box.Max.Y &&
            Point.Z >= Box.Min.Z && Point.Z <= Box.Max.Z )
            return true;

        return false;
    }

    /**
     * \brief Determines whether a line intersects a box
     */
    SKL_FORCEINLINE SKL_NODISCARD inline bool SLineBoxIntersection(
        const SBox    &Box,
        const SVector &Start,
        const SVector &End,
        const SVector &Direction,
        const SVector &OneOverDirection ) noexcept
    {
        SVector Time;
        bool    bStartIsOutside = false;

        if( Start.X < Box.Min.X )
        {
            bStartIsOutside = true;
            if( End.X >= Box.Min.X )
            {
                Time.X = ( Box.Min.X - Start.X ) * OneOverDirection.X;
            }
            else
            {
                return false;
            }
        }
        else if( Start.X > Box.Max.X )
        {
            bStartIsOutside = true;
            if( End.X <= Box.Max.X )
            {
                Time.X = ( Box.Max.X - Start.X ) * OneOverDirection.X;
            }
            else
            {
                return false;
            }
        }
        else
        {
            Time.X = SK_REAL_ZERO;
        }

        if( Start.Y < Box.Min.Y )
        {
            bStartIsOutside = true;
            if( End.Y >= Box.Min.Y )
            {
                Time.Y = ( Box.Min.Y - Start.Y ) * OneOverDirection.Y;
            }
            else
            {
                return false;
            }
        }
        else if( Start.Y > Box.Max.Y )
        {
            bStartIsOutside = true;
            if( End.Y <= Box.Max.Y )
            {
                Time.Y = ( Box.Max.Y - Start.Y ) * OneOverDirection.Y;
            }
            else
            {
                return false;
            }
        }
        else
        {
            Time.Y = SK_REAL_ZERO;
        }

        if( Start.Z < Box.Min.Z )
        {
            bStartIsOutside = true;
            if( End.Z >= Box.Min.Z )
            {
                Time.Z = ( Box.Min.Z - Start.Z ) * OneOverDirection.Z;
            }
            else
            {
                return false;
            }
        }
        else if( Start.Z > Box.Max.Z )
        {
            bStartIsOutside = true;
            if( End.Z <= Box.Max.Z )
            {
                Time.Z = ( Box.Max.Z - Start.Z ) * OneOverDirection.Z;
            }
            else
            {
                return false;
            }
        }
        else
        {
            Time.Z = SK_REAL_ZERO;
        }

        if( bStartIsOutside )
        {
            const skReal MaxTime = Max( Time.X, Max( Time.Y, Time.Z ) );

            if( MaxTime >= SK_REAL_ZERO && MaxTime <= SK_REAL_ONE )
            {
                const SVector    Hit                = Start + Direction * MaxTime;
                constexpr skReal BOX_SIDE_THRESHOLD = SKL_REAL_VALUE( 0.1 );
                if( Hit.X > Box.Min.X - BOX_SIDE_THRESHOLD && Hit.X < Box.Max.X + BOX_SIDE_THRESHOLD &&
                    Hit.Y > Box.Min.Y - BOX_SIDE_THRESHOLD && Hit.Y < Box.Max.Y + BOX_SIDE_THRESHOLD &&
                    Hit.Z > Box.Min.Z - BOX_SIDE_THRESHOLD && Hit.Z < Box.Max.Z + BOX_SIDE_THRESHOLD )
                {
                    return TRUE;
                }
            }

            return false;
        }
        else
        {
            return true;
        }
    }

    /**
     * \brief Determines whether a line intersects a sphere
     */
    SKL_FORCEINLINE SKL_NODISCARD inline bool SLineSphereIntersection( const SVector &Start, const SVector &Dir, skReal Length, const SVector &Origin, skReal Radius ) noexcept
    {
        const SVector EO   = Start - Origin;
        const skReal  v    = ( Dir | ( Origin - Start ) );
        const skReal  disc = Radius * Radius - ( ( EO | EO ) - v * v );

        if( disc >= SK_REAL_ZERO )
        {
            const skReal Time = ( v - Sqrt( disc ) ) / Length;

            if( Time >= SK_REAL_ZERO && Time <= SK_REAL_ONE )
            {
                return true;
            }
        }

        return false;
    }

    /**
     * \brief Compute intersection point of three planes. Return 1 if valid, 0 if infinite
     */
    SKL_FORCEINLINE SKL_NODISCARD inline bool SIntersectPlanes3( SVector &I, const SPlane &P1, const SPlane &P2, const SPlane &P3 ) noexcept
    {
        // Compute determinant, the triple product P1|(P2^P3)==(P1^P2)|P3.
        const skReal Det = ( P1 ^ P2 ) | P3;
        if( Square( Det ) < Square( SKL_REAL_VALUE( 0.001 ) ) )
        {
            // Degenerate.
            I = SVector( 0, 0, 0 );
            return false;
        }

        // Compute the intersection point, guaranteed valid if determinant is nonzero.
        I = ( ( P2 ^ P3 ) * P1.W + ( P3 ^ P1 ) * P2.W + ( P1 ^ P2 ) * P3.W ) / Det;

        return true;
    }

    /**
     * \brief Compute intersection point and direction of line joining two planes.
     * \returns Return true if valid, false if infinite.
     */
    SKL_FORCEINLINE SKL_NODISCARD inline bool SIntersectPlanes2( SVector &I, SVector &D, const SPlane &P1, const SPlane &P2 ) noexcept
    {
        // Compute line direction, perpendicular to both plane normals.
        D               = P1 ^ P2;
        const skReal DD = D.SizeSquared( );
        if( DD < Square( SKL_REAL_VALUE( 0.001 ) ) )
        {
            // Parallel or nearly parallel planes.
            D = I = SVector( 0, 0, 0 );
            return false;
        }

        // Compute intersection.
        I = ( ( P2 ^ D ) * P1.W + ( D ^ P1 ) * P2.W ) / DD;
        D.Normalize( );
        return true;
    }

    SKL_NODISCARD SVectorf ClosestPointOnSegment( const SVectorf &Point, const SVectorf &StartPoint, const SVectorf &EndPoint ) noexcept;

    SKL_NODISCARD SVectord ClosestPointOnSegment( const SVectord &Point, const SVectord &StartPoint, const SVectord &EndPoint ) noexcept;

    SKL_NODISCARD SVectorf ClosestPointOnTriangleToPoint( const SVectorf &Point, const SVectorf &A, const SVectorf &B, const SVectorf &C ) noexcept;

    SKL_NODISCARD SVectord ClosestPointOnTriangleToPoint( const SVectord &Point, const SVectord &A, const SVectord &B, const SVectord &C ) noexcept;
} // namespace SMath
