//!
//! \file SkylakeAssert.h
//! 
//! \brief Debug and release assert capabilities for SkylakeLibHeaderOnly
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 

#include <assert.h>

#define SKL_ASSERT(expression) assert(expression)

#if defined(NDEBUG)
    //! copied from assert.h
    
    _ACRTIMP void __cdecl _wassert(
        _In_z_ wchar_t const* _Message,
        _In_z_ wchar_t const* _File,
        _In_   unsigned       _Line
        );

    #define SKL_ASSERT_ALLWAYS(expression) (void)(                                           \
            (!!(expression)) ||                                                              \
            (_wassert(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0) \
        )
#else
    #define SKL_ASSERT_ALLWAYS(expression) assert(expression)
#endif