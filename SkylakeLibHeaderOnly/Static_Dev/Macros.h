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
#define SKL_COMPILER_NAME   ASD_COMPILER_NAME
#define SKL_CDECL           ASD_CDECL     
#define SKL_STDCALL         ASD_STDCALL   
#define SKL_THISCALL        ASD_THISCALL  
#define SKL_FASTCALL        ASD_FASTCALL  
#define SKL_VECTORCALL      ASD_VECTORCALL
#define SKL_FALLTHROUGH     ASD_FALLTHROUGH

#define SKL_ALLWAYS_LIKELY [[likely]]
#define SKL_ALLWAYS_UNLIKELY [[unlikely]]

#if defined(SKL_ENABLE_LIKELY_FLAGS)
    #define SKL_LIKELY [[likely]]
#else
    #define SKL_LIKELY
#endif

#if defined(SKL_ENABLE_UNLIKELY_FLAGS)
    #define SKL_UNLIKELY [[unlikely]]
#else
    #define SKL_UNLIKELY
#endif

#if ( defined(_MSC_VER) || defined(__INTEL_COMPILER) ) && !defined(SKL_BUILD_SHIPPING)
    #define SKL_ALLOCATOR_FUNCTION __declspec(allocator)
    #define SKL_NOVTABLE __declspec(novtable)
#else
    #define SKL_ALLOCATOR_FUNCTION 
    #define SKL_NOVTABLE
#endif

#define SKL_NORETURN [[noreturn]]

#define SKL_ALIGNMENT sizeof( void* )

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

#define SKL_STRCMP( InStr1, InStr2, InMaxSizeInBytes ) ::strncmp( InStr1, InStr2, InMaxSizeInBytes )
#define SKL_WSTRCMP( InStr1, InStr2, InMaxSizeInWords ) ::wcsncmp( InStr1, InStr2, InMaxSizeInWords )

#define SKL_STRICMP( InStr1, InStr2, InMaxSizeInBytes ) ::_strnicmp( InStr1, InStr2, InMaxSizeInBytes )
#define SKL_WSTRICMP( InStr1, InStr2, InMaxSizeInWords ) ::_wcsnicmp( InStr1, InStr2, InMaxSizeInWords )

#if defined(_MSC_VER)
    #define SKL_STRCPY( InDest, InSrc, InSizeInBytes ) ::strcpy_s( InDest, InSizeInBytes, InSrc )
    #define SKL_WSTRCPY( InDest, InSrc, InSizeInWords ) ::wcscpy_s( InDest, InSizeInWords, InSrc )
    
    #define SKL_STRLEN( InStr, InSizeInBytes ) ::strnlen_s( InStr, InSizeInBytes )
    #define SKL_WSTRLEN( InStr, InSizeInWords ) ::wcsnlen_s( InStr, InSizeInWords  )

    #define SKL_MEMCPY( InDest, InDestSize, InSrc, InSrcSize ) ::memcpy_s( InDest, InDestSize, InSrc, InSrcSize )
    #define SKL_MEMMOVE( InDest, InDestSize, InSrc, InSrcSize ) ::memmove_s( InDest, InDestSize,InSrc, InSrcSize )
#else
    #error @TODO
#endif

namespace SKL
{
    constexpr uint16_t CPlatformCacheLineSize = static_cast<uint16_t>( SKL_CACHE_LINE_SIZE );
}

#if SKL_USE_LARGE_WORLD_COORDS
    #define SKL_REAL_VALUE( x ) ( x )
#else
    #define SKL_REAL_VALUE( x ) ( x##f )
#endif

#define SKL_NO_MOVE_OR_COPY( ClassName )                \
    ClassName ( const ClassName & ) = delete;           \
    ClassName & operator=( const ClassName & ) = delete;\
    ClassName ( ClassName && ) = delete;                \
    ClassName & operator=( ClassName && ) = delete
    
#define SKL_DEFAULT_STATIC_CLASS( ClassName )           \
    ClassName () noexcept = default;                    \
    ~ClassName () noexcept = default;                   \
    ClassName ( const ClassName & ) = delete;           \
    ClassName & operator=( const ClassName & ) = delete;\
    ClassName ( ClassName && ) = delete;                \
    ClassName & operator=( ClassName && ) = delete
    
// These classes are meant to be used for ptr/ref types as an ATP/ATR idiom( API through pointer/ref idiom )
#define SKL_TYPE_PURE_INTERFACE_CLASS( ClassName )      \
    ClassName () noexcept = delete;                     \
    ~ClassName () noexcept = delete;                    \
    ClassName ( const ClassName & ) = delete;           \
    ClassName & operator=( const ClassName & ) = delete;\
    ClassName ( ClassName && ) = delete;                \
    ClassName & operator=( ClassName && ) = delete

