//!
//! \file SkylakeLib.h
//! 
//! \brief Compile time values as config for the SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

#ifndef SKLL_CTASKSCHEDULING_ASSUMETHATTASKHANDLINGWORKERGROUPCOUNTISPOWEROFTWO
#define SKLL_CTASKSCHEDULING_ASSUMETHATTASKHANDLINGWORKERGROUPCOUNTISPOWEROFTWO false
#endif

#ifndef SKLL_CTASKSCHEDULING_ASSUMETHATWORKERSCOUNTISPOWEROFTWO
#define SKLL_CTASKSCHEDULING_ASSUMETHATWORKERSCOUNTISPOWEROFTWO false
#endif

#ifndef SKLL_CTASKSCHEDULING_ASSUMEALLWORKERGROUPSHANDLETIMERTASKS
#define SKLL_CTASKSCHEDULING_ASSUMEALLWORKERGROUPSHANDLETIMERTASKS false
#endif

#ifndef SKLL_CTASKSCHEDULING_ASSUMEALLWORKERGROUPSHANDLEAOD
#define SKLL_CTASKSCHEDULING_ASSUMEALLWORKERGROUPSHANDLEAOD false
#endif

#ifndef SKLL_CTASKSCHEDULING_ASSUMEALLWORKERGROUPSHAVETLSMEMORYMANAGEMENT
#define SKLL_CTASKSCHEDULING_ASSUMEALLWORKERGROUPSHAVETLSMEMORYMANAGEMENT false
#endif

#ifndef SKLL_CTASKSCHEDULING_USEIFINSTEADOFMODULO
#define SKLL_CTASKSCHEDULING_USEIFINSTEADOFMODULO false
#endif

#ifndef SKLL_CTASK_DOTHROTTLEGENERALTASKEXECUTION
#define SKLL_CTASK_DOTHROTTLEGENERALTASKEXECUTION true
#endif

namespace SKL
{
    /*------------------------------------------------------------
        Feature flags
      ------------------------------------------------------------*/
    constexpr bool CTaskScheduling_AssumeThatTaskHandlingWorkerGroupCountIsPowerOfTwo = SKLL_CTASKSCHEDULING_ASSUMETHATTASKHANDLINGWORKERGROUPCOUNTISPOWEROFTWO;
    constexpr bool CTaskScheduling_AssumeThatWorkersCountIsPowerOfTwo = SKLL_CTASKSCHEDULING_ASSUMETHATWORKERSCOUNTISPOWEROFTWO;
    constexpr bool CTaskScheduling_AssumeAllWorkerGroupsHandleTimerTasks = SKLL_CTASKSCHEDULING_ASSUMEALLWORKERGROUPSHANDLETIMERTASKS;
    constexpr bool CTaskScheduling_AssumeAllWorkerGroupsHandleAOD = SKLL_CTASKSCHEDULING_ASSUMEALLWORKERGROUPSHANDLEAOD;
    constexpr bool CTaskScheduling_AssumeAllWorkerGroupsHaveTLSMemoryManagement = SKLL_CTASKSCHEDULING_ASSUMEALLWORKERGROUPSHAVETLSMEMORYMANAGEMENT;
    constexpr bool CTaskScheduling_UseIfInsteadOfModulo = SKLL_CTASKSCHEDULING_USEIFINSTEADOFMODULO;
    constexpr bool CTask_DoThrottleGeneralTaskExecution = SKLL_CTASK_DOTHROTTLEGENERALTASKEXECUTION;

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

    // Sizes guard, dont change!
    static_assert( CMemoryManager_Pool1_BlockSize < std::numeric_limits<uint32_t>::max()
                && CMemoryManager_Pool2_BlockSize < std::numeric_limits<uint32_t>::max() 
                && CMemoryManager_Pool3_BlockSize < std::numeric_limits<uint32_t>::max() 
                && CMemoryManager_Pool4_BlockSize < std::numeric_limits<uint32_t>::max() 
                && CMemoryManager_Pool5_BlockSize < std::numeric_limits<uint32_t>::max() 
                && CMemoryManager_Pool6_BlockSize < std::numeric_limits<uint32_t>::max() 
    );

    /*------------------------------------------------------------
        Thread local MemoryManager
      ------------------------------------------------------------*/
    struct ThreadLocalMemoryManagerConfig
    {
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
    };

    /*------------------------------------------------------------
        Thread local MemoryManager
      ------------------------------------------------------------*/
    constexpr size_t CTLSSyncSystem_QueueSize = 524288U; //!< [ 1024 * 512 ] Max number of TLSSyncTasks in the TLSSync tasks queue at once

    /*------------------------------------------------------------
        String Utils
      ------------------------------------------------------------*/
    constexpr uint32_t CStringUtilsWorkBenchBufferSize = 8192U;
}