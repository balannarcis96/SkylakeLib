//!
//! \file SkylakeAssert.h
//! 
//! \brief Debug and release assert capabilities for SkylakeLibHeaderOnly
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 

#undef NDEBUG
#define NDEBUG
#include <assert.h>

#if defined(SKL_BUILD_SHIPPING)
    #define SKL_ASSERT(expression)
#else
    #define SKL_ASSERT(expression) assert(expression)
#endif

#if not defined(SKL_NO_ASSERTS)
    #define SKL_ASSERT_ALLWAYS(expression) assert(expression)
#endif

