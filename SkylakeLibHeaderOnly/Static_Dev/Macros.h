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

#if ( defined(_MSC_VER) || defined(__INTEL_COMPILER) ) && !defined(SKL_BUILD_SHIPPING)
    #define SKL_ALLOCATOR_FUNCTION __declspec(allocator)
    #define SKL_NOVTABLE __declspec(novtable)
#else
    #define SKL_ALLOCATOR_FUNCTION 
    #define SKL_NOVTABLE
#endif

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

//! Clock helpers
#define TCLOCK_MILLIS( x ) std::chrono::milliseconds( x )
#define TCLOCK_MICROS( x ) std::chrono::microseconds( x )
#define TCLOCK_SLEEP_FOR_MILLIS( int_value ) std::this_thread::sleep_for( TCLOCK_MILLIS( int_value ) )
#define TCLOCK_SLEEP_FOR_MICROS( int_value ) std::this_thread::sleep_for( TCLOCK_MICROS( int_value ) )

#if defined(_MSC_VER)
    #define SKL_STRCPY( InDest, InSrc, InSizeInBytes ) ::strcpy_s( InDest, InSizeInBytes, InSrc )
    #define SKL_WSTRCPY( InDest, InSrc, InSizeInWords ) ::wcscpy_s( InDest, InSizeInWords, InSrc )
    
    #define SKL_STRLEN( InStr, InSizeInBytes ) ::strnlen_s( InStr, InSizeInBytes )
    #define SKL_WSTRLEN( InStr, InSizeInWords ) ::wcsnlen_s( InStr, InSizeInWords  )

    #define SKL_STRCMP( InStr1, InStr2, InMaxSizeInBytes ) ::strncmp( InStr1, InStr2, InMaxSizeInBytes )
    #define SKL_WSTRCMP( InStr1, InStr2, InMaxSizeInWords ) ::wcsncmp( InStr1, InStr2, InMaxSizeInWords )

    #define SKL_MEMCPY( InSrc, InSrcSize, InDest, InDestSize ) ::memcpy_s( InSrc, InSrcSize, InDest, InDestSize )
    #define SKL_MEMMOVE( InSrc, InSrcSize, InDest, InDestSize ) ::memmove_s( InSrc, InSrcSize, InDest, InDestSize )
#else
    #define SKL_STRCPY( InDest, InSrc, InSizeInBytes ) ::strcpy( InDest, InSrc )
    #define SKL_WSTRCPY( InDest, InSrc, InSizeInWords ) ::wcscpy( InDest, InSrc )

    #define SKL_STRLEN( InStr, InSizeInBytes ) ::strlen( InStr )
    #define SKL_WSTRLEN( InStr, InSizeInWords ) ::wcslen( InStr )

    #define SKL_STRCMP( InStr1, InStr2, InMaxSizeInBytes ) ::strcmp( InStr1, InStr2 )
    #define SKL_WSTRCMP( InStr1, InStr2, InMaxSizeInWords ) ::wcscmp( InStr1, InStr2 )
    
    #define SKL_MEMCPY( InSrc, InSrcSize, InDest, InDestSize ) ::memcpy( InSrc, InDest, InDestSize )
    #define SKL_MEMMOVE( InSrc, InSrcSize, InDest, InDestSize ) ::memmove( InSrc, InDest, InDestSize )
#endif

namespace SKL
{
    constexpr uint16_t CPlatformCacheLineSize = static_cast<uint16_t>( SKL_CACHE_LINE_SIZE );
}
