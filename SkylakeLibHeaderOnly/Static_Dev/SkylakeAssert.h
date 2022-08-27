//!
//! \file SkylakeAssert.h
//! 
//! \brief Debug and release assert capabilities for SkylakeLibHeaderOnly
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 

#if !defined(SKL_BUILD_SHIPPING)
    #undef NDEBUG
#endif
#include <assert.h>

#if defined(SKL_BUILD_SHIPPING)
    #define SKL_ASSERT(expression)
    #define SKL_ASSERT_MSG(expression, msg)
#else
    #define SKL_ASSERT(expression) { auto ASSERT_RESULT{ !!(expression) }; assert(ASSERT_RESULT); if( !(ASSERT_RESULT) ) { SKL_BREAK(); } }
    #define SKL_ASSERT_MSG(expression, msg) { auto ASSERT_RESULT{ !!(expression) }; assert(ASSERT_RESULT && msg); if( !ASSERT_RESULT ) { SKL_BREAK(); } }
#endif

#if not defined(SKL_NO_ASSERTS)
    #define SKL_ASSERT_ALLWAYS(expression) { auto ASSERT_RESULT{ !!(expression) }; assert(ASSERT_RESULT); if( !(ASSERT_RESULT) ) { SKL_BREAK(); } }
    #define SKL_ASSERT_ALLWAYS_MSG(expression, msg) { auto ASSERT_RESULT{ !!(expression) }; assert(ASSERT_RESULT && msg); if( !ASSERT_RESULT ) { SKL_BREAK(); } }
#endif

