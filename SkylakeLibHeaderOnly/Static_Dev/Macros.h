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
#define SKL_CACHE_LINE      sizeof( void* ) * 8 /*64bytes*/
#define SKL_CACHE_ALIGNED   alignas( SKL_CACHE_LINE ) 

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

//! Clock helpers
#define TCLOCK_MILLIS( x ) std::chrono::milliseconds( x )
#define TCLOCK_MICROS( x ) std::chrono::microseconds( x )
#define TCLOCK_SLEEP_FOR_MILLIS( int_value ) std::this_thread::sleep_for( TCLOCK_MILLIS( int_value ) )
#define TCLOCK_SLEEP_FOR_MICROS( int_value ) std::this_thread::sleep_for( TCLOCK_MICROS( int_value ) )

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

