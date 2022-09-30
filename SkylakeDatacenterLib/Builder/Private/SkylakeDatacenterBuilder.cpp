//!
//! \file SkylakeDatacenterBuilder.cpp
//! 
//! \brief Skylake Datacenter builder abstractions and utilities
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#include "SkylakeDatacenterBuilder.h"

namespace SKL::DC
{
    DatacenterBuilderExtension::MyBase::ElementsBlock& DatacenterBuilderExtension::GetElementsContainerForNewElements( uint32_t InCount, TBlockIndex& OutIndex ) noexcept
    {
        SKL_ASSERT( InCount <= CElementsBlockSize );
        
        for( uint32_t i = 0; i < this->Elements.Size(); ++i )
        {
            if( this->Elements[i].CanFit( InCount ) )
            {
                OutIndex = static_cast<TBlockIndex>( i );
                return this->Elements[i];
            }
        }

        MyBase::ElementsBlock NewBlock{};
        NewBlock.SetMaxSize( CElementsBlockSize );
        NewBlock.Reserve( CElementsBlockSize );

        this->Elements.AddItem( std::move( NewBlock ) );
        OutIndex = static_cast<TBlockIndex>( this->Elements.Size() - 1 );

        return this->Elements.Last();
    }
    DatacenterBuilderExtension::MyBase::AttributesBlock& DatacenterBuilderExtension::GetAttributesContainerForNewAttributes( uint32_t InCount, TBlockIndex& OutIndex ) noexcept
    {
        SKL_ASSERT( InCount <= CAttributesBlockSize );
        
        for( uint32_t i = 0; i < this->Attributes.Size(); ++i )
        {
            if( this->Attributes[i].CanFit( InCount ) )
            {
                OutIndex = static_cast<TBlockIndex>( i );
                return this->Attributes[i];
            }
        }

        MyBase::AttributesBlock NewBlock{};
        NewBlock.SetMaxSize( CAttributesBlockSize );
        NewBlock.Reserve( CAttributesBlockSize );

        this->Attributes.AddItem( std::move( NewBlock ) );
        OutIndex = static_cast<TBlockIndex>( this->Attributes.Size() - 1 );

        return this->Attributes.Last();
    }

    bool DatacenterBuilderExtension::AllocateAttributes( std::vector<RawAttribute>& InRawAttributes, TBlockIndices& OutIndices ) noexcept
    {
        SKL_ASSERT( false == InRawAttributes.empty() );

        auto& Block{ GetAttributesContainerForNewAttributes( static_cast<uint32_t>( InRawAttributes.size() ), OutIndices.first ) };
        
        // Start of this attributes block
        OutIndices.second = static_cast<TBlockIndex>( Block.Size() );

        for( auto& RawAttribute : InRawAttributes )
        {
            MyBase::Attribute NewAttribute{};

            // set the location cache
            NewAttribute.GetEditData().CachedLocation = { OutIndices.first, static_cast<TBlockIndex>( Block.Size() ) };

            // insert name string
            TNameIndex NameIndex{ CInvalidStringIndex };
            if( false == this->NamesMap.InsertString( RawAttribute.GetName(),RawAttribute.GetNameSize(), NameIndex ) )
            {
                SKLL_TRACE_MSG_FMT( "Failed to insert attribute name into names map! str[%ws]", RawAttribute.GetName() );
                return false;
            }
            NewAttribute.SetNameIndex( NameIndex );

            // insert value string
            TBlockIndices ValueIndices{ CInvalidBlockIndex, CInvalidBlockIndex };
            if( false == this->NamesMap.InsertString( RawAttribute.GetValue(),RawAttribute.GetValueSize(), ValueIndices ) )
            {
                SKLL_TRACE_MSG_FMT( "Failed to insert attribute value into values map! %ws=\"%ws\"", RawAttribute.GetName(), RawAttribute.GetValue() );
                return false;
            }
            NewAttribute.SetValueIndices( ValueIndices );

            Block.AddItem( std::move( NewAttribute ) );
        }

        return true;
    }
    bool DatacenterBuilderExtension::AllocateElementsSection( uint32_t InCount, TBlockIndices& OutIndices ) noexcept
    {
        SKL_ASSERT( 0 != InCount );

        auto& Block{ GetElementsContainerForNewElements( InCount, OutIndices.first ) };
        OutIndices.second = static_cast<TBlockIndex>( Block.Size() );

        for( uint32_t i = 0; i < InCount; ++i )
        {
            MyBase::Element NewEmptyElement{};
            NewEmptyElement.GetEditData().CachedLocation = { OutIndices.first, static_cast<TBlockIndex>( Block.Size() ) };
            Block.AddItem( std::move( NewEmptyElement ) );
        }

        return true;
    }
    bool DatacenterBuilderExtension::AllocateAttributesSection( uint32_t InCount, TBlockIndices& OutIndices ) noexcept
    {
        SKL_ASSERT( 0 != InCount );

        auto& Block{ GetAttributesContainerForNewAttributes( InCount, OutIndices.first ) };
        OutIndices.second = static_cast<TBlockIndex>( Block.Size() );

        for( uint32_t i = 0; i < InCount; ++i )
        {
            MyBase::Attribute NewEmptyAttribute{};
            NewEmptyAttribute.GetEditData().CachedLocation = { OutIndices.first, static_cast<TBlockIndex>( Block.Size() ) };
            Block.AddItem( std::move( NewEmptyAttribute ) );
        }

        return true;
    }

    bool DatacenterBuilderExtension::InsertName( const wchar_t* InString, uint32_t NameLengthInCharsNoNullTerminator, TNameIndex& OutIndex ) noexcept
    {
        return this->NamesMap.InsertString( InString, NameLengthInCharsNoNullTerminator, OutIndex );
    }
    bool DatacenterBuilderExtension::InsertValue( const wchar_t* InString, uint32_t StringLengthInCharsNoNullTerminator, TBlockIndices& OutIndices ) noexcept
    {
        return this->ValuesMap.InsertString( InString, StringLengthInCharsNoNullTerminator, OutIndices );
    }

    bool Builder::Build( TFilterIndex InFilterIndex, TLanguage InLanguage ) noexcept
    {
        if( CInvalidVersion == TargetVersion || CInvalidFormatVersion == TargetFormatVersion )
        {
            SKLL_TRACE_MSG_FMT( "Invalid version or format version value!" );
            return false;
        }

        DatacenterAdapter* Adapter{ GetAdapter() };
        if( nullptr == Adapter )
        {
            SKLL_TRACE_MSG_FMT( "No adapter set!" );
            return false;
        }

        Adapter->SetFilterIndex( InFilterIndex );
        Adapter->SetCurrentLanguageFilter( InLanguage );
        
        std::unique_ptr<RawElement> RawStructureRootRawElement{ Adapter->BuildRawStructure() };
        if( nullptr == RawStructureRootRawElement )
        {
            SKLL_TRACE_MSG_FMT( "Failed to build raw structure!" );
            return false;
        }

        Reset();

        DC.SetVersion( TargetVersion );
        DC.SetFormatVersion( TargetFormatVersion );
        DC.SetLanguage( InLanguage );

        return BuildDCTree( RawStructureRootRawElement.get() );
    }

    bool Builder::BuildDCTree( RawElement* InElement ) noexcept
    {
        TBlockIndices RootIndices{ CInvalidBlockIndex, CInvalidBlockIndex };
        if( false == DC.AllocateElementsSection( 1, RootIndices ) )
        {
            SKLL_TRACE_MSG( "Failed to allocate root elemnt!" );
            return false;
        }

        return BuildDCTreeRecursive( InElement, RootIndices );
    }
    
    bool Builder::BuildDCTreeRecursive( RawElement* InRawElement, TBlockIndices InDCElementIndices ) noexcept
    {
        MyDatacenter::Element* Element{ DC.GetElement( InDCElementIndices ) };

        Element->SetAttributesCount( static_cast<uint32_t>( InRawElement->Attributes.size() ) );
        Element->SetChildrenCount( static_cast<uint32_t>( InRawElement->Children.size() ) );
        Element->GetEditData().CachedLocation = InDCElementIndices;

        // insert element name string
        TNameIndex NameIndex{ CInvalidStringIndex };
        if( false == DC.InsertName( InRawElement->GetName(), InRawElement->GetNameSize(), NameIndex ) )
        {
            SKLL_TRACE_MSG_FMT( "Failed to insert element name into the names map. Name:%ws", InRawElement->GetName() );
            return false;
        }
        Element->SetNameIndex( NameIndex );

        if( 0 != InRawElement->GetValueSize() )
        {
            // insert element value string
            TBlockIndices ValueIndices{ CInvalidBlockIndex, CInvalidBlockIndex };
            if( false == DC.InsertValue( InRawElement->GetValue(), InRawElement->GetValueSize(), ValueIndices ) )
            {
                SKLL_TRACE_MSG_FMT( "Failed to insert element value into the values map. <%ws ...></>", InRawElement->GetName() );
                return false;
            }
            Element->SetValueIndices( ValueIndices );
            InRawElement->CachedValueIndices =  ValueIndices;
        }

        // cache back on the raw element 
        InRawElement->CachedNameIndex = NameIndex;
        InRawElement->CachedMyIndices = InDCElementIndices;

        // insert element attributes
        if( false == InRawElement->Attributes.empty() )
        {
            TBlockIndices AttributesIndices{ CInvalidBlockIndex, CInvalidBlockIndex };
            if( false == DC.AllocateAttributesSection( static_cast<uint32_t>( InRawElement->Attributes.size() ), AttributesIndices ) )
            {
                SKLL_TRACE_MSG_FMT( "Failed to allocate attributes block for element! count:%llu Name:%ws", InRawElement->Attributes.size(), InRawElement->GetName() );
                return false;
            }
            Element->SetAttributesIndices( AttributesIndices );

            for( uint32_t i = 0; i < static_cast<uint32_t>( InRawElement->Attributes.size() ); ++i )
            {
                auto&                    RawAttribute{ InRawElement->Attributes[i] };
                MyDatacenter::Attribute* DCAttribute { DC.GetAttribute( { AttributesIndices.first, static_cast<TBlockIndex>( i + AttributesIndices.second ) } ) };

                TNameIndex NameIndex{ CInvalidStringIndex };
                if( false == DC.InsertName( RawAttribute.GetName(), RawAttribute.GetNameSize(), NameIndex ) )
                {
                    SKLL_TRACE_MSG_FMT( "Failed to insert attribute name into the names map. <%ws %ws=\"%ws\"></>", InRawElement->GetName(), RawAttribute.GetName(), RawAttribute.GetValue() );
                    return false;
                }
                DCAttribute->SetNameIndex( NameIndex );

                TBlockIndices ValueIndices{ CInvalidBlockIndex, CInvalidBlockIndex };
                if( false == DC.InsertValue( RawAttribute.GetValue(), RawAttribute.GetValueSize(), ValueIndices ) )
                {
                    SKLL_TRACE_MSG_FMT( "Failed to insert attribute value into the values map. <%ws %ws=\"%ws\"></>", InRawElement->GetName(), RawAttribute.GetName(), RawAttribute.GetValue() );
                    return false;
                }
                DCAttribute->SetValueIndices( ValueIndices );

                RawAttribute.CachedNameIndex    = NameIndex;
                RawAttribute.CachedValueIndices = ValueIndices;
                RawAttribute.CachedMyLocation   = DCAttribute->GetEditData().CachedLocation;
            }
        }
  
        if( true == InRawElement->Children.empty() )
        {
            return true;
        }

        // insert element children recursively
        TBlockIndices ChildrenIndices{ CInvalidBlockIndex, CInvalidBlockIndex };
        if( false == DC.AllocateElementsSection( static_cast<uint32_t>( InRawElement->Children.size() ), ChildrenIndices ) )
        {
            SKLL_TRACE_MSG_FMT( "Failed to allocate child elements!. <%ws ...></>", InRawElement->GetName() );
            return false;
        }
        DC.GetElement( InDCElementIndices )->SetChildrenIndices( ChildrenIndices );
        for( uint32_t i = 0; i < static_cast<uint32_t>( InRawElement->Children.size() ); ++i )
        {
            if( false == BuildDCTreeRecursive( InRawElement->Children[i], { ChildrenIndices.first, static_cast<TBlockIndex>( i + ChildrenIndices.second ) } ) )
            {
                SKLL_TRACE_MSG_FMT( "Failed to build child element index %u!. <%ws ...></>", i, InRawElement->GetName() );
                return false;
            }
        }

        return true;
    }
}