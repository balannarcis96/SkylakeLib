//!
//! \file SkylakeDatacenterAdapter.h
//! 
//! \brief Skylake Datacenter adapter abstractions and utilities
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

#include <SkylakeDatacenter.h>

namespace SKL::DC
{
    using TFilterIndex = int32_t;
    struct DatacenterAdapter;
    struct Builder;

    constexpr TFilterIndex CNoParticularFilter = 0;

    struct RawAttribute
    {
        void BuildHash() noexcept;
    
        SKL_FORCEINLINE uint64_t GetHash() const noexcept { return Hash; }
        SKL_FORCEINLINE const wchar_t* GetName() const noexcept { return Name.c_str(); }
        SKL_FORCEINLINE const uint32_t GetNameSize() const noexcept { return static_cast<uint32_t>( Name.length() ); }
        SKL_FORCEINLINE void SetName( const wchar_t* InName ) noexcept { Name = InName; }
        SKL_FORCEINLINE const wchar_t* GetValue() const noexcept { return Value.c_str(); }
        SKL_FORCEINLINE const uint32_t GetValueSize() const noexcept { return static_cast<uint32_t>( Value.length() ); }
        SKL_FORCEINLINE void SetValue( const wchar_t* InValue ) noexcept { Value = InValue; }
    private:
        uint64_t      Hash              { 0 };
        std::wstring  Name              {};
        std::wstring  Value             {};
        TStringIndex  CachedNameIndex   { CInvalidStringIndex };
        TBlockIndices CachedValueIndices{ CInvalidBlockIndex, CInvalidBlockIndex };
        TBlockIndices CachedMyLocation  { CInvalidBlockIndex, CInvalidBlockIndex };

        friend DatacenterAdapter;
        friend Builder;
    };

    struct RawElement
    {
        RawElement() noexcept = default;
        ~RawElement() noexcept;

        SKL_FORCEINLINE void AddReference() noexcept{ ++ReferenceCount; }
        SKL_FORCEINLINE bool RemoveReference() noexcept
        {
            SKL_ASSERT( 0 != ReferenceCount );
            return 0 == --ReferenceCount;
        }
        SKL_FORCEINLINE bool HasValidDCIndices() const noexcept{ return CInvalidBlockIndex != CachedIndices.first && CInvalidBlockIndex != CachedIndices.second; }

        void BuildHash() noexcept;
        uint64_t BuildKey() const noexcept;

        SKL_FORCEINLINE uint64_t GetHash() const noexcept { return Hash; }

        SKL_FORCEINLINE const wchar_t* GetName() const noexcept { return Name.c_str(); }
        SKL_FORCEINLINE void SetName( const wchar_t* InName ) noexcept { Name = InName; }
        SKL_FORCEINLINE const uint32_t GetNameSize() const noexcept { return static_cast<uint32_t>( Name.length() ); }
        
        SKL_FORCEINLINE const wchar_t* GetValue() const noexcept { return Value.c_str(); }
        SKL_FORCEINLINE void SetValue( const wchar_t* InValue ) noexcept { Value = InValue; }
        SKL_FORCEINLINE const uint32_t GetValueSize() const noexcept { return static_cast<uint32_t>( Value.length() ); }

        SKL_FORCEINLINE RawElement* GetParent() const noexcept { return Parent; }
        SKL_FORCEINLINE void SetParent( RawElement* InParent ) noexcept { Parent = InParent; }
        
        SKL_FORCEINLINE void AddChild( RawElement* InElement ) noexcept { Children.push_back( InElement ); }
        SKL_FORCEINLINE void AddAttribute( RawAttribute&& InAttribute ) noexcept { Attributes.push_back( std::forward<RawAttribute>( InAttribute ) ); }

    private:
        std::wstring  Name              {};
        std::wstring  Value             {};
        uint64_t      Hash              { 0 };
        RawElement*   Parent            { nullptr };
        RawElement*   DuplicationOf     { nullptr };
        uint32_t      ReferenceCount    { 0 };
        TBlockIndices CachedIndices     { CInvalidBlockIndex, CInvalidBlockIndex };
        TBlockIndices CachedValueIndices{ CInvalidBlockIndex, CInvalidBlockIndex };
        TBlockIndices CachedMyIndices   { CInvalidBlockIndex, CInvalidBlockIndex };
        TStringIndex  CachedNameIndex   { CInvalidStringIndex };

        std::vector<RawAttribute> Attributes;
        std::vector<RawElement*>  Children;

        friend DatacenterAdapter;
        friend Builder;
    };

    struct DatacenterAdapter
    {
        DatacenterAdapter() = default;
        virtual ~DatacenterAdapter() = default;

        virtual bool IsLanguageAttributeByName( const std::string_view& InString ) const noexcept;
        virtual const wchar_t* GetRootNodeName() const noexcept { return L"__root__"; }

        virtual TLanguage ParseLanguageFromUtf8String( const char* InLanguageStr ) const noexcept = 0;
        virtual const char* GetLanguageString( TLanguage InLanguage ) const noexcept = 0;
        virtual bool ShouldSkipAttributeByName( const std::string_view& InString ) const noexcept = 0;
        virtual bool ShouldSkipElementByName( const std::string_view& InString ) const noexcept = 0;
        virtual const wchar_t* CleanAndConvertToUtf16ElementName( const std::string_view& InString ) noexcept = 0;
        virtual const wchar_t* CleanAndConvertToUtf16AttributeName( const std::string_view& InString ) noexcept = 0;
        virtual const wchar_t* ConvertUtf8ToUtf16( const char* InStr, size_t InStringLength ) noexcept = 0;
        virtual const char* ConvertUtf16ToUtf8( const wchar_t* InStr, size_t InStringLengthInWChars ) noexcept = 0;
        virtual std::vector<std::string> ScanForFilesInDirectory( const char* InRootDirectory, size_t& OutMaxFileSize, const std::vector<std::string>& InEtensions ) noexcept = 0;

        virtual std::unique_ptr<RawElement> BuildRawStructure() noexcept = 0;

        SKL_FORCEINLINE RawElement* GetRootElement() const noexcept { return Root.get(); }
        SKL_FORCEINLINE TLanguage GetCurrentLanguageFilter() const noexcept { return LanguageFilter; }
        SKL_FORCEINLINE void SetCurrentLanguageFilter( TLanguage InLanguageFilter ) noexcept { LanguageFilter = InLanguageFilter; }
        SKL_FORCEINLINE TFilterIndex GetFilterIndex() const noexcept { return FilterIndex; }
        SKL_FORCEINLINE void SetFilterIndex( TFilterIndex InIndex ) noexcept { FilterIndex = InIndex; }
    protected:
        std::unique_ptr<RawElement> Root;
        TLanguage                   LanguageFilter { CNoSpecificLanguage };
        TFilterIndex                FilterIndex{ CNoParticularFilter };

        friend Builder;
    };
}