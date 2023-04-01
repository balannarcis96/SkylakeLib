//!
//! \file StringUtils.h
//! 
//! \brief Skylake String utils abstractions
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    struct StringUtils final : protected ITLSSingleton<StringUtils>
    {
        StringUtils() noexcept :
            WorkBenchBuffer{ CStringUtilsWorkBenchBufferSize } {}

        //! Initialize
        RStatus Initialize() noexcept override 
        { 
            return RSuccess; 
        }

        //! Get the instance name
        const char* GetName() const noexcept override
        {
            return "[StringUtils]";
        }

        //! Convert ipv4 address to string 
        static const char* IpV4AddressToString( TIPv4Address InAddress ) noexcept;

        //! Convert ipv4 address to wide string 
        static const wchar_t* IpV4AddressToWString( TIPv4Address InAddress ) noexcept;

        //! Convert wide string(UTF16) to multi byte string (UTF8)
        static const char* ConvertUtf16ToUtf8( const wchar_t* InWString, size_t MaxCharCountInString ) noexcept;
        
        //! Convert  multi byte string (UTF8) to wide string(UTF16)
        static const wchar_t* ConvertUtf8ToUtf16( const char* InString, size_t MaxCharCountInString ) noexcept;

    private:
        BufferStream WorkBenchBuffer;

        friend class ServerInstance;
        friend RStatus Skylake_InitializeLibrary_Thread() noexcept;
        friend RStatus Skylake_TerminateLibrary_Thread() noexcept;
    };
}
