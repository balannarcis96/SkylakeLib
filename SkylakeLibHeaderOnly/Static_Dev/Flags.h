//!
//! \file Flags.h
//! 
//! \brief Macro flags
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

#if defined(SKL_BUILD_SHIPPING)
    #define SKL_IFNOTSHIPPING( expr ) 
    #define SKL_IFSHIPPING( expr ) expr
#else
    #define SKL_IFNOTSHIPPING( expr ) expr 
    #define SKL_IFSHIPPING( expr ) 
#endif

#if defined(SKL_MEMORY_STATISTICS)
    #define SKL_IFNOTMEMORYSTATS( expr ) 
    #define SKL_IFMEMORYSTATS( expr ) expr
#else
    #define SKL_IFNOTMEMORYSTATS( expr ) expr 
    #define SKL_IFMEMORYSTATS( expr ) 
#endif

#if defined(SKL_GUARD_ALLOC_SIZE)
    #define SKL_IF_ALLOC_SIZE_GUARDED( expr ) expr
    #define SKL_GUARD_ALLOC_SIZE_ON 1
#else
    #define SKL_IF_ALLOC_SIZE_GUARDED( expr ) 
    #define SKL_GUARD_ALLOC_SIZE_ON 0
#endif

#if defined(SKL_CACHE_LINE_MEM_MANAGER)
    #define SKL_IF_CACHE_LINE_MEM_MANAGER( expr ) expr
    #define SKL_IFNOT_CACHE_LINE_MEM_MANAGER( expr )
#else
    #define SKL_IF_CACHE_LINE_MEM_MANAGER( expr )
    #define SKL_IFNOT_CACHE_LINE_MEM_MANAGER( expr ) expr
#endif

#if !defined(SKL_L1_CACHE_LINE_64)
    #error Unsupported cache line size!
#endif
