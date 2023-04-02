//!
//! \file LocalMemoryManager.h
//! 
//! \brief Local memory allocation solution
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    struct LocalMemoryManagerProfilingDummy{};

    enum class ELocalMemoryManagerSourceType
    {
          Pool1
        , Pool2
        , Pool3
        , Pool4
        , Pool5
        , Pool6

        , OS
    };

    struct LocalMemoryManagerProfiling
    {
        using TimingValue = KPIValueAveragePoint<false>;
        static constexpr size_t PoolsCount = 6;

        LocalMemoryManagerProfiling() noexcept
        {
            ( void )memset( PoolAllocTime, 0, sizeof( TimingValue ) * PoolsCount );
            ( void )memset( PoolAllocCounters, 0, sizeof( uint64_t ) * PoolsCount );
        }

        SKL_FORCEINLINE void BeginTiming() noexcept 
        {
            TimingUtil.Begin();
        }

        template<ELocalMemoryManagerSourceType PoolIndex>
        SKL_FORCEINLINE void CalculateAndSaveTiming() noexcept
        {
            PoolAllocTime[static_cast<int32_t>( PoolIndex )].SetValue( TimingUtil.GetElapsedSeconds() );
        }
        
        template<ELocalMemoryManagerSourceType PoolIndex>
        SKL_FORCEINLINE SKL_NODISCARD double GetAllocTiming() const noexcept
        {
            return PoolAllocTime[static_cast<int32_t>( PoolIndex )].GetValue();
        }

        template<ELocalMemoryManagerSourceType PoolIndex>
        SKL_FORCEINLINE SKL_NODISCARD uint64_t GetAllocationForPool() const noexcept
        {
            return PoolAllocCounters[static_cast<int32_t>( PoolIndex )];
        }

        template<ELocalMemoryManagerSourceType PoolIndex>
        SKL_FORCEINLINE void IncrementAllocationForPool() noexcept
        {
            ( void )++PoolAllocCounters[static_cast<int32_t>( PoolIndex )];
        }

    private:
        KPITimeValue TimingUtil{};

        TimingValue PoolAllocTime[PoolsCount];     // Average alloc time for each pool
        uint64_t    PoolAllocCounters[PoolsCount]; // Total allocations count for each pool
    };

    template<typename TStaticConfig>
    class LocalMemoryManager
    {
    public:
        using StaticConfig     = TStaticConfig;
        using TStatisticsValue = std::conditional_t<StaticConfig::bIsThreadSafe, std::atomic_uint64_t, uint64_t>;

        static constexpr size_t InternalAlignment    = StaticConfig::bIsThreadSafe ? SKL_CACHE_LINE_SIZE : SKL_ALIGNMENT;
        static constexpr size_t MemoryBlockAlignment = StaticConfig::bAlignAllMemoryBlocksToTheCacheLine ? SKL_CACHE_LINE_SIZE : SKL_ALIGNMENT;
        static constexpr bool   EnableProfiling      = static_cast<uint16_t>( StaticConfig::ProfilingFlags ) != 0;

        template<size_t TBlockSize
               , size_t TBlockCount
               , bool bThreadSafe             = StaticConfig::bIsThreadSafe
               , bool bUseSpinLock_Or_Atomics = StaticConfig::bUseSpinLock_Or_Atomics
               , size_t Alignment             = MemoryBlockAlignment> 
        struct MemoryPool
        {
            static constexpr size_t BlockSize  = TBlockSize;
            static constexpr size_t BlockCount = TBlockCount;
            using TMemoryBlock                 = MemoryBlock<BlockSize>;
            using TObjectPool                  = LocalObjectPool<TMemoryBlock, BlockCount, false == bThreadSafe, bUseSpinLock_Or_Atomics, false, false, Alignment>;    
            
            TObjectPool Pool{};  
        };        

        struct AllocResult
        {
            void*  MemoryBlock    { nullptr };
            size_t MemoryBlockSize;

            SKL_FORCEINLINE const bool IsValid() const noexcept { return nullptr != MemoryBlock; }
            SKL_FORCEINLINE void Zero() noexcept { memset( MemoryBlock, 0, MemoryBlockSize ); }
        };
    
        static consteval bool CHasProfilingFlag( ELocalMemoryManagerProfilingFlags Flag ) noexcept
        {
            return ( static_cast<uint16_t>( Flag ) & static_cast<uint16_t>( StaticConfig::ProfilingFlags ) ) == static_cast<uint16_t>( Flag );
        }

        // Memory Pools
        using TPool1 = MemoryPool<TStaticConfig::Pool1_BlockSize, TStaticConfig::Pool1_BlockCount>;
        using TPool2 = MemoryPool<TStaticConfig::Pool2_BlockSize, TStaticConfig::Pool2_BlockCount>;
        using TPool3 = MemoryPool<TStaticConfig::Pool3_BlockSize, TStaticConfig::Pool3_BlockCount>;
        using TPool4 = MemoryPool<TStaticConfig::Pool4_BlockSize, TStaticConfig::Pool4_BlockCount>;
        using TPool5 = MemoryPool<TStaticConfig::Pool5_BlockSize, TStaticConfig::Pool5_BlockCount>;
        using TPool6 = MemoryPool<TStaticConfig::Pool6_BlockSize, TStaticConfig::Pool6_BlockCount>;
        using TProfilingData = std::conditional_t<EnableProfiling, LocalMemoryManagerProfiling, LocalMemoryManagerProfilingDummy>;

        //! Preallocate all pools
        RStatus Preallocate() noexcept
        {
            if( RSuccess != Pool1.Pool.Preallocate() ) SKL_UNLIKELY
            {
                SKLL_ERR_FMT( "LocalMemoryManager[%ws]::Preallocate() -> Failed to Preallocate Pool1", Name );
                return RFail;
            }
            if( RSuccess != Pool2.Pool.Preallocate() ) SKL_UNLIKELY
            {
                SKLL_ERR_FMT( "LocalMemoryManager[%ws]::Preallocate() -> Failed to Preallocate Pool2", Name );
                return RFail;
            }
            if( RSuccess != Pool3.Pool.Preallocate() ) SKL_UNLIKELY
            {
                SKLL_ERR_FMT( "LocalMemoryManager[%ws]::Preallocate() -> Failed to Preallocate Pool3", Name );
                return RFail;
            }
            if( RSuccess != Pool4.Pool.Preallocate() ) SKL_UNLIKELY
            {
                SKLL_ERR_FMT( "LocalMemoryManager[%ws]::Preallocate() -> Failed to Preallocate Pool4", Name );
                return RFail;
            }
            if( RSuccess != Pool5.Pool.Preallocate() ) SKL_UNLIKELY
            {
                SKLL_ERR_FMT( "LocalMemoryManager[%ws]::Preallocate() -> Failed to Preallocate Pool5", Name );
                return RFail;
            }
            if( RSuccess != Pool6.Pool.Preallocate() ) SKL_UNLIKELY
            {
                SKLL_ERR_FMT( "LocalMemoryManager[%ws]::Preallocate() -> Failed to Preallocate Pool6", Name );
                return RFail;
            }

            SKLL_INF_FMT( "LocalMemoryManager[%ws] ALL POOLS PREALLOCATED!", Name );

            return RSuccess;
        }

        void FreeAllPools() noexcept
        {
            Pool1.Pool.FreePool();
            Pool2.Pool.FreePool();
            Pool3.Pool.FreePool();
            Pool4.Pool.FreePool();
            Pool5.Pool.FreePool();
            Pool6.Pool.FreePool();
        }

        //! Zero memory all pools, this will force the OS to have the all pages ready in memory (hot)
        void ZeroAllMemory( ) noexcept
        {
            Pool1.Pool.ZeroAllMemory();
            Pool2.Pool.ZeroAllMemory();
            Pool3.Pool.ZeroAllMemory();
            Pool4.Pool.ZeroAllMemory();
            Pool5.Pool.ZeroAllMemory();
            Pool6.Pool.ZeroAllMemory();
        }
        
        //! Allocate new memory block with the size known at compile time
        template<size_t AllocateSize>
        SKL_NODISCARD AllocResult Allocate() noexcept
        {
            static_assert( 0 == SKL_GUARD_ALLOC_SIZE_ON || AllocateSize < StaticConfig::MaxAllocationSize, "LocalMemoryManager Cannot alloc this much memory at once!" );

            AllocResult Result{};
            
            if constexpr( EnableProfiling )
            {
                ProfilingData.BeginTiming();
            }
            
            #if defined(SKL_MEM_MANAGER_DECAY_TO_GLOBAL)

            Result.MemoryBlockSize = AllocateSize;
            Result.MemoryBlock     = SKL_MALLOC_ALIGNED( AllocateSize, MemoryBlockAlignment );
            
            if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Time_PoolAllocations ) )
                ProfilingData.template CalculateAndSaveTiming<ELocalMemoryManagerSourceType::OS>();
                
            if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Count_PoolAllocations ) )
                ProfilingData.template IncrementAllocationForPool<ELocalMemoryManagerSourceType::OS>();
                
            #else

            if constexpr( AllocateSize <= TStaticConfig::Pool1_BlockSize )
            {
                Result.MemoryBlockSize = TStaticConfig::Pool1_BlockSize;
                Result.MemoryBlock     = reinterpret_cast<void*>( Pool1.Pool.Allocate() );

                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Time_PoolAllocations ) )
                    ProfilingData.template CalculateAndSaveTiming<ELocalMemoryManagerSourceType::Pool1>();
                    
                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Count_PoolAllocations ) )
                    ProfilingData.template IncrementAllocationForPool<ELocalMemoryManagerSourceType::Pool1>();
            }
            else if constexpr( AllocateSize <= TStaticConfig::Pool2_BlockSize )
            {
                Result.MemoryBlockSize = TStaticConfig::Pool2_BlockSize;
                Result.MemoryBlock     = reinterpret_cast<void*>( Pool2.Pool.Allocate() );

                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Time_PoolAllocations ) )
                    ProfilingData.template CalculateAndSaveTiming<ELocalMemoryManagerSourceType::Pool2>();
                    
                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Count_PoolAllocations ) )
                    ProfilingData.template IncrementAllocationForPool<ELocalMemoryManagerSourceType::Pool2>();
            }
            else if constexpr( AllocateSize <= TStaticConfig::Pool3_BlockSize )
            {
                Result.MemoryBlockSize = TStaticConfig::Pool3_BlockSize;
                Result.MemoryBlock     = reinterpret_cast<void*>( Pool3.Pool.Allocate() );

                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Time_PoolAllocations ) )
                    ProfilingData.template CalculateAndSaveTiming<ELocalMemoryManagerSourceType::Pool3>();
                    
                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Count_PoolAllocations ) )
                    ProfilingData.template IncrementAllocationForPool<ELocalMemoryManagerSourceType::Pool3>();
            }
            else if constexpr( AllocateSize <= TStaticConfig::Pool4_BlockSize )
            {
                Result.MemoryBlockSize = TStaticConfig::Pool4_BlockSize;
                Result.MemoryBlock     = reinterpret_cast<void*>( Pool4.Pool.Allocate() );

                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Time_PoolAllocations ) )
                    ProfilingData.template CalculateAndSaveTiming<ELocalMemoryManagerSourceType::Pool4>();
                    
                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Count_PoolAllocations ) )
                    ProfilingData.template IncrementAllocationForPool<ELocalMemoryManagerSourceType::Pool4>();
            }
            else if constexpr( AllocateSize <= TStaticConfig::Pool5_BlockSize )
            {
                Result.MemoryBlockSize = TStaticConfig::Pool5_BlockSize;
                Result.MemoryBlock     = reinterpret_cast<void*>( Pool5.Pool.Allocate() );

                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Time_PoolAllocations ) )
                    ProfilingData.template CalculateAndSaveTiming<ELocalMemoryManagerSourceType::Pool5>();
                    
                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Count_PoolAllocations ) )
                    ProfilingData.template IncrementAllocationForPool<ELocalMemoryManagerSourceType::Pool5>();
            }
            else if constexpr( AllocateSize <= TStaticConfig::Pool6_BlockSize )
            {
                Result.MemoryBlockSize = TStaticConfig::Pool6_BlockSize;
                Result.MemoryBlock     = reinterpret_cast<void*>( Pool6.Pool.Allocate() );

                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Time_PoolAllocations ) )
                    ProfilingData.template CalculateAndSaveTiming<ELocalMemoryManagerSourceType::Pool6>();
                    
                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Count_PoolAllocations ) )
                    ProfilingData.template IncrementAllocationForPool<ELocalMemoryManagerSourceType::Pool6>();
            }
            else
            {
                Result.MemoryBlockSize = AllocateSize;
                Result.MemoryBlock     = SKL_MALLOC_ALIGNED( AllocateSize, MemoryBlockAlignment );

                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Time_PoolAllocations ) )
                    ProfilingData.template CalculateAndSaveTiming<ELocalMemoryManagerSourceType::OS>();
                    
                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Count_PoolAllocations ) )
                    ProfilingData.template IncrementAllocationForPool<ELocalMemoryManagerSourceType::OS>();
            }

            SKL_ASSERT( 0 == ( ( uintptr_t )Result.MemoryBlock ) % MemoryBlockAlignment );
#if defined(SKL_DEBUG_MEMORY_ALLOCATORS)
            {
                if constexpr( true == StaticConfig::bIsThreadSafe )
                {
                    std::lock_guard<std::mutex> Guard{ AllocationsMutex };
                    if( auto It{ Allocations.find( Result.MemoryBlock ) }; It == Allocations.end() )
                    {
                        Allocations.insert( { Result.MemoryBlock, 0 } );
                    }
                    else
                    {
                        SKL_BREAK();
                    }
                }
                else
                {
                    if( auto It{ Allocations.find( Result.MemoryBlock ) }; It == Allocations.end() )
                    {
                        Allocations.insert( { Result.MemoryBlock, 0 } );
                    }
                    else
                    {
                        SKL_BREAK();
                    }
                }
            }
#endif

            SKL_IFMEMORYSTATS( ++TotalAllocations );

            #endif
            return Result;
        }

        //! Allocate new memory block with the size known at run time
        SKL_NODISCARD AllocResult Allocate( size_t AllocateSize ) noexcept
        {
            AllocResult Result { };

            if( SKL_GUARD_ALLOC_SIZE_ON && ( AllocateSize > StaticConfig::MaxAllocationSize ) ) SKL_UNLIKELY
            {
                SKLL_ERR_FMT( "LocalMemoryManager[%ws]::Allocate( AllocateSize ) Cannot alloc more than %llu. Attempted %llu!", Name, StaticConfig::MaxAllocationSize, AllocateSize );
                return Result;
            }
         
            if constexpr( EnableProfiling )
            {
                ProfilingData.BeginTiming();
            }
            
            #if defined(SKL_MEM_MANAGER_DECAY_TO_GLOBAL)

            Result.MemoryBlockSize = AllocateSize;
            Result.MemoryBlock     = SKL_MALLOC_ALIGNED( AllocateSize, MemoryBlockAlignment );
            
            if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Time_PoolAllocations ) )
                ProfilingData.template CalculateAndSaveTiming<ELocalMemoryManagerSourceType::OS>();
                
            if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Count_PoolAllocations ) )
                ProfilingData.template IncrementAllocationForPool<ELocalMemoryManagerSourceType::OS>();
                
            #else

            if ( AllocateSize <= TStaticConfig::Pool1_BlockSize )
            {
                Result.MemoryBlockSize = TStaticConfig::Pool1_BlockSize;
                Result.MemoryBlock     = reinterpret_cast<void*>( Pool1.Pool.Allocate() );

                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Time_PoolAllocations ) )
                    ProfilingData.template CalculateAndSaveTiming<ELocalMemoryManagerSourceType::Pool1>();
                    
                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Count_PoolAllocations ) )
                    ProfilingData.template IncrementAllocationForPool<ELocalMemoryManagerSourceType::Pool1>();
            }
            else if ( AllocateSize <= TStaticConfig::Pool2_BlockSize )
            {
                Result.MemoryBlockSize = TStaticConfig::Pool2_BlockSize;
                Result.MemoryBlock     = reinterpret_cast<void*>( Pool2.Pool.Allocate() );

                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Time_PoolAllocations ) )
                    ProfilingData.template CalculateAndSaveTiming<ELocalMemoryManagerSourceType::Pool2>();
                    
                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Count_PoolAllocations ) )
                    ProfilingData.template IncrementAllocationForPool<ELocalMemoryManagerSourceType::Pool2>();
            }
            else if ( AllocateSize <= TStaticConfig::Pool3_BlockSize )
            {
                Result.MemoryBlockSize = TStaticConfig::Pool3_BlockSize;
                Result.MemoryBlock     = reinterpret_cast<void*>( Pool3.Pool.Allocate() );

                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Time_PoolAllocations ) )
                    ProfilingData.template CalculateAndSaveTiming<ELocalMemoryManagerSourceType::Pool3>();
                    
                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Count_PoolAllocations ) )
                    ProfilingData.template IncrementAllocationForPool<ELocalMemoryManagerSourceType::Pool3>();
            }
            else if ( AllocateSize <= TStaticConfig::Pool4_BlockSize )
            {
                Result.MemoryBlockSize = TStaticConfig::Pool4_BlockSize;
                Result.MemoryBlock     = reinterpret_cast<void*>( Pool4.Pool.Allocate() );

                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Time_PoolAllocations ) )
                    ProfilingData.template CalculateAndSaveTiming<ELocalMemoryManagerSourceType::Pool4>();
                    
                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Count_PoolAllocations ) )
                    ProfilingData.template IncrementAllocationForPool<ELocalMemoryManagerSourceType::Pool4>();
            }
            else if ( AllocateSize <= TStaticConfig::Pool5_BlockSize )
            {
                Result.MemoryBlockSize = TStaticConfig::Pool5_BlockSize;
                Result.MemoryBlock     = reinterpret_cast<void*>( Pool5.Pool.Allocate() );

                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Time_PoolAllocations ) )
                    ProfilingData.template CalculateAndSaveTiming<ELocalMemoryManagerSourceType::Pool5>();
                    
                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Count_PoolAllocations ) )
                    ProfilingData.template IncrementAllocationForPool<ELocalMemoryManagerSourceType::Pool5>();
            }
            else if ( AllocateSize <= TStaticConfig::Pool6_BlockSize )
            {
                Result.MemoryBlockSize = TStaticConfig::Pool6_BlockSize;
                Result.MemoryBlock     = reinterpret_cast<void*>( Pool6.Pool.Allocate() );

                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Time_PoolAllocations ) )
                    ProfilingData.template CalculateAndSaveTiming<ELocalMemoryManagerSourceType::Pool6>();
                    
                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Count_PoolAllocations ) )
                    ProfilingData.template IncrementAllocationForPool<ELocalMemoryManagerSourceType::Pool6>();
            }
            else
            {
                Result.MemoryBlockSize = AllocateSize;
                Result.MemoryBlock     = SKL_MALLOC_ALIGNED( AllocateSize, MemoryBlockAlignment );

                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Time_PoolAllocations ) )
                    ProfilingData.template CalculateAndSaveTiming<ELocalMemoryManagerSourceType::OS>();
                    
                if constexpr( CHasProfilingFlag( ELocalMemoryManagerProfilingFlags::Count_PoolAllocations ) )
                    ProfilingData.template IncrementAllocationForPool<ELocalMemoryManagerSourceType::OS>();
            }

            SKL_ASSERT( 0 == ( ( uintptr_t )Result.MemoryBlock ) % MemoryBlockAlignment );

#if defined(SKL_DEBUG_MEMORY_ALLOCATORS)
            {
                if constexpr( true == StaticConfig::bIsThreadSafe )
                {
                    std::lock_guard<std::mutex> Guard{ AllocationsMutex };
                    if( auto It{ Allocations.find( Result.MemoryBlock ) }; It == Allocations.end() )
                    {
                        Allocations.insert( { Result.MemoryBlock, 0 } );
                    }
                    else
                    {
                        SKL_BREAK();
                    }
                }
                else
                {
                    if( auto It{ Allocations.find( Result.MemoryBlock ) }; It == Allocations.end() )
                    {
                        Allocations.insert( { Result.MemoryBlock, 0 } );
                    }
                    else
                    {
                        SKL_BREAK();
                    }
                }
            }
#endif

            SKL_IFMEMORYSTATS( ++TotalAllocations );

            #endif

            return Result;
        }
       
        //! Deallocate memory block with the size known at compile time
        template<size_t AllocateSize>
        void Deallocate( void* InPointer ) noexcept 
        {
            SKL_ASSERT( 0 == ( ( uintptr_t )InPointer ) % MemoryBlockAlignment );
#if defined(SKL_DEBUG_MEMORY_ALLOCATORS)
            {
                if constexpr( true == StaticConfig::bIsThreadSafe )
                {
                    std::lock_guard<std::mutex> Guard{ AllocationsMutex };
                    if( auto It{ Allocations.find( InPointer ) }; It != Allocations.end() )
                    {
                        Allocations.erase( It );
                    }
                    else
                    {
                        SKL_BREAK();
                    }
                }
                else
                {
                    if( auto It{ Allocations.find( InPointer ) }; It != Allocations.end() )
                    {
                        Allocations.erase( It );
                    }
                    else
                    {
                        SKL_BREAK();
                    }
                }
            }
#endif

            if constexpr( AllocateSize <= TStaticConfig::Pool1_BlockSize )
            {
                Pool1.Pool.Deallocate( reinterpret_cast<TPool1::TMemoryBlock*>( InPointer ) );
            }
            else if constexpr( AllocateSize <= TStaticConfig::Pool2_BlockSize )
            {
                Pool2.Pool.Deallocate( reinterpret_cast<TPool2::TMemoryBlock*>( InPointer ) );
            }
            else if constexpr( AllocateSize <= TStaticConfig::Pool3_BlockSize )
            {
                Pool3.Pool.Deallocate( reinterpret_cast<TPool3::TMemoryBlock*>( InPointer ) );
            }
            else if constexpr( AllocateSize <= TStaticConfig::Pool4_BlockSize )
            {
                Pool4.Pool.Deallocate( reinterpret_cast<TPool4::TMemoryBlock*>( InPointer ) );
            }
            else if constexpr( AllocateSize <= TStaticConfig::Pool5_BlockSize )
            {
                Pool5.Pool.Deallocate( reinterpret_cast<TPool5::TMemoryBlock*>( InPointer ) );
            }
            else if constexpr( AllocateSize <= TStaticConfig::Pool6_BlockSize )
            {
                Pool6.Pool.Deallocate( reinterpret_cast<TPool6::TMemoryBlock*>( InPointer ) );
            }
            else
            {
                SKL_FREE_SIZE_ALIGNED( InPointer, AllocateSize, MemoryBlockAlignment );
                SKL_IFMEMORYSTATS( ++CustomSizeDeallocations );
            }

            SKL_IFMEMORYSTATS( ++TotalDeallocations );
        }   

        //! Deallocate memory block with the size known at run time
        void Deallocate( void* InPointer, size_t AllocateSize ) noexcept 
        {
            SKL_ASSERT( 0 == ( ( uintptr_t )InPointer ) % MemoryBlockAlignment );
#if defined(SKL_DEBUG_MEMORY_ALLOCATORS)
            {
                if constexpr( true == StaticConfig::bIsThreadSafe )
                {
                    std::lock_guard<std::mutex> Guard{ AllocationsMutex };
                    if( auto It{ Allocations.find( InPointer ) }; It != Allocations.end() )
                    {
                        Allocations.erase( It );
                    }
                    else
                    {
                        SKL_BREAK();
                    }
                }
                else
                {
                    if( auto It{ Allocations.find( InPointer ) }; It != Allocations.end() )
                    {
                        Allocations.erase( It );
                    }
                    else
                    {
                        SKL_BREAK();
                    }
                }
            }
#endif

            if( AllocateSize <= TStaticConfig::Pool1_BlockSize )
            {
                Pool1.Pool.Deallocate( reinterpret_cast<TPool1::TMemoryBlock*>( InPointer ) );
            }
            else if( AllocateSize <= TStaticConfig::Pool2_BlockSize )
            {
                Pool2.Pool.Deallocate( reinterpret_cast<TPool2::TMemoryBlock*>( InPointer ) );
            }
            else if( AllocateSize <= TStaticConfig::Pool3_BlockSize )
            {
                Pool3.Pool.Deallocate( reinterpret_cast<TPool3::TMemoryBlock*>( InPointer ) );
            }
            else if( AllocateSize <= TStaticConfig::Pool4_BlockSize )
            {
                Pool4.Pool.Deallocate( reinterpret_cast<TPool4::TMemoryBlock*>( InPointer ) );
            }
            else if( AllocateSize <= TStaticConfig::Pool5_BlockSize )
            {
                Pool5.Pool.Deallocate( reinterpret_cast<TPool5::TMemoryBlock*>( InPointer ) );
            }
            else if( AllocateSize <= TStaticConfig::Pool6_BlockSize )
            {
                Pool6.Pool.Deallocate( reinterpret_cast<TPool6::TMemoryBlock*>( InPointer ) );
            }
            else
            {
                SKL_FREE_SIZE_ALIGNED( InPointer, AllocateSize, MemoryBlockAlignment );
                SKL_IFMEMORYSTATS( ++CustomSizeDeallocations );
            }

            SKL_IFMEMORYSTATS( ++TotalDeallocations );
        }
    
        //! Deallocate memory block with the size known at run time
        SKL_FORCEINLINE void Deallocate( AllocResult& InAllocResult ) noexcept
        {
            SKL_ASSERT( true == InAllocResult.IsValid() );

            Deallocate( InAllocResult.MemoryBlock, static_cast<size_t>( InAllocResult.MemoryBlockSize ) );

            InAllocResult.MemoryBlock = nullptr;
        }

        //! Deallocate memory block with the size known at run time
        SKL_FORCEINLINE void Deallocate( AllocResult* InAllocResult ) noexcept
        {
            SKL_ASSERT( true == InAllocResult->IsValid() );

            Deallocate( InAllocResult->MemoryBlock, static_cast<size_t>( InAllocResult->MemoryBlockSize ) );

            InAllocResult->MemoryBlock = nullptr;
        }

        //! Log manager statistics
        void LogStatistics() noexcept
        {
#if defined(SKL_MEMORY_STATISTICS)
            SKLL_INF_FMT( "LocalMemoryManager[%ws] ###############################################################", Name );
            SKLL_INF_FMT( "Pool1:\n\t\tAllocations:%lld\n\t\tDeallocations:%lld\n\t\tOSAllocations:%lld\n\t\tOSDeallocations:%lld",
                    Pool1.Pool.GetTotalAllocations( ),
                    Pool1.Pool.GetTotalDeallocations( ),
                    Pool1.Pool.GetTotalOSAllocations( ),
                    Pool1.Pool.GetTotalOSDeallocations( ) );
            SKLL_INF_FMT( "Pool2:\n\t\tAllocations:%lld\n\t\tDeallocations:%lld\n\t\tOSAllocations:%lld\n\t\tOSDeallocations:%lld",
                    Pool2.Pool.GetTotalAllocations( ),
                    Pool2.Pool.GetTotalDeallocations( ),
                    Pool2.Pool.GetTotalOSAllocations( ),
                    Pool2.Pool.GetTotalOSDeallocations( ) );
            SKLL_INF_FMT( "Pool3:\n\t\tAllocations:%lld\n\t\tDeallocations:%lld\n\t\tOSAllocations:%lld\n\t\tOSDeallocations:%lld",
                    Pool3.Pool.GetTotalAllocations( ),
                    Pool3.Pool.GetTotalDeallocations( ),
                    Pool3.Pool.GetTotalOSAllocations( ),
                    Pool3.Pool.GetTotalOSDeallocations( ) );
            SKLL_INF_FMT( "Pool4:\n\t\tAllocations:%lld\n\t\tDeallocations:%lld\n\t\tOSAllocations:%lld\n\t\tOSDeallocations:%lld",  
                    Pool4.Pool.GetTotalAllocations( ), 
                    Pool4.Pool.GetTotalDeallocations( ), 
                    Pool4.Pool.GetTotalOSAllocations( ), 
                    Pool4.Pool.GetTotalOSDeallocations( ) );                                                                
            SKLL_INF_FMT( "Pool5:\n\t\tAllocations:%lld\n\t\tDeallocations:%lld\n\t\tOSAllocations:%lld\n\t\tOSDeallocations:%lld",
                    Pool5.Pool.GetTotalAllocations( ),
                    Pool5.Pool.GetTotalDeallocations( ),
                    Pool5.Pool.GetTotalOSAllocations( ),
                    Pool5.Pool.GetTotalOSDeallocations( ) );
            SKLL_INF_FMT( "Pool6:\n\t\tAllocations:%lld\n\t\tDeallocations:%lld\n\t\tOSAllocations:%lld\n\t\tOSDeallocations:%lld",
                    Pool6.Pool.GetTotalAllocations( ),
                    Pool6.Pool.GetTotalDeallocations( ),
                    Pool6.Pool.GetTotalOSAllocations( ),
                    Pool6.Pool.GetTotalOSDeallocations( ) );
            SKLL_INF_FMT( "CustomSize(OS Blocks):\n\t\tAllocations:%lld\n\t\tDeallocations:%lld",
                    CustomSizeAllocations.load( ),
                    CustomSizeDeallocations.load( ) );
            SKLL_INF_FMT( "GAllocate:\n\t\tAllocations:%lld\n\t\tDeallocations:%lld",
                    TotalAllocations.load( ),
                    TotalDeallocations.load( ) );
            SKLL_INF_FMT( "Total Allocation:%lld\n\tTotal Deallocations:%lld\n\tTotal OSAllocations:%lld\n\tTotal OSDeallocations:%lld",
                    Pool1.Pool.GetTotalAllocations( ) 
                        + Pool1.Pool.GetTotalAllocations( ) 
                        + Pool2.Pool.GetTotalAllocations( ) 
                        + Pool3.Pool.GetTotalAllocations( ) 
                        + Pool4.Pool.GetTotalAllocations( ) 
                        + Pool5.Pool.GetTotalAllocations( ) 
                        + CustomSizeAllocations.load( ),

                     Pool1.Pool.GetTotalDeallocations( ) 
                        + Pool2.Pool.GetTotalDeallocations( ) 
                        + Pool3.Pool.GetTotalDeallocations( ) 
                        + Pool4.Pool.GetTotalDeallocations( ) 
                        + Pool5.Pool.GetTotalDeallocations( ) 
                        + CustomSizeDeallocations.load( ),

                     Pool1.Pool.GetTotalOSAllocations( ) 
                        + Pool1.Pool.GetTotalOSAllocations( ) 
                        + Pool2.Pool.GetTotalOSAllocations( ) 
                        + Pool3.Pool.GetTotalOSAllocations( ) 
                        + Pool4.Pool.GetTotalOSAllocations( ) 
                        + Pool5.Pool.GetTotalOSAllocations( ),

                     Pool1.Pool.GetTotalOSDeallocations( ) 
                        + Pool1.Pool.GetTotalOSDeallocations( ) 
                        + Pool2.Pool.GetTotalOSDeallocations( ) 
                        + Pool3.Pool.GetTotalOSDeallocations( ) 
                        + Pool4.Pool.GetTotalOSDeallocations( ) 
                        + Pool5.Pool.GetTotalOSDeallocations( )
                    );
            SKLL_INF_FMT( "LocalMemoryManager[%ws] ###############################################################", Name );
#else
            SKLL_INF_FMT( "LocalMemoryManager[%ws]::LogStatistics()\n\t\tTried to log memory statistics, but the LocalMemoryManager has the statistics turned off!", Name );
#endif
        }
        
        TPool1         Pool1{};
        TPool2         Pool2{};
        TPool3         Pool3{};
        TPool4         Pool4{};
        TPool5         Pool5{};
        TPool6         Pool6{};
        TProfilingData ProfilingData;
        const wchar_t* Name { TStaticConfig::PoolName };
        
        // Stats variables
        SKL_IFMEMORYSTATS( alignas( InternalAlignment ) TStatisticsValue CustomSizeAllocations  { 0 }; );
        SKL_IFMEMORYSTATS( alignas( InternalAlignment ) TStatisticsValue CustomSizeDeallocations{ 0 }; );
        SKL_IFMEMORYSTATS( alignas( InternalAlignment ) TStatisticsValue TotalAllocations       { 0 }; );
        SKL_IFMEMORYSTATS( alignas( InternalAlignment ) TStatisticsValue TotalDeallocations     { 0 }; );

#if defined(SKL_DEBUG_MEMORY_ALLOCATORS)
        std::mutex                         AllocationsMutex;
        std::unordered_map<void*, int32_t> Allocations;
#endif
    };
}