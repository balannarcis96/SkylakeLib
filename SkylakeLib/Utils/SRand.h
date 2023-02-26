//!
//! \file SRand.h
//! 
//! \brief Skylake Random abstractions
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    //! Thread local rand facility
    struct TRand final: private ITLSSingleton<TRand>
    {
        static void InitializeThread() noexcept
        {
            if( nullptr == GetInstance() )
            {
                const auto Result{ Create() };
                SKL_ASSERT( RSuccess == Result );
                ( void )Result;
            }
        }
        static void ShutdownThread() noexcept
        {
            Destroy();
        }

        //! Generate pseudo random value in interval [0, UINT32_MAX]
        SKL_FORCEINLINE SKL_NODISCARD static uint32_t NextRandom() noexcept
        {
            SKL_ASSERT( nullptr != GetInstance() );
            return GetInstance()->Rand.NextRandom();
        }

        //! Generate pseudo random value in interval [InMin, InMax]
        SKL_FORCEINLINE SKL_NODISCARD static uint32_t NextRandomInRange( uint32_t InMin, uint32_t InMax ) noexcept
        {
            SKL_ASSERT( nullptr != GetInstance() );
            return GetInstance()->Rand.NextRandomInRange( InMin, InMax );
        }

        //! Generate pseudo random value in interval (0.0f, 1.0f]
        SKL_FORCEINLINE SKL_NODISCARD static float NextRandomF() noexcept
        {
            SKL_ASSERT( nullptr != GetInstance() );
            return GetInstance()->Rand.NextRandomF();
        }
        
        //! Generate pseudo random value in interval (0.0, 1.0]
        SKL_FORCEINLINE SKL_NODISCARD static double NextRandomD() noexcept
        {
            SKL_ASSERT( nullptr != GetInstance() );
            return GetInstance()->Rand.NextRandomD();
        }

        RStatus Initialize( ) noexcept override { return RSuccess; }
        const char *GetName( ) const noexcept override { return "[TRand]"; }

    private:
        Squirrel3Rand Rand{};
    };

    //! Global, thread safe, rand facility
    struct GRand final
    {
        //! Generate pseudo random value in interval [0, UINT32_MAX]
        SKL_FORCEINLINE SKL_NODISCARD static uint32_t NextRandom() noexcept
        {
            SpinLockScopeGuard Guard{ Lock };
            return Rand.NextRandom();
        }

        //! Generate pseudo random value in interval [InMin, InMax]
        SKL_FORCEINLINE SKL_NODISCARD static uint32_t NextRandomInRange( uint32_t InMin, uint32_t InMax ) noexcept
        {
            SpinLockScopeGuard Guard{ Lock };
            return Rand.NextRandomInRange( InMin, InMax );
        }

        //! Generate pseudo random value in interval (0.0f, 1.0f]
        SKL_FORCEINLINE SKL_NODISCARD static float NextRandomF() noexcept
        {
            SpinLockScopeGuard Guard{ Lock };
            return Rand.NextRandomF();
        }
        
        //! Generate pseudo random value in interval (0.0, 1.0]
        SKL_FORCEINLINE SKL_NODISCARD static double NextRandomD() noexcept
        {
            SpinLockScopeGuard Guard{ Lock };
            return Rand.NextRandomD();
        }

    private:
        static SpinLock      Lock;
        static Squirrel3Rand Rand;
    };
}