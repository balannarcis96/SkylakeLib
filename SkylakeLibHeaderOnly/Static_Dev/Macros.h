//!
//! \file Macros.h
//! 
//! \brief Macros for SkylakeLibHeaderOnly
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 

//! Forward ASD macros to SKL macros
#define SKL_FORCEINLINE     ASD_FORCEINLINE
#define SKL_NOINLINE        ASD_NOINLINE
#define SKL_NODISCARD       ASD_NODISCARD
#define SKL_UNLIKELY        ASD_UNLIKELY
#define SKL_LIKELY          ASD_LIKELY
#define SKL_FALLTHROUGH     ASD_FALLTHROUGH
#define SKL_COMPILER_NAME   ASD_COMPILER_NAME
#define SKL_CDECL           ASD_CDECL     
#define SKL_STDCALL         ASD_STDCALL   
#define SKL_THISCALL        ASD_THISCALL  
#define SKL_FASTCALL        ASD_FASTCALL  
#define SKL_VECTORCALL      ASD_VECTORCALL

#define SKL_ALIGNMENT       sizeof( void* ) 

#undef CONCAT_
#define CONCAT_( x, y ) x##y

#undef CONCAT
#define CONCAT( x, y ) CONCAT_( x, y )

#undef STRINGIFY
#define STRINGIFY( x ) #x

#undef TOSTRING
#define TOSTRING( x ) STRINGIFY( x )

#undef FALSE
#define FALSE 0

#undef TRUE
#define TRUE 1

//! Skylake real value helper
#if defined(SKL_REAL_TYPE_SINGLE)
    #define SKL_REAL_VALUE( x ) ( x##f )
#elif defined(SKL_REAL_TYPE_DOUBLE)
    #define SKL_REAL_VALUE( x ) ( x )
#else
    #error "Unknown real type!"
#endif

//! Skylake math values
#if defined(SKL_HEADERONLY_ENABLE_MATH)
    #undef PI
    #define PI SKL_REAL_VALUE( 3.1415926535897932 )
    #define SMALL_NUMBER SKL_REAL_VALUE( 1.e-8 )
    #define KINDA_SMALL_NUMBER SKL_REAL_VALUE( 1.e-4 )
    #define BIG_NUMBER SKL_REAL_VALUE( 3.4e+38 )
    #define EULERS_NUMBER SKL_REAL_VALUE( 2.71828182845904523536 )
#endif

//! Skylake real general values
#define SK_REAL_ZERO SKL_REAL_VALUE( 0.0 )
#define SK_REAL_ONE SKL_REAL_VALUE( 1.0 )

//! Clock helpers
#define TCLOCK_MILLIS( x ) std::chrono::milliseconds( x )
#define TCLOCK_SLEEP_FOR_MILLIS( int_value ) std::this_thread::sleep_for( TCLOCK_MILLIS( int_value ) )
