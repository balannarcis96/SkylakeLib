//!
//! \file SkylakeDatacenterXMLAdapter.cpp
//! 
//! \brief Skylake Datacenter XML adapter abstractions and utilities
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#include "SkylakeDatacenterXMLAdapter.h"

#include <rapidxml.hpp>

namespace SKL::DC
{
    bool IsElementEligibleForLanguage( const DatacenterXMLAdapter* InAdaptor, ::rapidxml::xml_node<char>* InNode ) noexcept
    {
        SKL_ASSERT( nullptr != InNode );

        if( CNoSpecificLanguage == InAdaptor->GetCurrentLanguageFilter() )
        {
            return true;
        }

        ::rapidxml::xml_attribute<char> * Attribute{ InNode->first_attribute() };
        while( nullptr != Attribute )
        {
            if( true == InAdaptor->IsLanguageAttributeByName( Attribute->name() ) )
            {
                const auto* ValueStr{ Attribute->value() };
                SKL_ASSERT( nullptr != ValueStr );

                const auto Language{ InAdaptor->ParseLanguageFromUtf8String( ValueStr ) };

                if( InAdaptor->IsVerbose() )
                {
                    SKLL_LOG_FMT( "Found element with language attribute!\n\tElement:%s\n\tAttribute:%s=\"%s\"\n\tFilterLanguage:%s"
                                , InNode->name()
                                , Attribute->name()
                                , Attribute->value() 
                                , InAdaptor->GetLanguageString( InAdaptor->GetCurrentLanguageFilter() ) );
                }

                if( InAdaptor->GetCurrentLanguageFilter() != Language )
                {
                    return false;
                }

                break;
            }

            Attribute = Attribute->next_attribute();
        }
        
        return true;
    }

    std::unique_ptr<RawElement> ParseXmlFileNode( std::string_view FileName, DatacenterXMLAdapter* InAdaptor, RawElement* InParent, ::rapidxml::xml_node<char>* InNode ) noexcept
    {
        if( InNode->type() != rapidxml::node_element )
        {
            return nullptr;
        }

        if( true == InAdaptor->ShouldSkipElementByName( InNode->name() ) )
        {
            if( InAdaptor->IsVerbose() )
            {
                SKLL_LOG_FMT( "DatacenterXMLAddaptor: Skipped element BY NAME!\n\tFile:%s\n\tElementName:%s"
                            , FileName.data()
                            , InNode->name() );
            }

            return nullptr;
        }

        if( false == IsElementEligibleForLanguage( InAdaptor, InNode ) )
        {
            if( InAdaptor->IsVerbose() )
            {
                SKLL_LOG_FMT( "DatacenterXMLAddaptor: Skipped element BY LANGUAGE!\n\tFile:%s\n\tElementName:%s\n\tAcceptedLanguage:%s"
                            , FileName.data()
                            , InNode->name()
                            , InAdaptor->GetLanguageString( static_cast<TLanguage>( InAdaptor->GetCurrentLanguageFilter() ) ) );
            }

            return nullptr;
        }
        
        if( true == std::string_view{ InNode->name() }.empty() ) SKL_UNLIKELY
        {
            if( nullptr != InParent )
            {
                SKL_ASSERT( nullptr != InParent->GetName() );
				SKLL_TRACE_MSG_FMT( "Found element with no name as child of <%ws ...> </%ws>", InParent->GetName(), InParent->GetName() );
            }
            else
            {
				SKLL_TRACE_MSG( "Found element with no name!!" );
            }

            return nullptr;
        }

        const wchar_t* CleanElementName{ InAdaptor->CleanAndConvertToUtf16ElementName( InNode->name() ) };
        SKL_ASSERT( nullptr != CleanElementName );

        std::unique_ptr<RawElement> NewElement{ new RawElement() };
        SKL_ASSERT( nullptr != NewElement );
        NewElement->SetName( CleanElementName );
        NewElement->SetParent( InParent );

        if( 0 != InNode->value_size() && false == std::string_view{ InNode->value() }.empty() )
        {
            //This node has value string
            const auto* ValueString{ InAdaptor->ConvertUtf8ToUtf16( const_cast<const char*>( InNode->value() ), InNode->value_size() ) };
            if( nullptr == ValueString )
            {
				SKLL_TRACE_MSG_FMT( "Failed to convert utf8[<%s>%s</>] element value to utf16", InNode->name(), InNode->value() );
				return nullptr;
            }

            NewElement->SetValue( ValueString );
        }

        ::rapidxml::xml_attribute<char>* Attribute{ InNode->first_attribute() };
        while( nullptr != Attribute )
        {
            if( true == std::string_view{ Attribute->name() }.empty() )
            {
				SKLL_TRACE_MSG_FMT( "Found attibute with no name!! <%s ...></%s>", InNode->name(), InNode->name() );
				return nullptr;
            }

            if( false == InAdaptor->ShouldSkipAttributeByName( Attribute->name() ) )
            {
                RawAttribute NewAttribute{};

                const auto* Name{ InAdaptor->CleanAndConvertToUtf16AttributeName( Attribute->name() ) };
                if( nullptr == Name )
                {
            	    SKLL_TRACE_MSG_FMT( "Failed to convert utf8[<%s %s=\"%s\"></>] attribute name to utf16", InNode->name( ), Attribute->name(), Attribute->value() );
				    return nullptr;
                }
                NewAttribute.SetName( Name );
            
                if( false == std::string_view{ Attribute->value() }.empty() )
                {
                    const auto* Value{ InAdaptor->ConvertUtf8ToUtf16( Attribute->value(), Attribute->value_size() ) };
                    if( nullptr == Value )
                    {
            	        SKLL_TRACE_MSG_FMT( "Failed to convert utf8[<%s %s=\"%s\"></>] attribute value to utf16", InNode->name( ), Attribute->name(), Attribute->value() );
				        return nullptr;
                    }
                    NewAttribute.SetValue( Value );
                }
                else
                {
                    NewAttribute.SetValue( L"" );
                }

                NewElement->AddAttribute( std::move( NewAttribute ) );
            }

            Attribute = Attribute->next_attribute();
        }

        auto* ChildNode{ InNode->first_node() };
        while( nullptr != ChildNode )
        {
            auto ChildElement{ ParseXmlFileNode( FileName, InAdaptor, NewElement.get(), ChildNode ) };
            if( nullptr != ChildElement )
            {
                NewElement->AddChild( ChildElement.release() );
            }

            ChildNode = ChildNode->next_sibling();
        }

        NewElement->AddReference();
        return NewElement;
    }

    std::unique_ptr<RawElement> DatacenterXMLAdapter::BuildRawStructure() noexcept 
    {
        if( IsVerbose() )
        {
            SKLL_LOG_FMT( "Building the raw structure:\n\tLanguageFilter:%s\n\tFilterIndex:%d"
                       , GetLanguageString( GetCurrentLanguageFilter() )
                       , GetFilterIndex() );
        }

        size_t MaxFileSize     { 0 };
        auto   FilesInDirectory{ ScanForFilesInDirectory( GetTargetDirectory(), MaxFileSize, AcceptedFileExtensions ) };
        if( true == FilesInDirectory.empty() )
        {
            SKLL_TRACE_MSG_FMT( "Could not find any files in the given directory and with the given extensions! Dir[%s]!", GetTargetDirectory() );
            return nullptr;
        }

        std::vector<std::unique_ptr<RawElement>> AllElements;
        AllElements.reserve( 1024 );

        //const size_t DirectoryNameLength{ strlen( GetTargetDirectory() ) };
        for( auto& FileName: FilesInDirectory )
        {
            std::ifstream File{ FileName.c_str(), std::ios::binary };
            if( false == File.is_open() )
            {
                SKLL_TRACE_MSG_FMT( "Failed to open file[%s]!", FileName.c_str() );
                return nullptr;
            }

            File.seekg( 0, std::ios::end );
            const size_t FileSize{ static_cast<size_t>( File.tellg() ) };
            File.seekg( 0, std::ios::beg );

            if( 0 == FileSize )
            {
                File.close();
                SKLL_TRACE_MSG_FMT( "Skipping empty file[%s]!", FileName.c_str() );
                continue;
            }

            auto Buffer = std::make_unique<uint8_t[]>( FileSize + 64 );
            if( nullptr == Buffer )
            {
                File.close();
                SKLL_TRACE_MSG_FMT( "Failed to allocate buffer! size:%llu file[%s]!", FileSize, FileName.c_str() );
                return nullptr;
            }

            File.read( reinterpret_cast<char*>( Buffer.get() ), FileSize );

            if( true == File.bad() )
            {
                File.close();
                SKLL_TRACE_MSG_FMT( "Failed to read file[%s]!", FileName.c_str() );
                return nullptr;
            }

            Buffer[FileSize] = '\0';

            File.close();

            auto XmlDoc{ std::make_unique<::rapidxml::xml_document<char>>() };
            if( nullptr == XmlDoc )
            {
                SKLL_TRACE_MSG( "Failed to allocate xmlDoc object !" );
                return nullptr;
            }

            XmlDoc->parse<0>( reinterpret_cast<char*>( Buffer.get() ) );
            
			if ( nullptr == XmlDoc->first_node( ) )
			{
                SKLL_TRACE_MSG( "Failed to allocate xmlDoc object !" );
				return nullptr;
			}

            if( true == ShouldSkipElementByName( { XmlDoc->first_node()->name() } ) )
            {
                if( IsVerbose() )
                {
                    SKLL_LOG_FMT( "DatacenterXMLAddaptor: Skipped element BY NAME!\n\tFile:%s\n\tElementName:%s"
                                , FileName.data()
                                , XmlDoc->first_node()->name() );
                }

                continue;
            }

            if( false == IsElementEligibleForLanguage( this, XmlDoc->first_node() ) )
            {
                if( IsVerbose() )
                {
                    SKLL_LOG_FMT( "DatacenterXMLAddaptor: Skipped element BY LANGUAGE!\n\tFile:%s\n\tElementName:%s\n\tAcceptedLanguage:%s"
                                , FileName.data()
                                , XmlDoc->first_node()->name()
                                , GetLanguageString( static_cast<TLanguage>( GetCurrentLanguageFilter() ) ) );
                }
                continue;
            }

            auto FileRootNode{ ParseXmlFileNode( FileName, this, nullptr, XmlDoc->first_node() ) };
            if( nullptr == FileRootNode )
            {
                SKLL_TRACE_MSG_FMT( "Failed to parse xml file %s!", FileName.c_str() );
				return nullptr;
            }

            AllElements.push_back( std::move( FileRootNode ) );
        }

        auto RootNode{ std::make_unique<RawElement>() };
        SKL_ASSERT( nullptr != RootNode );
        RootNode->SetName( GetRootNodeName() );

        for( auto& Element: AllElements )
        {
            if( nullptr == Element ){ continue; }
            RootNode->AddChild( Element.release() );
        }

        return RootNode;
    }
}