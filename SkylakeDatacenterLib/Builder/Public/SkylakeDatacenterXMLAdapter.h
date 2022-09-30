//!
//! \file SkylakeDatacenterXMLAdapter.h
//! 
//! \brief Skylake Datacenter XML adapter abstractions and utilities
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

#include "SkylakeDatacenterBuilder.h"

namespace SKL::DC
{
    struct DatacenterXMLAdapter : DatacenterAdapter
    {
        static constexpr size_t CBuffersLength = 4096;

        DatacenterXMLAdapter() noexcept
        {
            AddAcceptedFileExtension( ".xml" );
            
            Utf8Buffer.reset( new char[ CBuffersLength ] );
            SKL_ASSERT( nullptr != Utf8Buffer.get() );
            Utf8Buffer[0] = L'\0';

            Utf16Buffer.reset( new wchar_t[ CBuffersLength ] );
            SKL_ASSERT( nullptr != Utf16Buffer.get() );
            Utf16Buffer[0] = L'\0';
        }

        std::unique_ptr<RawElement> BuildRawStructure() noexcept override;

        SKL_FORCEINLINE void AddAcceptedFileExtension( std::string InFileExtension ) noexcept
        {
            AcceptedFileExtensions.emplace_back( std::move( InFileExtension ) );
        }
        SKL_FORCEINLINE const char* GetTargetDirectory() const noexcept { return TargetDirectory.c_str(); }
        SKL_FORCEINLINE void SetTargetDirectory( const char* InTargetDirectory ) noexcept { TargetDirectory = InTargetDirectory; }

        SKL_FORCEINLINE std::pair<wchar_t*, size_t> GetUtf16Buffer() const noexcept { return { Utf16Buffer.get(), CBuffersLength }; }
        SKL_FORCEINLINE const wchar_t* ConvertUtf8ToUtf16( const char* InStr, size_t InStringLength ) noexcept
        {
            if( true == SKL::GMultiByteToWideChar( InStr, InStringLength, Utf16Buffer.get(), CBuffersLength ) )
            {
                return Utf16Buffer.get();
            }

            return nullptr;
        }
        SKL_FORCEINLINE std::pair<char*, size_t> GetUtf8Buffer() const noexcept { return { Utf8Buffer.get(), CBuffersLength }; }
        SKL_FORCEINLINE const wchar_t* ConvertUtf16ToUtf8( const wchar_t* InStr, size_t InStringLengthInWChars ) noexcept
        {
            if( true == SKL::GWideCharToMultiByte( InStr, InStringLengthInWChars, Utf8Buffer.get(), CBuffersLength ) )
            {
                return Utf16Buffer.get();
            }

            return nullptr;
        }

    protected:
        std::string                TargetDirectory       {};
        std::vector<std::string>   AcceptedFileExtensions{};
        std::unique_ptr<char[]>    Utf8Buffer            { nullptr };
        std::unique_ptr<wchar_t[]> Utf16Buffer           { nullptr };
    };
}