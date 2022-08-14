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
    constexpr bool   CMemoryManager_UseSpinLock_Or_Atomics = true;                  //!< Should the MemoryManager use SpinLock or atomic operation for internal thread sync
    constexpr size_t CMemoryManager_Alignment              = sizeof( void * );      //!< Alignment the MemoryManager will use internally for all memory blocks
    constexpr size_t CMemoryManager_Pool1_BlockSize        = 64;                    //!< [64     bytes] MemoryManager.Pool1 block size in bytes
    constexpr size_t CMemoryManager_Pool1_BlockCount       = 32768;                 //!< [32768 blocks] MemoryManager.Pool1 number of cached blocks
    constexpr size_t CMemoryManager_Pool2_BlockSize        = 128;                   //!< [128    bytes] MemoryManager.Pool2 block size in bytes
    constexpr size_t CMemoryManager_Pool2_BlockCount       = 32768;                 //!< [32768 blocks] MemoryManager.Pool2 number of cached blocks
    constexpr size_t CMemoryManager_Pool3_BlockSize        = 512;                   //!< [512    bytes] MemoryManager.Pool3 block size in bytes
    constexpr size_t CMemoryManager_Pool3_BlockCount       = 32768;                 //!< [32768 blocks] MemoryManager.Pool3 number of cached blocks
    constexpr size_t CMemoryManager_Pool4_BlockSize        = 1024;                  //!< [1024   bytes] MemoryManager.Pool4 block size in bytes
    constexpr size_t CMemoryManager_Pool4_BlockCount       = 16384;                 //!< [16384 blocks] MemoryManager.Pool4 number of cached blocks
    constexpr size_t CMemoryManager_Pool5_BlockSize        = (1024 * 512);          //!< [512   kbytes] MemoryManager.Pool5 block size in bytes
    constexpr size_t CMemoryManager_Pool5_BlockCount       = 8192;                  //!< [8192  blocks] MemoryManager.Pool5 number of cached blocks
    constexpr size_t CMemoryManager_Pool6_BlockSize        = ((1024 * 1024) * 2);   //!< [2     mbytes] MemoryManager.Pool6 block size in bytes
    constexpr size_t CMemoryManager_Pool6_BlockCount       = 8;                     //!< [8     blocks] MemoryManager.Pool6 number of cached blocks


    // Sizes guard, dont change!
    static_assert( CMemoryManager_Pool1_BlockSize < std::numeric_limits<uint32_t>::max()
                && CMemoryManager_Pool2_BlockSize < std::numeric_limits<uint32_t>::max() 
                && CMemoryManager_Pool3_BlockSize < std::numeric_limits<uint32_t>::max() 
                && CMemoryManager_Pool4_BlockSize < std::numeric_limits<uint32_t>::max() 
                && CMemoryManager_Pool5_BlockSize < std::numeric_limits<uint32_t>::max() 
                && CMemoryManager_Pool6_BlockSize < std::numeric_limits<uint32_t>::max() 
    );
}