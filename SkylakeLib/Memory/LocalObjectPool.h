#pragma once
//!
//! \file LocalObjectPool.h
//! 
//! \brief ObjectPool: Ring buffer based [thread safe?] object pool
//!         bNoSync:
//!             [true] : No thread synchronization 
//!             [false]: Use thread synchronization [default]
//!         bUseSpinLock:
//!             [true] : SpinLock is used for thread synchronization [default]
//!             [false]: Atomic operations are used for thread synchronization (might spill when under heavy contention)
//!         
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 

namespace SKL
{
    template<typename T, size_t PoolSize, bool TbNoSync = false, bool TbUseSpinLock = true, bool TbPerformConstruction = true, bool TbPerformDestruction = true, size_t TAlignment = SKL_ALIGNMENT>
    class LocalObjectPool
    {
    public:
        struct PoolTraits
        {
            static constexpr size_t MyObjectSize        { sizeof( T ) };
            static constexpr size_t MyPoolSize          { PoolSize };
            static constexpr size_t MyPoolMask          { PoolSize - 1 };
            static constexpr size_t Alignment           { TAlignment };
            static constexpr size_t InternalAlignment   { true == TbNoSync ? SKL_ALIGNMENT : SKL_CACHE_LINE_SIZE };
            static constexpr bool   bNoSync             { TbNoSync };
            static constexpr bool   bUseSpinLock        { true == bNoSync || true == TbUseSpinLock };
            static constexpr bool   bPerformConstruction{ TbPerformConstruction };
            static constexpr bool   bPerformDestruction { TbPerformDestruction };

            using MyPoolType       = T;
            using MyType           = ObjectPool<T, PoolSize>;
            using TPoolHead        = std::conditional_t<( true == bNoSync || true == bUseSpinLock ), uint64_t, std::atomic_uint64_t>;
            using TPoolTail        = std::conditional_t<( true == bNoSync || true == bUseSpinLock ), uint64_t, std::atomic_uint64_t>;
            using TPoolPtr         = std::conditional_t<( true == bNoSync || true == bUseSpinLock ), void*, std::atomic<void*>>;
            using TPoolSpinLock    = std::conditional_t<bNoSync, SKL::FakeSpinLock, SKL::SpinLock>;
            using TStatisticsValue = std::conditional_t<bNoSync, uint64_t, std::atomic_uint64_t>;

            static_assert( ( MyPoolSize & MyPoolMask ) == 0, "LocalObjectPool size must be a power of 2" );
        };

        //! Preallocate and fill the whole Pool with [PoolSize] elements
        constexpr RStatus Preallocate() noexcept
        {
            // ! Hopefully SKL_MALLOC_ALIGNED will allocate in a continuous fashion.
            for( size_t i = 0; i < PoolSize; i++ )
            {
                Pool[i] = SKL_MALLOC_ALIGNED( sizeof( T ), PoolTraits::Alignment );

                SKL_IFSHIPPING( memset( Pool[i], 0, sizeof( T ) ) );

                if( Pool[i] == nullptr )
                {
                    return RFail;
                }
            }

            return RSuccess;
        }

        //! Safely free all pool blocks
        constexpr void FreePool() noexcept
        {
            if constexpr ( false == PoolTraits::bNoSync )
            {
                if constexpr( true == PoolTraits::bUseSpinLock )
                {
                    { //Critical section
                        SpinLockScopeGuard Guard{ SpinLock };

                        for( size_t i = 0; i < PoolSize; i++ )
                        {
                            auto* PopValue = Pool[i];
                            Pool[i] = nullptr;
                            if( nullptr != PopValue )
                            {
                                SKL_FREE_SIZE_ALIGNED( PopValue, PoolTraits::MyObjectSize, PoolTraits::Alignment );
                            }
                        }
                    }
                }
                else
                {
                    for( size_t i = 0; i < PoolSize; i++ )
                    {
                        auto* PopValue = Pool[i].exchange( nullptr );
                        if( nullptr != PopValue )
                        {
                            SKL_FREE_SIZE_ALIGNED( PopValue, PoolTraits::MyObjectSize, PoolTraits::Alignment );
                        }
                    }
                }
            }
            else
            {
                for( size_t i = 0; i < PoolSize; i++ )
                {
                    auto* PopValue = Pool[i];
                    Pool[i] = nullptr;
                    if( nullptr != PopValue )
                    {
                        SKL_FREE_SIZE_ALIGNED( PopValue, PoolTraits::MyObjectSize, PoolTraits::Alignment );
                    }
                }
            }

#if defined(SKL_MEMORY_STATISTICS) 
            TotalAllocations     = 0;
            TotalDeallocations   = 0;
            TotalOSAllocations   = 0;
            TotalOSDeallocations = 0;
#endif
        }

        //! Allocate new T
        template<typename... TArgs>
        SKL_FORCEINLINE SKL_NODISCARD constexpr T *Allocate( TArgs... Args ) noexcept
        {
            return AllocateImpl( std::forward<TArgs>( Args )... );
        }

        //! Allocate new T
        SKL_FORCEINLINE SKL_NODISCARD constexpr T *Allocate() noexcept
        {
            return AllocateImpl();
        }

        //! Zero all pool items [not thread safe]
        constexpr void ZeroAllMemory() noexcept
        {
            for( size_t i = 0; i < PoolSize; i++ )
            {
                memset( Pool[i], 0, sizeof( T ) );
            }
        }

        //! Get GUID of this Pool instance
        SKL_FORCEINLINE SKL_NODISCARD constexpr size_t GetPoolId() noexcept
        {
            return reinterpret_cast<size_t>( &PoolTraits::MyType::Preallocate );
        }

#if defined(SKL_MEMORY_STATISTICS) 
        SKL_FORCEINLINE SKL_NODISCARD constexpr size_t GetTotalDeallocations() noexcept
        {
            return TotalDeallocations.load( std::memory_order_acquire );
        }

        SKL_FORCEINLINE SKL_NODISCARD constexpr size_t GetTotalAllocations() noexcept
        {
            return TotalAllocations.load( std::memory_order_acquire );
        }

        SKL_FORCEINLINE SKL_NODISCARD constexpr size_t GetTotalOSDeallocations() noexcept
        {
            return TotalOSDeallocations.load( std::memory_order_acquire );
        }

        SKL_FORCEINLINE SKL_NODISCARD constexpr size_t GetTotalOSAllocations() noexcept
        {
            return TotalOSAllocations.load( std::memory_order_acquire );
        }
#endif

        //! Deallocate T
        constexpr void Deallocate( T *Obj ) noexcept
        {
            static_assert( std::is_nothrow_destructible_v<T> 
                        || false == PoolTraits::bPerformDestruction 
                        || false == PoolTraits::bPerformConstruction
                        ,  "LocalObjectPool::Deallocate() T must be nothrow destructible" );

            SKL_ASSERT( reinterpret_cast<uint64_t>( Obj ) % PoolTraits::Alignment == 0 );

            if constexpr( PoolTraits::bPerformDestruction && PoolTraits::bPerformConstruction )
            {
                //Call destructor manually
                GDestructNothrow( Obj );
            }

            void* PrevVal;

            if constexpr( PoolTraits::bUseSpinLock )
            {
                { //Critical section
                    SpinLockScopeGuard Guard{ SpinLock };

                    const uint64_t InsPos{ TailPosition++ };

                    PrevVal                               = Pool[InsPos & PoolTraits::MyPoolMask];
                    Pool[InsPos & PoolTraits::MyPoolMask] = reinterpret_cast<void*>( Obj );
                }
            }
            else
            {
                const uint64_t InsPos{ TailPosition.fetch_add( 1, std::memory_order_relaxed ) };
                PrevVal = Pool[InsPos & PoolTraits::MyPoolMask].exchange( reinterpret_cast<void*>( Obj ) );
            }

            if( nullptr != PrevVal ) SKL_UNLIKELY
            {
                // stomped over valid pointer, just deallocate to OS
                SKL_FREE_SIZE_ALIGNED( PrevVal, PoolTraits::MyObjectSize, PoolTraits::Alignment );
                SKL_IFMEMORYSTATS( ++TotalOSDeallocations );
                return;
            }

            SKL_IFMEMORYSTATS( ++TotalDeallocations );
        }

        constexpr T* Debug_ProbeAt( uint64_t InIndex ) noexcept
        {
            if constexpr( false == PoolTraits::bNoSync && false == PoolTraits::bUseSpinLock )
            {
                return reinterpret_cast<T*>( Pool[InIndex & PoolTraits::MyPoolMask].load( std::memory_order_acquire ) );
            }
            else
            {
                return reinterpret_cast<T*>( Pool[InIndex & PoolTraits::MyPoolMask] );
            }
        }

    private:
        template<typename... TArgs>
        SKL_NODISCARD SKL_FORCEINLINE constexpr T *AllocateImpl( TArgs... Args ) noexcept
        {
            T *Allocated;

            if constexpr( PoolTraits::bUseSpinLock )
            {
                { //Critical section
                    SpinLockScopeGuard Guard{ SpinLock };

                    const uint64_t PopPos{ HeadPosition++ };

                    Allocated                             = reinterpret_cast<T*>( Pool[PopPos & PoolTraits::MyPoolMask] );
                    Pool[PopPos & PoolTraits::MyPoolMask] = nullptr;
                }
            }
            else
            {
                const uint64_t PopPos{ HeadPosition.fetch_add( 1, std::memory_order_relaxed ) };
                Allocated = reinterpret_cast<T*>( Pool[PopPos & PoolTraits::MyPoolMask].exchange( nullptr ) );
            }

            if( nullptr == Allocated ) SKL_UNLIKELY
            {
                // dequeued nullptr, allocate from OS
                Allocated = reinterpret_cast<T*>( SKL_MALLOC_ALIGNED( PoolTraits::MyObjectSize, PoolTraits::Alignment ) );
                if( nullptr == Allocated ) SKL_UNLIKELY
                {
                    return nullptr;
                }

                SKL_IFMEMORYSTATS( ++TotalOSAllocations );
            }

            if constexpr( PoolTraits::bPerformConstruction )
            {
                GConstructNothrow<T>( Allocated, std::forward<TArgs>( Args )... );
            }

            SKL_IFMEMORYSTATS( ++TotalAllocations );
            SKL_ASSERT( reinterpret_cast<uint64_t>( Allocated ) % PoolTraits::Alignment == 0 );

            return Allocated;
        }

        alignas( PoolTraits::InternalAlignment ) typename PoolTraits::TPoolHead     HeadPosition  { 0U };
        alignas( PoolTraits::InternalAlignment ) typename PoolTraits::TPoolTail     TailPosition  { 0U };
        alignas( PoolTraits::InternalAlignment ) typename PoolTraits::TPoolPtr      Pool[PoolSize]{};
        alignas( PoolTraits::InternalAlignment ) typename PoolTraits::TPoolSpinLock SpinLock      {};
 
#if defined(SKL_MEMORY_STATISTICS) 
        alignas( PoolTraits::InternalAlignment ) typename PoolTraits::TStatisticsValue TotalAllocations    { 0U };
        alignas( PoolTraits::InternalAlignment ) typename PoolTraits::TStatisticsValue TotalDeallocations  { 0U };
        alignas( PoolTraits::InternalAlignment ) typename PoolTraits::TStatisticsValue TotalOSAllocations  { 0U };
        alignas( PoolTraits::InternalAlignment ) typename PoolTraits::TStatisticsValue TotalOSDeallocations{ 0U };
#endif
    };
} // namespace SKL