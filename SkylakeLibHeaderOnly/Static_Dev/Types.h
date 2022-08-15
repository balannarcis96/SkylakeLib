//!
//! \file Types.h
//! 
//! \brief Base types for SkylakeLibHeaderOnly
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 

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

    using BOOL16 = int16_t;
    using BOOL   = int32_t;
    using UBOOL  = uint32_t;

    constexpr TObjectId  TObjectIdNone = 0;
    constexpr TObjectId  TObjectIdMax  = std::numeric_limits< uint32_t >::max( );
    
#if defined(SKL_REAL_TYPE_SINGLE)
    using TReal = float;
#elif defined(SKL_REAL_TYPE_DOUBLE)
    using TReal = double;
#else
    #error "Unknown real type!"
#endif

    constexpr TReal TRealZero = SK_REAL_ZERO;
    constexpr TReal TRealOne  = SK_REAL_ONE;
    constexpr TReal TRealUnit = SK_REAL_ONE;
}

using TSkylakeReal                      = SKL::TReal;
constexpr TSkylakeReal TSkylakeRealZero = SKL::TRealZero;
constexpr TSkylakeReal TSkylakeRealOne  = SKL::TRealOne;
constexpr TSkylakeReal TSkylakeRealUnit = SKL::TRealUnit;

