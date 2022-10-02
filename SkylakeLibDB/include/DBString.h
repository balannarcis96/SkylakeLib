//!
//! \file SkylakeLibDB.h
//! 
//! \brief MySQL c++ interface library
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL::DB
{
    template<size_t MaxSize>
    struct DBString
    {
        static constexpr size_t CUTF16Size = MaxSize;
        static constexpr size_t CUTF8Size = CUTF16Size * 4; //utf8mb4

        DBString() noexcept = default;
        ~DBString() noexcept = default;

        // Copy
        DBString( const DBString &Other ) noexcept
            : bHasSource{ Other.bHasSource }
            , bIsUTF8Source{ Other.bIsUTF8Source }
            , bHasUTF8{ Other.bHasUTF8 }
            , bHasUTF16{ Other.bHasUTF16 }
        {
            ::memcpy( Utf8, Other.Utf8, CUTF8Size );
            ::memcpy( Utf16, Other.Utf16, sizeof( wchar_t ) * CUTF16Size );
        }
        DBString &operator=( const DBString &Other ) noexcept
        {
            SKL_ASSERT( this != &Other );

            bHasSource    = Other.bHasSource;
            bIsUTF8Source = Other.bIsUTF8Source;
            bHasUTF8      = Other.bHasUTF8;
            bHasUTF16     = Other.bHasUTF16;

            ::memcpy( Utf8, Other.Utf8, CUTF8Size );
            ::memcpy( Utf16, Other.Utf16, sizeof( wchar_t ) * CUTF16Size );

            return *this;
        }

        // Cant move
        DBString( DBString && ) = delete;
        DBString &operator=( DBString && ) = delete;

        SKL_FORCEINLINE static DBString<MaxSize> FromUtf8( const char* InUtf8 ) noexcept
        {
            return DBString<MaxSize>{ InUtf8 };
        }
        SKL_FORCEINLINE static DBString<MaxSize> FromUtf16( const wchar_t* InUtf16 ) noexcept
        {
            return DBString<MaxSize>{ InUtf16 };
        }

        SKL_NODISCARD char *GetUtf8( ) noexcept
        {
            if( true == bHasSource && false == bHasUTF8 )
            {
                if( false == GWideCharToMultiByte( Utf16, Utf8 ) )
                {
                    return nullptr;
                }

                bHasUTF8 = true;
            }

            return Utf8;
        }
        SKL_NODISCARD wchar_t *GetUtf16( bool bForce = false ) noexcept
        {
            if( true == bHasSource && ( false == bHasUTF16 || true == bForce ) )
            {
                if( false == GMultiByteToWideChar( Utf8, Utf16 ) )
                {
                    return nullptr;
                }

                bHasUTF16 = true;
            }

            return Utf16;
        }

        SKL_NODISCARD size_t GetUtf8SizeNoConvert( ) const noexcept
        {
            if( false == bHasSource )
            {
                return 0;
            }

            return SKL_STRLEN( Utf8, CUTF8Size );
        }
        SKL_NODISCARD size_t GetUtf16SizeNoConvert( ) const noexcept
        {
            if( false == bHasSource )
            {
                return 0;
            }

            return SKL_WSTRLEN( Utf16, CUTF16Size );
        }
        SKL_NODISCARD size_t GetUtf8Size( ) noexcept
        {
            if( false == bHasSource )
            {
                return 0;
            }

            if( false == bHasUTF8 )
            {
                return SKL_STRLEN( GetUtf8( ), CUTF8Size );
            }

            return SKL_STRLEN( Utf8, CUTF8Size );
        }
        SKL_NODISCARD size_t GetUtf16Size( ) noexcept
        {
            if( false == bHasSource )
            {
                return 0;
            }

            if( false == bHasUTF16 )
            {
                return SKL_WSTRLEN( GetUtf16( ), CUTF16Size );
            }

            return SKL_WSTRLEN( Utf16, CUTF16Size );
        }

        bool operator==( const char *Utf8Str ) noexcept
        {
            if( false == bHasSource )
            {
                return false;
            }

            if( false == bHasUTF8 )
            {
                return SKL_STRCMP( GetUtf8( ), Utf8Str, CUTF8Size ) == 0;
            }

            return SKL_STRCMP( Utf8, Utf8Str, CUTF8Size ) == 0;
        }
        bool operator==( const wchar_t *Utf16Str ) noexcept
        {
            if( false == bHasSource )
            {
                return false;
            }

            if( false == bHasUTF16 )
            {
                return SKL_WSTRCMP( GetUtf16(), Utf16Str, CUTF16Size ) == 0;
            }

            return SKL_WSTRCMP( Utf16, Utf16Str, CUTF16Size ) == 0;
        }
        SKL_FORCEINLINE bool operator!=( const char *Utf8Str ) noexcept
        {
            return false == operator==( Utf8Str );
        }
        SKL_FORCEINLINE bool operator!=( const wchar_t *Utf16Str ) noexcept
        {
            return false == operator==( Utf16Str );
        }

        SKL_FORCEINLINE void CopyUtf16Into( wchar_t *TargetBuffer, const size_t TargetBufferSizeInWords ) noexcept
        {
            SKL_WSTRCPY( TargetBuffer, GetUtf16( ), TargetBufferSizeInWords );
        }
        SKL_FORCEINLINE void CopyUtf8Into( char *TargetBuffer, const size_t TargetBufferSize ) noexcept
        {
            SKL_STRCPY( TargetBuffer, GetUtf8(), TargetBufferSize );
        }
        SKL_FORCEINLINE void CopyUtf16IntoNoConvert( wchar_t *TargetBuffer, const size_t TargetBufferSizeInWords ) const noexcept
        {
            SKL_ASSERT( true == bHasUTF16 );
            SKL_WSTRCPY( TargetBuffer, Utf16, TargetBufferSizeInWords );
        }
        SKL_FORCEINLINE void CopyUtf8IntoNoConvert( char *TargetBuffer, const size_t TargetBufferSize ) const noexcept
        {
            SKL_ASSERT( true == bHasUTF8 );
            SKL_STRCPY( TargetBuffer, Utf8, TargetBufferSize );
        }
    private:
        explicit DBString( const char *Utf8 ) noexcept 
            : bHasSource{ true }
            , bIsUTF8Source{ true }
            , bHasUTF8{ true }
        {
            SKL_STRCPY( this->Utf8, Utf8, CUTF8Size );
            Utf16[ 0 ] = L'\0';
        }

        explicit DBString( const wchar_t *Utf16 ) noexcept 
            : bHasSource{ true }
            , bHasUTF16{ true }
        {
            SKL_WSTRCPY( this->Utf16, Utf16, CUTF16Size );
            Utf8[ 0 ] = '\0';
        }

        bool    bHasSource          { false };
        bool    bIsUTF8Source       { false };
        bool    bHasUTF8            { false };
        bool    bHasUTF16           { false };
        char    Utf8[ CUTF8Size ]   { 0 };
        wchar_t Utf16[ CUTF16Size ] { 0 };

        friend struct DBStatement;
    };
}