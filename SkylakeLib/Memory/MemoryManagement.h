//!
//! \file MemoryManagement.h
//! 
//! \brief Global memory allocation solution
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    template<size_t BlockSize>
    using MemoryBlock = std::array<uint8_t, BlockSize>;

    class MemoryManager final
    {
    public:
        template<size_t TBlockSize, size_t TBlockCount> 
        struct MemoryPool
        {
            static constexpr size_t BlockSize  = TBlockSize;
            static constexpr size_t BlockCount = TBlockCount;
            using TMemoryBlock                 = MemoryBlock<BlockSize>;
            using TObjectPool                  = ObjectPool<TMemoryBlock, BlockCount, false, CMemoryManager_UseSpinLock_Or_Atomics, false, false, CMemoryManager_Alignment>;      
        };        

        struct AllocResult
        {
            void*  MemoryBlock    { nullptr };
            size_t MemoryBlockSize;

            SKL_FORCEINLINE const bool IsValid() const noexcept { return nullptr != MemoryBlock; }
            SKL_FORCEINLINE void Zero() noexcept { memset( MemoryBlock, 0, MemoryBlockSize ); }
        };
    
        // Memory Pools
        using Pool1 = MemoryPool<CMemoryManager_Pool1_BlockSize, CMemoryManager_Pool1_BlockCount>;
        using Pool2 = MemoryPool<CMemoryManager_Pool2_BlockSize, CMemoryManager_Pool2_BlockCount>;
        using Pool3 = MemoryPool<CMemoryManager_Pool3_BlockSize, CMemoryManager_Pool3_BlockCount>;
        using Pool4 = MemoryPool<CMemoryManager_Pool4_BlockSize, CMemoryManager_Pool4_BlockCount>;
        using Pool5 = MemoryPool<CMemoryManager_Pool5_BlockSize, CMemoryManager_Pool5_BlockCount>;
        using Pool6 = MemoryPool<CMemoryManager_Pool6_BlockSize, CMemoryManager_Pool6_BlockCount>;
        
        //! 
        //! \brief Preallocate all pools
        //! 
        static RStatus Preallocate() noexcept
        {
            if( RSuccess != Pool1::TObjectPool::Preallocate() )
            {
                SKL_ERR( "MemoryManager::Preallocate() -> Failed to Preallocate Pool1" );
                return RFail;
            }
            if( RSuccess != Pool2::TObjectPool::Preallocate() )
            {
                SKL_ERR( "MemoryManager::Preallocate() -> Failed to Preallocate Pool2" );
                return RFail;
            }
            if( RSuccess != Pool3::TObjectPool::Preallocate() )
            {
                SKL_ERR( "MemoryManager::Preallocate() -> Failed to Preallocate Pool3" );
                return RFail;
            }
            if( RSuccess != Pool4::TObjectPool::Preallocate() )
            {
                SKL_ERR( "MemoryManager::Preallocate() -> Failed to Preallocate Pool4" );
                return RFail;
            }
            if( RSuccess != Pool5::TObjectPool::Preallocate() )
            {
                SKL_ERR( "MemoryManager::Preallocate() -> Failed to Preallocate Pool5" );
                return RFail;
            }
            if( RSuccess != Pool6::TObjectPool::Preallocate() )
            {
                SKL_ERR( "MemoryManager::Preallocate() -> Failed to Preallocate Pool6" );
                return RFail;
            }

            SKL_INF( "MemoryManager ALL POOLS PREALLOCATED!" );

            return RSuccess;
        }

        //! 
        //! \brief Zero memory all pools, this will force the OS to have the all pages ready in memory (hot)
        //! 
        static void ZeroAllMemory( ) noexcept
        {
            Pool1::TObjectPool::ZeroAllMemory( );
            Pool2::TObjectPool::ZeroAllMemory( );
            Pool3::TObjectPool::ZeroAllMemory( );
            Pool4::TObjectPool::ZeroAllMemory( );
            Pool5::TObjectPool::ZeroAllMemory( );
            Pool6::TObjectPool::ZeroAllMemory( );
        }
        
        //! Allocate new memory block with the size known at compile time
        template<size_t AllocateSize>
        static AllocResult Allocate() noexcept
        {
            static_assert( 0 == SKL_GUARD_ALLOC_SIZE_ON || AllocateSize < CMemoryManager_MaxAllocSize, "Cannot alloc this much memory at once!" );

            AllocResult Result { };
         
            if constexpr( AllocateSize <= CMemoryManager_Pool1_BlockSize )
            {
                Result.MemoryBlockSize = CMemoryManager_Pool1_BlockSize;
                Result.MemoryBlock     = reinterpret_cast<void*>( Pool1::TObjectPool::Allocate() );
            }
            else if constexpr( AllocateSize <= CMemoryManager_Pool2_BlockSize )
            {
                Result.MemoryBlockSize = CMemoryManager_Pool2_BlockSize;
                Result.MemoryBlock     = reinterpret_cast<void*>( Pool2::TObjectPool::Allocate() );
            }
            else if constexpr( AllocateSize <= CMemoryManager_Pool3_BlockSize )
            {
                Result.MemoryBlockSize = CMemoryManager_Pool3_BlockSize;
                Result.MemoryBlock     = reinterpret_cast<void*>( Pool3::TObjectPool::Allocate() );
            }
            else if constexpr( AllocateSize <= CMemoryManager_Pool4_BlockSize )
            {
                Result.MemoryBlockSize = CMemoryManager_Pool4_BlockSize;
                Result.MemoryBlock     = reinterpret_cast<void*>( Pool4::TObjectPool::Allocate() );
            }
            else if constexpr( AllocateSize <= CMemoryManager_Pool5_BlockSize )
            {
                Result.MemoryBlockSize = CMemoryManager_Pool5_BlockSize;
                Result.MemoryBlock     = reinterpret_cast<void*>( Pool5::TObjectPool::Allocate() );
            }
            else if constexpr( AllocateSize <= CMemoryManager_Pool6_BlockSize )
            {
                Result.MemoryBlockSize = CMemoryManager_Pool6_BlockSize;
                Result.MemoryBlock     = reinterpret_cast<void*>( Pool6::TObjectPool::Allocate() );
            }
            else
            {
                Result.MemoryBlockSize = AllocateSize;
                Result.MemoryBlock     = SKL_MALLOC_ALIGNED( AllocateSize, CMemoryManager_Alignment );
            }

            SKL_IFMEMORYSTATS( ++TotalAllocations );

            return Result;
        }

        //! Allocate new memory block with the size known at run time
        static AllocResult Allocate( size_t AllocateSize ) noexcept
        {
            AllocResult Result { };

            if( SKL_GUARD_ALLOC_SIZE_ON && ( AllocateSize > CMemoryManager_MaxAllocSize ) ) SKL_UNLIKELY
            {
                SKL_ERR_FMT( "MemorymManager::Allocate( AllocateSize ) Cannot alloc more than %llu. Attempted %llu!", CMemoryManager_MaxAllocSize, AllocateSize );
                return Result;
            }
            
            if( AllocateSize <= CMemoryManager_Pool1_BlockSize )
            {
                Result.MemoryBlockSize = CMemoryManager_Pool1_BlockSize;
                Result.MemoryBlock     = reinterpret_cast<void*>( Pool1::TObjectPool::Allocate() );
            }
            else if( AllocateSize <= CMemoryManager_Pool2_BlockSize )
            {
                Result.MemoryBlockSize = CMemoryManager_Pool2_BlockSize;
                Result.MemoryBlock     = reinterpret_cast<void*>( Pool2::TObjectPool::Allocate() );
            }
            else if( AllocateSize <= CMemoryManager_Pool3_BlockSize )
            {
                Result.MemoryBlockSize = CMemoryManager_Pool3_BlockSize;
                Result.MemoryBlock     = reinterpret_cast<void*>( Pool3::TObjectPool::Allocate() );
            }
            else if( AllocateSize <= CMemoryManager_Pool4_BlockSize )
            {
                Result.MemoryBlockSize = CMemoryManager_Pool4_BlockSize;
                Result.MemoryBlock     = reinterpret_cast<void*>( Pool4::TObjectPool::Allocate() );
            }
            else if( AllocateSize <= CMemoryManager_Pool5_BlockSize )
            {
                Result.MemoryBlockSize = CMemoryManager_Pool5_BlockSize;
                Result.MemoryBlock     = reinterpret_cast<void*>( Pool5::TObjectPool::Allocate() );
            }
            else if( AllocateSize <= CMemoryManager_Pool6_BlockSize )
            {
                Result.MemoryBlockSize = CMemoryManager_Pool6_BlockSize;
                Result.MemoryBlock     = reinterpret_cast<void*>( Pool6::TObjectPool::Allocate() );
            }
            else
            {
                Result.MemoryBlockSize = AllocateSize;
                Result.MemoryBlock     = SKL_MALLOC_ALIGNED( AllocateSize, CMemoryManager_Alignment );

                SKL_IFMEMORYSTATS( ++CustomSizeAllocations );
            }

            SKL_IFMEMORYSTATS( ++TotalAllocations );

            return Result;
        }
       
        //! Deallocate memory block with the size known at compile time
        template<size_t AllocateSize>
        static void Deallocate( void* InPointer ) noexcept 
        {
            if constexpr( AllocateSize <= CMemoryManager_Pool1_BlockSize )
            {
                Pool1::TObjectPool::Deallocate( reinterpret_cast<Pool1::TMemoryBlock*>( InPointer ) );
            }
            else if constexpr( AllocateSize <= CMemoryManager_Pool2_BlockSize )
            {
                Pool2::TObjectPool::Deallocate( reinterpret_cast<Pool2::TMemoryBlock*>( InPointer ) );
            }
            else if constexpr( AllocateSize <= CMemoryManager_Pool3_BlockSize )
            {
                Pool3::TObjectPool::Deallocate( reinterpret_cast<Pool3::TMemoryBlock*>( InPointer ) );
            }
            else if constexpr( AllocateSize <= CMemoryManager_Pool4_BlockSize )
            {
                Pool4::TObjectPool::Deallocate( reinterpret_cast<Pool4::TMemoryBlock*>( InPointer ) );
            }
            else if constexpr( AllocateSize <= CMemoryManager_Pool5_BlockSize )
            {
                Pool5::TObjectPool::Deallocate( reinterpret_cast<Pool5::TMemoryBlock*>( InPointer ) );
            }
            else if constexpr( AllocateSize <= CMemoryManager_Pool6_BlockSize )
            {
                Pool6::TObjectPool::Deallocate( reinterpret_cast<Pool6::TMemoryBlock*>( InPointer ) );
            }
            else
            {
                SKL_FREE_SIZE_ALIGNED( InPointer, AllocateSize, CMemoryManager_Alignment );
                SKL_IFMEMORYSTATS( ++CustomSizeDeallocations );
            }

            SKL_IFMEMORYSTATS( ++TotalDeallocations );
        }

        //! Deallocate memory block with the size known at run time
        static void Deallocate( void* InPointer, size_t AllocateSize ) noexcept 
        {
            if( AllocateSize <= CMemoryManager_Pool1_BlockSize )
            {
                Pool1::TObjectPool::Deallocate( reinterpret_cast<Pool1::TMemoryBlock*>( InPointer ) );
            }
            else if( AllocateSize <= CMemoryManager_Pool2_BlockSize )
            {
                Pool2::TObjectPool::Deallocate( reinterpret_cast<Pool2::TMemoryBlock*>( InPointer ) );
            }
            else if( AllocateSize <= CMemoryManager_Pool3_BlockSize )
            {
                Pool3::TObjectPool::Deallocate( reinterpret_cast<Pool3::TMemoryBlock*>( InPointer ) );
            }
            else if( AllocateSize <= CMemoryManager_Pool4_BlockSize )
            {
                Pool4::TObjectPool::Deallocate( reinterpret_cast<Pool4::TMemoryBlock*>( InPointer ) );
            }
            else if( AllocateSize <= CMemoryManager_Pool5_BlockSize )
            {
                Pool5::TObjectPool::Deallocate( reinterpret_cast<Pool5::TMemoryBlock*>( InPointer ) );
            }
            else if( AllocateSize <= CMemoryManager_Pool6_BlockSize )
            {
                Pool6::TObjectPool::Deallocate( reinterpret_cast<Pool6::TMemoryBlock*>( InPointer ) );
            }
            else
            {
                SKL_FREE_SIZE_ALIGNED( InPointer, AllocateSize, CMemoryManager_Alignment );
                SKL_IFMEMORYSTATS( ++CustomSizeDeallocations );
            }

            SKL_IFMEMORYSTATS( ++TotalDeallocations );
        }
    
        SKL_FORCEINLINE static void Deallocate( AllocResult& InAllocResult ) noexcept
        {
            SKL_ASSERT( true == InAllocResult.IsValid() );

            Deallocate( InAllocResult.MemoryBlock, static_cast<size_t>( InAllocResult.MemoryBlockSize ) );

            InAllocResult.MemoryBlock = nullptr;
        }

        SKL_FORCEINLINE static void Deallocate( AllocResult* InAllocResult ) noexcept
        {
            SKL_ASSERT( true == InAllocResult->IsValid() );

            Deallocate( InAllocResult->MemoryBlock, static_cast<size_t>( InAllocResult->MemoryBlockSize ) );

            InAllocResult->MemoryBlock = nullptr;
        }

        //! 
        //! \brief Zero memory all pools, this will force the OS to have the all pages ready in memory (hot)
        //! 
        static void LogStatistics( ) noexcept
        {
#if defined(SKL_MEMORY_STATISTICS)
            SKL_INF( "MemoryManager ###############################################################" );
            SKL_INF_FMT( "Pool1:\n\t\tAllocations:%lld\n\t\tDeallocations:%lld\n\t\tOSAllocations:%lld\n\t\tOSDeallocations:%lld",
                    Pool1::TObjectPool::GetTotalAllocations( ),
                    Pool1::TObjectPool::GetTotalDeallocations( ),
                    Pool1::TObjectPool::GetTotalOSAllocations( ),
                    Pool1::TObjectPool::GetTotalOSDeallocations( ) );
            SKL_INF_FMT( "Pool2:\n\t\tAllocations:%lld\n\t\tDeallocations:%lld\n\t\tOSAllocations:%lld\n\t\tOSDeallocations:%lld",
                    Pool2::TObjectPool::GetTotalAllocations( ),
                    Pool2::TObjectPool::GetTotalDeallocations( ),
                    Pool2::TObjectPool::GetTotalOSAllocations( ),
                    Pool2::TObjectPool::GetTotalOSDeallocations( ) );
            SKL_INF_FMT( "Pool3:\n\t\tAllocations:%lld\n\t\tDeallocations:%lld\n\t\tOSAllocations:%lld\n\t\tOSDeallocations:%lld",
                    Pool3::TObjectPool::GetTotalAllocations( ),
                    Pool3::TObjectPool::GetTotalDeallocations( ),
                    Pool3::TObjectPool::GetTotalOSAllocations( ),
                    Pool3::TObjectPool::GetTotalOSDeallocations( ) );
            SKL_INF_FMT( "Pool4:\n\t\tAllocations:%lld\n\t\tDeallocations:%lld\n\t\tOSAllocations:%lld\n\t\tOSDeallocations:%lld",  
                    Pool4::TObjectPool::GetTotalAllocations( ), 
                    Pool4::TObjectPool::GetTotalDeallocations( ), 
                    Pool4::TObjectPool::GetTotalOSAllocations( ), 
                    Pool4::TObjectPool::GetTotalOSDeallocations( ) );                                                                
            SKL_INF_FMT( "Pool5:\n\t\tAllocations:%lld\n\t\tDeallocations:%lld\n\t\tOSAllocations:%lld\n\t\tOSDeallocations:%lld",
                    Pool5::TObjectPool::GetTotalAllocations( ),
                    Pool5::TObjectPool::GetTotalDeallocations( ),
                    Pool5::TObjectPool::GetTotalOSAllocations( ),
                    Pool5::TObjectPool::GetTotalOSDeallocations( ) );
            SKL_INF_FMT( "Pool6:\n\t\tAllocations:%lld\n\t\tDeallocations:%lld\n\t\tOSAllocations:%lld\n\t\tOSDeallocations:%lld",
                    Pool6::TObjectPool::GetTotalAllocations( ),
                    Pool6::TObjectPool::GetTotalDeallocations( ),
                    Pool6::TObjectPool::GetTotalOSAllocations( ),
                    Pool6::TObjectPool::GetTotalOSDeallocations( ) );
            SKL_INF_FMT( "CustomSize(OS Blocks):\n\t\tAllocations:%lld\n\t\tDeallocations:%lld",
                    CustomSizeAllocations.load( ),
                    CustomSizeDeallocations.load( ) );
            SKL_INF_FMT( "GAllocate:\n\t\tAllocations:%lld\n\t\tDeallocations:%lld",
                    TotalAllocations.load( ),
                    TotalDeallocations.load( ) );
            SKL_INF_FMT( "Total Allocation:%lld\n\tTotal Deallocations:%lld\n\tTotal OSAllocations:%lld\n\tTotal OSDeallocations:%lld",
                    Pool1::TObjectPool::GetTotalAllocations( ) 
                        + Pool1::TObjectPool::GetTotalAllocations( ) 
                        + Pool2::TObjectPool::GetTotalAllocations( ) 
                        + Pool3::TObjectPool::GetTotalAllocations( ) 
                        + Pool4::TObjectPool::GetTotalAllocations( ) 
                        + Pool5::TObjectPool::GetTotalAllocations( ) 
                        + CustomSizeAllocations.load( ),

                     Pool1::TObjectPool::GetTotalDeallocations( ) 
                        + Pool2::TObjectPool::GetTotalDeallocations( ) 
                        + Pool3::TObjectPool::GetTotalDeallocations( ) 
                        + Pool4::TObjectPool::GetTotalDeallocations( ) 
                        + Pool5::TObjectPool::GetTotalDeallocations( ) 
                        + CustomSizeDeallocations.load( ),

                     Pool1::TObjectPool::GetTotalOSAllocations( ) 
                        + Pool1::TObjectPool::GetTotalOSAllocations( ) 
                        + Pool2::TObjectPool::GetTotalOSAllocations( ) 
                        + Pool3::TObjectPool::GetTotalOSAllocations( ) 
                        + Pool4::TObjectPool::GetTotalOSAllocations( ) 
                        + Pool5::TObjectPool::GetTotalOSAllocations( ),

                     Pool1::TObjectPool::GetTotalOSDeallocations( ) 
                        + Pool1::TObjectPool::GetTotalOSDeallocations( ) 
                        + Pool2::TObjectPool::GetTotalOSDeallocations( ) 
                        + Pool3::TObjectPool::GetTotalOSDeallocations( ) 
                        + Pool4::TObjectPool::GetTotalOSDeallocations( ) 
                        + Pool5::TObjectPool::GetTotalOSDeallocations( )
                    );
            SKL_INF( "MemoryManager ###############################################################" );
#else
            SKL_WRN( "MemoryManager::LogStatistics()\n\t\tTried to log memory statistics, but the MemoryManager has the statistics turned off!" );
#endif
        }

        // Stats variables
        SKL_IFMEMORYSTATS( static SKL_CACHE_ALIGNED std::atomic<size_t> CustomSizeAllocations   );
        SKL_IFMEMORYSTATS( static SKL_CACHE_ALIGNED std::atomic<size_t> CustomSizeDeallocations );
        SKL_IFMEMORYSTATS( static SKL_CACHE_ALIGNED std::atomic<size_t> TotalAllocations        );
        SKL_IFMEMORYSTATS( static SKL_CACHE_ALIGNED std::atomic<size_t> TotalDeallocations      );
    };
}
