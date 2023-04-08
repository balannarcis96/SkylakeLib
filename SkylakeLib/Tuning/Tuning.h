//!
//! \file SkylakeLib.h
//! 
//! \brief Compile time values as config for the SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    /*------------------------------------------------------------
        MemoryManager
      ------------------------------------------------------------*/
    constexpr bool   CMemoryManager_UseSpinLock_Or_Atomics                      = true;                        //!< Should the MemoryManager use SpinLock or atomic operation for internal thread sync
    constexpr size_t CMemoryManager_Pool1_BlockSize                             = 64U;                         //!< [64     bytes] MemoryManager.Pool1 block size in bytes
    constexpr size_t CMemoryManager_Pool1_BlockCount                            = 32768U;                      //!< [32768 blocks] MemoryManager.Pool1 number of cached blocks
    constexpr size_t CMemoryManager_Pool2_BlockSize                             = 128U;                        //!< [128    bytes] MemoryManager.Pool2 block size in bytes
    constexpr size_t CMemoryManager_Pool2_BlockCount                            = 32768U;                      //!< [32768 blocks] MemoryManager.Pool2 number of cached blocks
    constexpr size_t CMemoryManager_Pool3_BlockSize                             = 512U;                        //!< [512    bytes] MemoryManager.Pool3 block size in bytes
    constexpr size_t CMemoryManager_Pool3_BlockCount                            = 32768U;                      //!< [32768 blocks] MemoryManager.Pool3 number of cached blocks
    constexpr size_t CMemoryManager_Pool4_BlockSize                             = 1024U;                       //!< [1024   bytes] MemoryManager.Pool4 block size in bytes
    constexpr size_t CMemoryManager_Pool4_BlockCount                            = 16384U;                      //!< [16384 blocks] MemoryManager.Pool4 number of cached blocks
    constexpr size_t CMemoryManager_Pool5_BlockSize                             = (1024U * 512U);              //!< [512   kbytes] MemoryManager.Pool5 block size in bytes
    constexpr size_t CMemoryManager_Pool5_BlockCount                            = 8192U;                       //!< [8192  blocks] MemoryManager.Pool5 number of cached blocks
    constexpr size_t CMemoryManager_Pool6_BlockSize                             = ((1024U * 1024U) * 2U);      //!< [2     mbytes] MemoryManager.Pool6 block size in bytes
    constexpr size_t CMemoryManager_Pool6_BlockCount                            = 8U;                          //!< [8     blocks] MemoryManager.Pool6 number of cached blocks
    SKL_IF_ALLOC_SIZE_GUARDED( constexpr size_t CMemoryManager_MaxAllocSize     = ((1024U * 1024U) * 1024U) ); //!< [1        GiB] The maximum size the MemoryManager is allowed to alloc at once
    SKL_IF_CACHE_LINE_MEM_MANAGER( constexpr size_t CMemoryManager_Alignment    = SKL_CACHE_LINE_SIZE );       //!< Align all the MemoryManager memory blocks to the cache line
    SKL_IFNOT_CACHE_LINE_MEM_MANAGER( constexpr size_t CMemoryManager_Alignment = sizeof( void * ) );          //!< Align all the MemoryManager memory blocks to 8 bytes

    // Sizes guard, don't change!
    static_assert( CMemoryManager_Pool1_BlockSize < std::numeric_limits<uint32_t>::max()
                && CMemoryManager_Pool2_BlockSize < std::numeric_limits<uint32_t>::max() 
                && CMemoryManager_Pool3_BlockSize < std::numeric_limits<uint32_t>::max() 
                && CMemoryManager_Pool4_BlockSize < std::numeric_limits<uint32_t>::max() 
                && CMemoryManager_Pool5_BlockSize < std::numeric_limits<uint32_t>::max() 
                && CMemoryManager_Pool6_BlockSize < std::numeric_limits<uint32_t>::max() 
    );
    
    enum class ELocalMemoryManagerProfilingFlags: uint16_t
    {
          None                   = 0
        , Time_PoolAllocations   = 1 << 0
        , Time_OsAllocations     = 1 << 2
        , Time_AllDeallocations  = 1 << 3
        , Time_OsDeallocations   = 1 << 4
        , Count_PoolAllocations  = 1 << 5
        , Count_OsAllocations    = 1 << 6
        , Count_AllDeallocations = 1 << 7
        , Count_OsDeallocations  = 1 << 8

        , Time_All  = Time_PoolAllocations  | Time_OsAllocations  | Time_AllDeallocations  | Time_OsDeallocations
        , Count_All = Count_PoolAllocations | Count_OsAllocations | Count_AllDeallocations | Count_OsDeallocations
        , All       = Time_All | Count_All
    };
    
    /*------------------------------------------------------------
        Thread local MemoryManager
      ------------------------------------------------------------*/
    struct ThreadLocalMemoryManagerConfig
    {
        //@TODO expose these values as compiletime configurable
        static constexpr size_t  Pool1_BlockSize  = 64U;                 
        static constexpr size_t  Pool1_BlockCount = 32768U;              
        static constexpr size_t  Pool2_BlockSize  = 128U;                
        static constexpr size_t  Pool2_BlockCount = 32768U;              
        static constexpr size_t  Pool3_BlockSize  = 512U;                
        static constexpr size_t  Pool3_BlockCount = 32768U;              
        static constexpr size_t  Pool4_BlockSize  = 1024U;               
        static constexpr size_t  Pool4_BlockCount = 16384U;              
        static constexpr size_t  Pool5_BlockSize  = (1024U * 512U);       
        static constexpr size_t  Pool5_BlockCount = 8192U;               
        static constexpr size_t  Pool6_BlockSize  = ((1024U * 1024U) * 2U);
        static constexpr size_t  Pool6_BlockCount = 8U;         

        static constexpr wchar_t PoolName[]                          = L"MainThreadLocalMemoryManager";         
        static constexpr bool    bIsThreadSafe                       = false;
        static constexpr bool    bUseSpinLock_Or_Atomics             = false;
        static constexpr bool    bAlignAllMemoryBlocksToTheCacheLine = false;
#if defined(SKL_GUARD_ALLOC_SIZE)
        static constexpr size_t  MaxAllocationSize                   = CMemoryManager_MaxAllocSize;
#else
        static constexpr size_t  MaxAllocationSize                   =  0U;
#endif

#if !defined(SKL_KPI_TLS_MEM_ALLOC_TIME) && !defined(SKL_KPI_TLS_MEM_ALLOC_CNT)
        static constexpr ELocalMemoryManagerProfilingFlags ProfilingFlags = ELocalMemoryManagerProfilingFlags::None;
#elif defined(SKL_KPI_TLS_MEM_ALLOC_TIME) && !defined(SKL_KPI_TLS_MEM_ALLOC_CNT)
        static constexpr ELocalMemoryManagerProfilingFlags ProfilingFlags = ELocalMemoryManagerProfilingFlags::Time_All;
#elif !defined(SKL_KPI_TLS_MEM_ALLOC_TIME) && defined(SKL_KPI_TLS_MEM_ALLOC_CNT)
        static constexpr ELocalMemoryManagerProfilingFlags ProfilingFlags = ELocalMemoryManagerProfilingFlags::Count_All;
#else
        static constexpr ELocalMemoryManagerProfilingFlags ProfilingFlags = ELocalMemoryManagerProfilingFlags::All;
#endif
    };

    /*------------------------------------------------------------
        Thread local MemoryManager
      ------------------------------------------------------------*/
    constexpr size_t CTLSSyncSystem_QueueSize = 524288U; //!< [ 1024 * 512 ] Max number of TLSSyncTasks in the TLSSync tasks queue at once

    /*------------------------------------------------------------
        String Utils
      ------------------------------------------------------------*/
    constexpr uint32_t CStringUtilsWorkBenchBufferSize = 8192U;
    
    /*------------------------------------------------------------
        Worker
      ------------------------------------------------------------*/
    constexpr uint32_t CMaxAsyncRequestsToDequeuePerTick = 32U;
    constexpr uint32_t CWorkerGroupNameMaxChars          = 64U;

    /*------------------------------------------------------------
        Measurements
      ------------------------------------------------------------*/
    constexpr auto CKPIPointsToAverageFrom = 8; // power of two

    /*------------------------------------------------------------
        Computed Flags
      ------------------------------------------------------------*/
#if defined(SKL_KPI_QUEUE_SIZES)
    constexpr bool CKPIQueueSizes = true;
#else
    constexpr bool CKPIQueueSizes = false;
#endif

#if defined(SKL_KPI_WORKER_TICK)
    constexpr bool CKPIWorkerTickTimings = true;
#else
    constexpr bool CKPIWorkerTickTimings = false;
#endif

#if defined(SKL_MEM_TIME_OS)
    constexpr bool CKPI_OS_MemAllocTimings = true;
#else
    constexpr bool CKPI_OS_MemAllocTimings = false;
#endif

#if defined(SKL_MEM_TIME_GLOBAL)
    constexpr bool CKPI_Global_MemAllocTimings = true;
#else
    constexpr bool CKPI_Global_MemAllocTimings = false;
#endif

#if defined(SKL_MEM_COUNTER_OS)
    constexpr bool CKPI_OS_MemAllocCount = true;
#else
    constexpr bool CKPI_OS_MemAllocCount = false;
#endif

#if defined(SKL_MEM_COUNTER_GLOBAL)
    constexpr bool CKPI_Global_MemAllocCount = true;
#else
    constexpr bool CKPI_Global_MemAllocCount = false;
#endif
}