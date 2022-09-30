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

    protected:
        std::string                TargetDirectory       {};
        std::vector<std::string>   AcceptedFileExtensions{};
        std::unique_ptr<wchar_t[]> Utf16Buffer           { nullptr };
    };
}