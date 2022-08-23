#pragma once
//!
//! \file StaticObjectPool.h
//! 
//! \brief ObjectPool: Ring buffer based [thread safe?] object pool
//!         bNoSync:
//!             [true] : No thread syncronization 
//!             [false]: Use thread syncronization [default]
//!         bUseSpinLock:
//!             [true] : SpinLock is used for thread synchronization [default]
//!             [false]: Atomic operations are used for thread synchronization (might spill when under heavy contention)
//!         
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 

namespace SKL
{
    template< typename T, size_t PoolSize, bool TbNoSync = false, bool TbUseSpinLock = true, bool TbPerformConstruction = true, bool TbPerformDestruction = true, size_t TAlignment = SKL_ALIGNMENT >
    class ObjectPool
    {
    public:
        struct PoolTraits
        {
            static constexpr size_t MyObjectSize         { sizeof( T ) };
            static constexpr size_t MyPoolSize           { PoolSize };
            static constexpr size_t MyPoolMask           { PoolSize - 1 };
            static constexpr size_t Alignment            { TAlignment };
            static constexpr bool   bNoSync              { TbNoSync };
            static constexpr bool   bUseSpinLock         { bNoSync || TbUseSpinLock };
            static constexpr bool   bPerformConstruction { TbPerformConstruction };
            static constexpr bool   bPerformDestruction  { TbPerformDestruction };

            using MyPoolType    = T;
            using MyType        = ObjectPool< T, PoolSize >;
            using TPoolHead     = std::conditional_t<( bNoSync || bUseSpinLock ), uint64_t, std::atomic<uint64_t>>;
            using TPoolTail     = std::conditional_t<( bNoSync || bUseSpinLock ), uint64_t, std::atomic<uint64_t>>;
            using TPoolPtr      = std::conditional_t<( bNoSync || bUseSpinLock ), void*, std::atomic<void*>>;
            using TPoolSpinLock = std::conditional_t<bNoSync, SKL::FakeSpinLock, SKL::SpinLock>;

            static_assert( ( MyPoolSize & MyPoolMask ) == 0, "ObjectPool size must be a power of 2" );
        };

        //Preallocate and fill the whole Pool with [PoolSize] elements
        constexpr static RStatus Preallocate( ) noexcept
        {
            // ! Hopefully SKL_MALLOC_ALIGNED will allocate in a continuous fashion.
            for( size_t i = 0; i < PoolSize; i++ )
            {
                Pool[ i ] = SKL_MALLOC_ALIGNED( PoolTraits::MyObjectSize, PoolTraits::Alignment );
                mi_assert(((uintptr_t)Pool[ i ] % PoolTraits::Alignment) == 0);
                SKL_ASSERT( ((uintptr_t)Pool[ i ]) % PoolTraits::Alignment == 0 );

                SKL_IFSHIPPING( memset( Pool[ i ], 0, sizeof( T ) ) );

                if( Pool[ i ] == nullptr )
                {
                    return RFail;
                }
            }

            return RSuccess;
        }

        constexpr static void FreePool() noexcept
        {
            if constexpr ( false == PoolTraits::bNoSync )
            {
                if constexpr( true == PoolTraits::bUseSpinLock )
                {
                    { //Critical section
                        SpinLockScopeGuard Guard { SpinLock };

                        for( size_t i = 0; i < PoolSize; i++ )
                        {
                            auto* PopValue{ Pool[ i ] };
                            Pool[ i ] = nullptr;
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
                        auto* PopValue{ Pool[ i ].exchange( nullptr ) };
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
                    auto* PopValue{ Pool[ i ] };
                    Pool[ i ] = nullptr;
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
        template< typename... TArgs >
        constexpr SKL_FORCEINLINE static T *Allocate( TArgs... Args ) noexcept
        {
            return AllocateImpl( std::forward< TArgs >( Args )... );
        }

        //! Allocate new T
        constexpr SKL_FORCEINLINE static T *Allocate( ) noexcept
        {
            return AllocateImpl();
        }

        //! Zero all pool items [not thread safe]
        constexpr static void ZeroAllMemory( ) noexcept
        {
            for( size_t i = 0; i < PoolSize; i++ )
            {
                memset( Pool[ i ], 0, sizeof( T ) );
            }
        }

        //! Get GUID of this Pool instance
        SKL_FORCEINLINE constexpr static size_t GetPoolId( ) noexcept
        {
            return reinterpret_cast< size_t >( &PoolTraits::MyType::Preallocate );
        }

#if defined(SKL_MEMORY_STATISTICS) 
        SKL_FORCEINLINE constexpr static size_t GetTotalDeallocations( ) noexcept
        {
            return TotalDeallocations.load( std::memory_order_acquire );
        }

        SKL_FORCEINLINE constexpr static size_t GetTotalAllocations( ) noexcept
        {
            return TotalAllocations.load( std::memory_order_acquire );
        }

        SKL_FORCEINLINE constexpr static size_t GetTotalOSDeallocations( ) noexcept
        {
            return TotalOSDeallocations.load( std::memory_order_acquire );
        }

        SKL_FORCEINLINE constexpr static size_t GetTotalOSAllocations( ) noexcept
        {
            return TotalOSAllocations.load( std::memory_order_acquire );
        }
#endif

        //Deallocate T
        constexpr static void Deallocate( T *Obj ) noexcept
        {
            static_assert( std::is_nothrow_destructible_v< T > || false == PoolTraits::bPerformDestruction || false == PoolTraits::bPerformConstruction, "ObjectPool::Deallocate() T must be nothrow destructible" );
            SKL_ASSERT( ((uintptr_t)Obj) % PoolTraits::Alignment == 0 );
            if( ((uintptr_t)Obj) % PoolTraits::Alignment != 0 )
            {
                SKL_ASSERT( ((uintptr_t)Obj) % PoolTraits::Alignment == 0 );
            }

            if constexpr( PoolTraits::bPerformDestruction && PoolTraits::bPerformConstruction )
            {
                //Call destructor manually
                GDestructNothrow( Obj );
            }

            void* PrevVal;

            if constexpr( PoolTraits::bUseSpinLock )
            {
                { //Critical section
                    SpinLockScopeGuard Guard { SpinLock };

                    const auto InsPos { TailPosition++ };

                    PrevVal                                 = Pool[ InsPos & PoolTraits::MyPoolMask ];
                    Pool[ InsPos & PoolTraits::MyPoolMask ] = reinterpret_cast< void* >( Obj );
                }
            }
            else
            {
                const auto InsPos { TailPosition.fetch_add( 1, std::memory_order_acq_rel) };
                PrevVal = Pool[ ( InsPos - 1 ) & PoolTraits::MyPoolMask ].exchange( reinterpret_cast< void* >( Obj ) );
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

        constexpr static T* Debug_ProbeAt( uint64_t InIndex ) noexcept
        {
            if constexpr ( false == PoolTraits::bNoSync && false == PoolTraits::bUseSpinLock )
            {
                return reinterpret_cast<T*>( Pool[ InIndex & PoolTraits::MyPoolMask ].load( std::memory_order_acquire ) );
            }
            else
            {
                return reinterpret_cast<T*>( Pool[ InIndex & PoolTraits::MyPoolMask ] );
            }
        }

    private:
        template< typename... TArgs >
        _NODISCARD SKL_FORCEINLINE constexpr static T *AllocateImpl( TArgs... Args ) noexcept
        {
            T *Allocated;

            if constexpr( PoolTraits::bUseSpinLock )
            {
                { //Critical section
                    SpinLockScopeGuard Guard { SpinLock };

                    const uint64_t PopPos { HeadPosition++ };

                    Allocated                               = reinterpret_cast< T * >( Pool[ PopPos & PoolTraits::MyPoolMask ] );
                    Pool[ PopPos & PoolTraits::MyPoolMask ] = nullptr;
                }
            }
            else
            {
                const auto PopPos = { HeadPosition.fetch_add( 1, std::memory_order_acq_rel ) };
                Allocated = reinterpret_cast<T*>( Pool[ ( PopPos - 1 ) & PoolTraits::MyPoolMask ].exchange( nullptr ) );
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

            SKL_ASSERT( ((uintptr_t)Allocated) % PoolTraits::Alignment == 0 );

            return Allocated;
        }

        static inline SKL_CACHE_ALIGNED PoolTraits::TPoolHead     HeadPosition     { 0 };
        static inline SKL_CACHE_ALIGNED PoolTraits::TPoolTail     TailPosition     { 0 };
        static inline SKL_CACHE_ALIGNED PoolTraits::TPoolPtr      Pool[ PoolSize ] {};
        static inline SKL_CACHE_ALIGNED PoolTraits::TPoolSpinLock SpinLock         {};

#if defined(SKL_MEMORY_STATISTICS) 
        static SKL_CACHE_ALIGNED std::atomic<size_t> TotalAllocations;
        static SKL_CACHE_ALIGNED std::atomic<size_t> TotalDeallocations;
        static SKL_CACHE_ALIGNED std::atomic<size_t> TotalOSAllocations;
        static SKL_CACHE_ALIGNED std::atomic<size_t> TotalOSDeallocations;
#endif
    };

#if defined(SKL_MEMORY_STATISTICS) 
    template< typename T, size_t PoolSize, bool bNoSync, bool bUseSpinLock, bool bPerformConstruction, bool bPerformDestruction, size_t TAlignment >
    inline SKL_CACHE_ALIGNED std::atomic< size_t > ObjectPool< T, PoolSize, bNoSync, bUseSpinLock, bPerformConstruction, bPerformDestruction, TAlignment >::TotalAllocations;

    template< typename T, size_t PoolSize, bool bNoSync, bool bUseSpinLock, bool bPerformConstruction, bool bPerformDestruction, size_t TAlignment >
    inline SKL_CACHE_ALIGNED std::atomic< size_t > ObjectPool< T, PoolSize, bNoSync, bUseSpinLock, bPerformConstruction, bPerformDestruction, TAlignment >::TotalDeallocations;

    template< typename T, size_t PoolSize, bool bNoSync, bool bUseSpinLock, bool bPerformConstruction, bool bPerformDestruction, size_t TAlignment >
    inline SKL_CACHE_ALIGNED std::atomic< size_t > ObjectPool< T, PoolSize, bNoSync, bUseSpinLock, bPerformConstruction, bPerformDestruction, TAlignment >::TotalOSAllocations;

    template< typename T, size_t PoolSize, bool bNoSync, bool bUseSpinLock, bool bPerformConstruction, bool bPerformDestruction, size_t TAlignment >
    inline SKL_CACHE_ALIGNED std::atomic< size_t > ObjectPool< T, PoolSize, bNoSync, bUseSpinLock, bPerformConstruction, bPerformDestruction, TAlignment >::TotalOSDeallocations;
#endif
} // namespace SKL