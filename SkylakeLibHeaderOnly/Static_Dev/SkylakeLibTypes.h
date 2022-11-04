//!
//! \file SkylakeLibTypes.h
//! 
//! \brief Base types for SkylakeLibHeaderOnly
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 

#if SKL_USE_LARGE_WORLD_COORDS
    using skReal = double;
#else
    using skReal = float;
#endif

namespace SKL
{
    using TClock             = std::chrono::high_resolution_clock;
    using TClockTimePoint    = std::chrono::high_resolution_clock::time_point;
    using TDuration          = int32_t;
    using TTimePoint         = TDuration;
    using TSystemTimePoint   = uint64_t;
    using TEpochTimePoint    = uint64_t;
    using TEpochTimeDuration = uint64_t;
    using TObjectId          = uint32_t;
    using TDatabaseId        = uint64_t;

    using BOOL16 = int16_t;
    using BOOL   = int32_t;
    using UBOOL  = uint32_t;

    constexpr TObjectId   CObjectIdNone     = 0;
    constexpr TObjectId   CObjectIdMax      = std::numeric_limits<TObjectId>::max( );
    constexpr TDatabaseId CDatabaseId_None  = 0;
    constexpr TDuration   CInfiniteDuration = 8888888;
    
    struct SRawAngle
    {
        int16_t Angle{ 0 };

        SKL_FORCEINLINE static SRawAngle FromDeg( float InDeg ) noexcept
        {
            return { 
                static_cast<int16_t>( ( 32767.0f / 180.0f ) * InDeg ) 
            };
        }
        SKL_FORCEINLINE static SRawAngle FromDeg( double InDeg ) noexcept
        {
            return { 
                static_cast<int16_t>( ( 32767.0 / 180.0 ) * InDeg ) 
            };
        }
        SKL_FORCEINLINE float ToDeg() const noexcept
        {
            return static_cast<float>( Angle ) * ( 180.0f / 32767.0f );
        }
        SKL_FORCEINLINE double ToDegd() const noexcept
        {
            return static_cast<double>( Angle ) * ( 180.0 / 32767.0 );
        }

        SKL_FORCEINLINE static SRawAngle FromRad( float InRad ) noexcept
        {
            return { 
                static_cast<int16_t>( ( 32767.0f / 3.1415926535897932f ) * InRad ) 
            };
        }
        SKL_FORCEINLINE static SRawAngle FromRad( double InRad ) noexcept
        {
            return { 
                static_cast<int16_t>( ( 32767.0 / 3.1415926535897932  ) * InRad ) 
            };
        }
        SKL_FORCEINLINE float ToRad() const noexcept
        {
            return static_cast<float>( Angle ) * ( 3.1415926535897932f / 32767.0f );
        }
        SKL_FORCEINLINE double ToRadd() const noexcept
        {
            return static_cast<double>( Angle ) * ( 3.1415926535897932 / 32767.0 );
        }
    };

    struct SRawVector2
    {
        skReal X{ SKL_REAL_VALUE( 0.0 ) };
        skReal Y{ SKL_REAL_VALUE( 0.0 ) };
    };

    struct SRawVector2f
    {
        float X{ 0.0f };
        float Y{ 0.0f };
    };

    struct SRawVector2d
    {
        double X{ 0.0 };
        double Y{ 0.0 };
    };

    struct SRawVector
    {
        skReal X{ SKL_REAL_VALUE( 0.0 ) };
        skReal Y{ SKL_REAL_VALUE( 0.0 ) };
        skReal Z{ SKL_REAL_VALUE( 0.0 ) };
    };
    
    struct SRawVectorf
    {
        float X{ 0.0f };
        float Y{ 0.0f };
        float Z{ 0.0f };
    };
    
    struct SRawVectord
    {
        double X{ 0.0 };
        double Y{ 0.0 };
        double Z{ 0.0 };
    };

    struct SRawPlane
    {
        skReal X{ SKL_REAL_VALUE( 0.0 ) };
        skReal Y{ SKL_REAL_VALUE( 0.0 ) };
        skReal Z{ SKL_REAL_VALUE( 0.0 ) };
        skReal W{ SKL_REAL_VALUE( 0.0 ) };
    };

    struct SRawPlanef
    {
        float X{ 0.0f };
        float Y{ 0.0f };
        float Z{ 0.0f };
        float W{ 0.0f };
    };

    struct SRawPlaned
    {
        double X{ 0.0 };
        double Y{ 0.0 };
        double Z{ 0.0 };
        double W{ 0.0 };
    };
}
