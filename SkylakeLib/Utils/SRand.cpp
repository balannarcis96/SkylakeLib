//!
//! \file SRand.cpp
//! 
//! \brief Skylake Random abstractions
//! 
//! \source https://youtu.be/LWFzPP8ZbdU?t=2817
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#include "SkylakeLib.h"

namespace SKL
{
    SpinLock      GRand::Lock{};
    Squirrel3Rand GRand::Rand{};

    uint32_t Squirrel3( TRandPosition InPosition, TRandSeed InSeed ) noexcept
    {
        constexpr uint32_t NOISE1 = Squirrel1_NOISE1;
        constexpr uint32_t NOISE2 = Squirrel1_NOISE2;
        constexpr uint32_t NOISE3 = Squirrel1_NOISE3;

        auto Result{ static_cast< uint32_t >( InPosition ) };

        //Apply noise pass 1
        Result *= NOISE1;
        Result ^= ( Result >> 8 );

        //Apply seed
        Result += InSeed;

        //Apply noise pass 2
        Result += NOISE2;
        Result ^= ( Result << 8 );

        //Apply noise pass 3
        Result *= NOISE3;
        Result ^= ( Result >> 8 );

        return Result;
    }

    TRandPosition Squirrel3Rand::NextSeed() noexcept
    {
        // Use the time value as the seed
        Seed = static_cast< TRandSeed >( time( nullptr ) );
        Position = 1;

        return Position;
    }

    TRandPosition Squirrel3Rand::GetPosition() noexcept
    {
        //If reached max position for this seed, reseed
        if ( static_cast< uint32_t >( Position ) + 1 >= std::numeric_limits<uint32_t>::max() ) SKL_UNLIKELY
        {
            return NextSeed( );
        }

        return ++Position;
    }
}