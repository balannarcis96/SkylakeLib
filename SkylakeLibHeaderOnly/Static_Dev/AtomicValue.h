//!
//! \file AtomicValue.h
//! 
//! \brief Extension to the std atomic library, relaxed/acquire+release atomic wrapper 
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

#include <atomic>

namespace std
{
    template<typename T, bool bDefaultTo_Relaxed_AcquireRelease = true>
    class interlocked_value
    {
        using TOutType  = T;
        using TType     = T;
        using MyType    = interlocked_value<T, bDefaultTo_Relaxed_AcquireRelease>;

        static_assert( is_pointer_v<TOutType> || is_integral_v<TOutType> || is_enum_v<TOutType>, "T can not be used!" );
        static_assert( std::atomic<TType>::is_always_lock_free, "T can not be used, not lock free!" );

    public:
        interlocked_value() noexcept :
            AtomicWrappedValue{ }
        {}

        interlocked_value( TOutType InValue ) noexcept
        {
            store_relaxed( InValue );
        }

        // Store relaxed
        SKL_FORCEINLINE void store_relaxed( TOutType InValue ) noexcept
        {
            AtomicWrappedValue.store( InValue, std::memory_order_relaxed );
        }

        // Store release
        SKL_FORCEINLINE void store_release( TOutType InValue ) noexcept
        {
            AtomicWrappedValue.store( InValue, std::memory_order_release );
        }

        // Store
        SKL_FORCEINLINE void store( TOutType InValue ) noexcept
        {
            if constexpr( bDefaultTo_Relaxed_AcquireRelease )
            {
                store_relaxed( InValue );
            }
            else
            {
                store_release( InValue );
            }
        }

        // Load relaxed
        SKL_FORCEINLINE TOutType load_relaxed() const noexcept
        {
            return AtomicWrappedValue.load( std::memory_order_relaxed );
        }

        // Load acquire
        SKL_FORCEINLINE TOutType load_acquire() const noexcept
        {
            return AtomicWrappedValue.load( std::memory_order_acquire );
        }

        // Load
        SKL_FORCEINLINE TOutType load() const noexcept
        {
            if constexpr( bDefaultTo_Relaxed_AcquireRelease )
            {
                return load_relaxed();
            }
            else
            {
                return load_acquire();
            }
        }

        // Implicit and explicit load
        SKL_FORCEINLINE operator TOutType() const noexcept
        {
            return load( );
        }

        // Store
        SKL_FORCEINLINE void operator=( TOutType InValue ) noexcept
        {
            store( InValue );
        }

        // Operator ->, available only for pointer types
        SKL_FORCEINLINE TType operator->() noexcept
        {
            static_assert( std::is_pointer_v< TType >, "TType must of ptr type" );

            return load( );
        }

        // Operator -> const, available only for pointer types
        SKL_FORCEINLINE TType operator->() const noexcept
        {
            static_assert( std::is_pointer_v< TType >, "TType must of ptr type" );

            return load( );
        }

        // Copy
        template<typename TOther, bool _bDefaultTo_Relaxed_AcquireRelease>
        SKL_FORCEINLINE interlocked_value( const interlocked_value<TOther, _bDefaultTo_Relaxed_AcquireRelease> &Other ) noexcept
        {
            store( static_cast< TOutType >( Other.load( ) ) );
        }

        // Copy
        template<typename TOther, bool _bDefaultTo_Relaxed_AcquireRelease>
        SKL_FORCEINLINE MyType &operator=( const interlocked_value<TOther, _bDefaultTo_Relaxed_AcquireRelease> &Other ) noexcept
        {
            SKL_ASSERT( this != &Other );
            store( static_cast< TOutType >( Other.load() ) );
            return *this;
        }

        // Compare exchange
        SKL_FORCEINLINE bool cas( TOutType InValue, TOutType& InExpected ) noexcept
        {
            return AtomicWrappedValue.compare_exchange_weak( InExpected, InValue, std::memory_order_release, std::memory_order_relaxed );
        }

        // Exchange values
        SKL_FORCEINLINE TOutType exchange( TOutType InValue ) noexcept
        {
            return AtomicWrappedValue.exchange( InValue, std::memory_order_acq_rel );
        }

        //@TODO math and logic operators

        // Decrease the value by 1 and return the value before the decrement.
        SKL_FORCEINLINE TOutType decrement() noexcept
        {
            return AtomicWrappedValue.fetch_sub( 1, std::memory_order_acq_rel );
        }
        
        // Decrease the value by ByValue and return the value before the decrement.
        SKL_FORCEINLINE TOutType decrement( TType ByValue ) noexcept
        {
            return AtomicWrappedValue.fetch_sub( ByValue, std::memory_order_acq_rel );
        }

        // Increments the value by 1 and return the value before the increment.
        SKL_FORCEINLINE TOutType increment() noexcept
        {
            return AtomicWrappedValue.fetch_add( 1, std::memory_order_acq_rel );
        }
        
        // Increments the value by ByValue and return the value before the increment.
        SKL_FORCEINLINE TOutType increment( TType ByValue ) noexcept
        {
            return AtomicWrappedValue.fetch_add( ByValue, std::memory_order_acq_rel );
        }

    private:
        std::atomic<TType> AtomicWrappedValue;
    };

    /**
     * \brief All operations on this value are atomic with relaxed loads and stores
     * \notice cas() and exchange() calls are synced (not relaxed)
     */
    template<typename T>
    using relaxed_value = interlocked_value<T, true>;

    /**
     * \brief All operations on this value are atomic with acquire loads and release stores
     * \notice cas() and exchange() calls are synced (not relaxed)
     */
    template<typename T>
    using synced_value = interlocked_value<T, false>;

} // namespace std
