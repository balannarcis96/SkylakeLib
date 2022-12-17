//!
//! \file SpinLock.h
//! 
//! \brief Efficient SpinLock abstraction
//! 
//! \reference https://rigtorp.se/spinlock/ (Correctly implementing a spinlock in C++)
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    struct FakeSpinLock
    {
        SKL_FORCEINLINE void Lock() noexcept {}
        SKL_FORCEINLINE bool TryLock() noexcept {}
        SKL_FORCEINLINE void Unlock() noexcept {}

        // std compatible 
        SKL_FORCEINLINE void lock() noexcept {}
        SKL_FORCEINLINE void try_lock() noexcept {}
        SKL_FORCEINLINE void unlock() noexcept {}
    };

    struct SpinLock
    {
        void Lock() noexcept
        {
            for(;;)
            {
                // Optimistically assume the lock is free on the first try
                if( false == LockFlag.exchange( true, std::memory_order_acquire ) ) 
                {
                    return;
                }

                // Wait for lock to be released without generating cache misses
                while( LockFlag.load( std::memory_order_relaxed ) ) {
                    // Issue X86 PAUSE or ARM YIELD instruction to reduce contention between
                    // hyper-threads
                    _mm_pause();
                }   
            }
        }    
   
        SKL_FORCEINLINE bool TryLock() noexcept 
        {
            // First do a relaxed load to check if lock is free in order to prevent
            // unnecessary cache misses if someone does while(!try_lock())
            return false == LockFlag.load( std::memory_order_relaxed ) &&
                   false == LockFlag.exchange( true, std::memory_order_acquire );
        }

        SKL_FORCEINLINE void Unlock() noexcept 
        {
            LockFlag.store( false, std::memory_order_release );
        }

        // std compatible 
        SKL_FORCEINLINE void lock() noexcept { Lock(); }
        SKL_FORCEINLINE void try_lock() noexcept { TryLock(); }
        SKL_FORCEINLINE void unlock() noexcept { Unlock(); }

    private:
        std::atomic<bool> LockFlag { false };
    };  

    template<typename TSpinLock>
    struct SpinLockScopeGuard
    {
        static_assert( std::is_same_v<TSpinLock, SpinLock> || std::is_same_v<TSpinLock, FakeSpinLock> );

        SpinLockScopeGuard( TSpinLock& TargetSpinLock ) noexcept : TargetSpinLock{ TargetSpinLock } {
            TargetSpinLock.Lock();
        }
            
        ~SpinLockScopeGuard() noexcept
        {
            TargetSpinLock.Unlock();
        }

    private:
        TSpinLock& TargetSpinLock;
    };
}