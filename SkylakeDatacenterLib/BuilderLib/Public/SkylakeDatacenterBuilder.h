//!
//! \file SkylakeDatacenterBuilder.h
//! 
//! \brief Skylake Datacenter builder abstractions and utilities
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

#include "SkylakeDatacenterAdapter.h"

namespace SKL::DC
{
    struct Builder;
    
    struct DatacenterBuilderExtension: public Datacenter<true>
    {
        using MyBase = Datacenter<true>;

    private:
        bool AllocateAttributes( std::vector<RawAttribute>& InRawAttributes, TBlockIndices& OutIndices ) noexcept;
        bool AllocateElementsSection( uint32_t InCount, TBlockIndices& OutIndices ) noexcept;
        bool AllocateAttributesSection( uint32_t InCount, TBlockIndices& OutIndices ) noexcept;

        MyBase::ElementsBlock& GetElementsContainerForNewElements( uint32_t InCount, TBlockIndex& OutIndex ) noexcept;
        MyBase::AttributesBlock& GetAttributesContainerForNewAttributes( uint32_t InCount, TBlockIndex& OutIndex ) noexcept;

        bool InsertName( const wchar_t* InString, uint32_t NameLengthInCharsNoNullTerminator, TNameIndex& OutIndex ) noexcept;
        bool InsertValue( const wchar_t* InString, uint32_t StringLengthInCharsNoNullTerminator, TBlockIndices& OutIndices ) noexcept;

        friend Builder;
    };

    struct Builder
    {
        using MyDatacenter = DatacenterBuilderExtension;

        SKL_FORCEINLINE DatacenterAdapter* GetAdapter() const noexcept { return Aadapter.get(); }
        SKL_FORCEINLINE void SetAdapter( DatacenterAdapter* InAdapter ) noexcept { Aadapter.reset( InAdapter ); }

        bool Build( TFilterIndex InFilterIndex, TLanguage InLanguage = CNoSpecificLanguage ) noexcept;

        void Reset() noexcept
        {
            DC.Clear();
        }

        SKL_FORCEINLINE MyDatacenter& GetDatacenter() noexcept { return DC; }
        SKL_FORCEINLINE const MyDatacenter& GetDatacenter() const noexcept { return DC; }

    private:
        bool BuildDCTree( RawElement* InElement ) noexcept;
        bool BuildDCTreeRecursive( RawElement* InElement, TBlockIndices InDCElementIndices ) noexcept;

        TVersion                           TargetVersion      { CInvalidVersion };
        TFormatVersion                     TargetFormatVersion{ CInvalidFormatVersion };
        TLanguage                          TargetLanguage     { CNoSpecificLanguage };
        std::unique_ptr<DatacenterAdapter> Aadapter           { nullptr };
        MyDatacenter                       DC                 {};
    };
}