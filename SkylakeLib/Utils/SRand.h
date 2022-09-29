//!
//! \file SRand.h
//! 
//! \brief Skylake Random abstractions
//! 
//! \source https://youtu.be/LWFzPP8ZbdU?t=2817
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    using TRandSeed     = uint32_t;
    using TRandPosition = int32_t;

    //! Reliable seedable noise function
    uint32_t Squirrel3( TRandPosition InPosition, TRandSeed InSeed ) noexcept;

    //! Reliable seedable 2D noise function
    SKL_FORCEINLINE uint32_t Squirrel3_2D( int32_t InX, int32_t InY, TRandSeed InSeed ) noexcept
    {
        constexpr uint32_t PRIME{ Squirrel3_2D_PRIME };
        return Squirrel3( InX + ( PRIME * InY ), InSeed );
    }
    
    //! Reliable seedable 3D noise function
    SKL_FORCEINLINE uint32_t Squirrel3_3D( int32_t InX, int32_t InY, int32_t InZ, TRandSeed InSeed ) noexcept
    {
        constexpr uint32_t PRIME1{ Squirrel3_3D_PRIME1 };
        constexpr uint32_t PRIME2{ Squirrel3_3D_PRIME2 };
        return Squirrel3( InX + ( PRIME1 * InY )+ ( PRIME2 * InZ ), InSeed );
    }

    struct Squirrel3Rand
    {
        Squirrel3Rand() noexcept { NextSeed(); }
        ~Squirrel3Rand() noexcept = default;

        //! Generate pseudo random value in interval [0, UINT32_MAX]
        SKL_FORCEINLINE uint32_t NextRandom() noexcept
        {
            return Squirrel3( GetPosition(), Seed );
        }

        //! Generate pseudo random value in interval [InMin, InMax]
        SKL_FORCEINLINE uint32_t NextRandomInRange( uint32_t InMin, uint32_t InMax ) noexcept
        {
            return ( Squirrel3( GetPosition(), Seed ) % InMax ) + InMin;
        }

        //! Generate pseudo random value in interval (0.0f, 1.0f]
        SKL_FORCEINLINE float NextRandomF() noexcept
        {
            constexpr auto FMaxUInt32 = static_cast<float>( std::numeric_limits<uint32_t>::max() );
            return static_cast<float>( NextRandom() ) / FMaxUInt32;
        }
        
        //! Generate pseudo random value in interval (0.0, 1.0]
        SKL_FORCEINLINE double NextRandomD() noexcept
        {
            constexpr auto DMaxUInt32 = static_cast<double>( std::numeric_limits<uint32_t>::max() );
            return static_cast<double>( NextRandom() ) / DMaxUInt32;
        }

    private:
        TRandPosition NextSeed() noexcept;
        TRandPosition GetPosition() noexcept;

        TRandSeed     Seed    { 0 }; //!< Seed of this rand instance
        TRandPosition Position{ 0 }; //!< Position to generate the next noise from
    };

    //! Thread local rand facillity
    struct TRand final: private ITLSSingleton<TRand>
    {
        static void InitializeThread() noexcept
        {
            if( nullptr == GetInstance() )
            {
                const auto Result{ Create() };
                SKL_ASSERT( RSuccess == Result );
            }
        }
        static void ShutdownThread() noexcept
        {
            Destroy();
        }

        //! Generate pseudo random value in interval [0, UINT32_MAX]
        SKL_FORCEINLINE static uint32_t NextRandom() noexcept
        {
            SKL_ASSERT( nullptr != GetInstance() );
            return GetInstance()->Rand.NextRandom();
        }

        //! Generate pseudo random value in interval [InMin, InMax]
        SKL_FORCEINLINE static uint32_t NextRandomInRange( uint32_t InMin, uint32_t InMax ) noexcept
        {
            SKL_ASSERT( nullptr != GetInstance() );
            return GetInstance()->Rand.NextRandomInRange( InMin, InMax );
        }

        //! Generate pseudo random value in interval (0.0f, 1.0f]
        SKL_FORCEINLINE static float NextRandomF() noexcept
        {
            SKL_ASSERT( nullptr != GetInstance() );
            return GetInstance()->Rand.NextRandomF();
        }
        
        //! Generate pseudo random value in interval (0.0, 1.0]
        SKL_FORCEINLINE static double NextRandomD() noexcept
        {
            SKL_ASSERT( nullptr != GetInstance() );
            return GetInstance()->Rand.NextRandomD();
        }

        RStatus Initialize( ) noexcept override { return RSuccess; }
        const char *GetName( ) const noexcept override { return "[TRand]"; }

    private:
        Squirrel3Rand Rand{};
    };

    //! Global, thread safe, rand facillity
    struct GRand final
    {
        //! Generate pseudo random value in interval [0, UINT32_MAX]
        SKL_FORCEINLINE static uint32_t NextRandom() noexcept
        {
            SpinLockScopeGuard Guard{ Lock };
            return Rand.NextRandom();
        }

        //! Generate pseudo random value in interval [InMin, InMax]
        SKL_FORCEINLINE static uint32_t NextRandomInRange( uint32_t InMin, uint32_t InMax ) noexcept
        {
            SpinLockScopeGuard Guard{ Lock };
            return Rand.NextRandomInRange( InMin, InMax );
        }

        //! Generate pseudo random value in interval (0.0f, 1.0f]
        SKL_FORCEINLINE static float NextRandomF() noexcept
        {
            SpinLockScopeGuard Guard{ Lock };
            return Rand.NextRandomF();
        }
        
        //! Generate pseudo random value in interval (0.0, 1.0]
        SKL_FORCEINLINE static double NextRandomD() noexcept
        {
            SpinLockScopeGuard Guard{ Lock };
            return Rand.NextRandomD();
        }

    private:
        static SpinLock      Lock;
        static Squirrel3Rand Rand;
    };
}