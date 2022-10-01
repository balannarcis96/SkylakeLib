#include <gtest/gtest.h>

#include <SkylakeLib.h>

#include <SkylakeDatacenterBuilder.h>
#include <SkylakeDatacenterXMLAdapter.h>

namespace DatacenterTests
{
    class DatacenterTestsFixture : public testing::Test
    {
    public:
        using Datacenter = SKL::DC::Datacenter<false>;
        using BuildDatacenter = SKL::DC::Datacenter<true>;
        using TestBlockArray = Datacenter::BlockArray<Datacenter::Attribute, static_cast<uint32_t>( Datacenter::GetAttributeSerialSize() )>;

        DatacenterTestsFixture() 
            : testing::Test() {}

        void SetUp() override
        {

        }
        void TearDown() override
        {

        }

        Datacenter::Attribute Attribute_APIDummy() const noexcept
        {
            Datacenter::Attribute Attr;
            Attr.NameIndex = 23;
            Attr.Value     = { 1 , 2 };
            return Attr;
        }
        void Attribute_APIDummy_Validate( const Datacenter::Attribute& InDummyAttr ) const noexcept
        {
            ASSERT_TRUE( 23 == InDummyAttr.NameIndex );    
            ASSERT_TRUE( 1 == InDummyAttr.Value.first );    
            ASSERT_TRUE( 2 == InDummyAttr.Value.second );    
        }
        
        void Attribute_ValueAPIDummy_Validate() const noexcept
        {
            {
                Datacenter::Attribute Attr;
                Attr.CachedValueRef = L"2147483647";
                ASSERT_TRUE( std::numeric_limits<int32_t>::max() == Attr.GetInt() );    
                ASSERT_TRUE( std::numeric_limits<int32_t>::max() == Attr.GetInt64() );    
                ASSERT_TRUE( std::numeric_limits<int32_t>::max() == Attr.GetUInt64() );    
            }
            
            {
                Datacenter::Attribute Attr;
                Attr.CachedValueRef = L"7FFFFFFF";
                ASSERT_TRUE( std::numeric_limits<int32_t>::max() == Attr.GetInt( 16 ) );  
                ASSERT_TRUE( std::numeric_limits<int32_t>::max() == Attr.GetInt64( 16 ) );    
                ASSERT_TRUE( std::numeric_limits<int32_t>::max() == Attr.GetUInt64( 16 ) );    
            }

            {
                Datacenter::Attribute Attr;
                Attr.CachedValueRef = L"4294967295";
                ASSERT_TRUE( std::numeric_limits<uint32_t>::max() == Attr.GetUInt() );    
                ASSERT_TRUE( std::numeric_limits<uint32_t>::max() == Attr.GetInt64() );    
                ASSERT_TRUE( std::numeric_limits<uint32_t>::max() == Attr.GetUInt64() );    
            }
            
            {
                Datacenter::Attribute Attr;
                Attr.CachedValueRef = L"FFFFFFFF";
                ASSERT_TRUE( std::numeric_limits<uint32_t>::max() == Attr.GetUInt( 16 ) );  
                ASSERT_TRUE( std::numeric_limits<uint32_t>::max() == Attr.GetInt64( 16 ) );    
                ASSERT_TRUE( std::numeric_limits<uint32_t>::max() == Attr.GetUInt64( 16 ) );    
            }

            {
                Datacenter::Attribute Attr;
                Attr.CachedValueRef = L"23.555";
                ASSERT_TRUE( true == SKL::FIsNearlyEqual( 23.555f, Attr.GetFloat(), 0.005f ) );    
            }
            
            {
                Datacenter::Attribute Attr;
                Attr.CachedValueRef = L"23.5555";
                ASSERT_TRUE( true == SKL::FIsNearlyEqual( 23.555, Attr.GetDouble(), 0.005 ) );    
            }
            
            {
                Datacenter::Attribute Attr;
                Attr.CachedValueRef = L"23.555, 23.555";

                float Point2D[2]{ 0.0f, 0.0f };
                ASSERT_TRUE( true == Attr.Get2DPoint( Point2D ) );

                ASSERT_TRUE( true == SKL::FIsNearlyEqual( 23.555f, Point2D[0], 0.005f ) );    
                ASSERT_TRUE( true == SKL::FIsNearlyEqual( 23.555f, Point2D[1], 0.005f ) );    
            }
            
            {
                Datacenter::Attribute Attr;
                Attr.CachedValueRef = L"23.555, 23.555";

                double Point2D[2]{ 0.0f, 0.0f };
                ASSERT_TRUE( true == Attr.Get2DPointD( Point2D ) );

                ASSERT_TRUE( true == SKL::FIsNearlyEqual( 23.555, Point2D[0], 0.005 ) );    
                ASSERT_TRUE( true == SKL::FIsNearlyEqual( 23.555, Point2D[1], 0.005 ) );    
            }

            {
                Datacenter::Attribute Attr;
                Attr.CachedValueRef = L"23.555, 23.555, 23.555";

                float Point3D[3]{ 0.0f, 0.0f, 0.0f };
                ASSERT_TRUE( true == Attr.Get3DPoint( Point3D ) );

                ASSERT_TRUE( true == SKL::FIsNearlyEqual( 23.555f, Point3D[0], 0.005f ) );    
                ASSERT_TRUE( true == SKL::FIsNearlyEqual( 23.555f, Point3D[1], 0.005f ) );    
                ASSERT_TRUE( true == SKL::FIsNearlyEqual( 23.555f, Point3D[2], 0.005f ) );    
            }
            
            {
                Datacenter::Attribute Attr;
                Attr.CachedValueRef = L"23.555, 23.555, 23.555";

                double Point3D[3]{ 0.0, 0.0, 0.0 };
                ASSERT_TRUE( true == Attr.Get3DPointD( Point3D ) );

                ASSERT_TRUE( true == SKL::FIsNearlyEqual( 23.555, Point3D[0], 0.005 ) );    
                ASSERT_TRUE( true == SKL::FIsNearlyEqual( 23.555, Point3D[1], 0.005 ) );    
                ASSERT_TRUE( true == SKL::FIsNearlyEqual( 23.555, Point3D[2], 0.005 ) );    
            }
        }
        
        Datacenter::Element Element_APIDummy() const noexcept
        {
            Datacenter::Element Element;
            Element.NameIndex        = 23;
            Element.ChildrenCount    = 5;
            Element.ChildrenIndices  = { 1 , 2 };
            Element.AttributesCount  = 6;
            Element.AttributeIndices = { 3 , 4 };
            return Element;
        }
        void Element_APIDummy_Validate( const Datacenter::Element& InDummyElement ) const noexcept
        {
            ASSERT_TRUE( 23 == InDummyElement.NameIndex );    
            ASSERT_TRUE( 5 == InDummyElement.ChildrenCount );    
            ASSERT_TRUE( 1 == InDummyElement.ChildrenIndices.first );    
            ASSERT_TRUE( 2 == InDummyElement.ChildrenIndices.second );   
            ASSERT_TRUE( 6 == InDummyElement.AttributesCount );    
            ASSERT_TRUE( 3 == InDummyElement.AttributeIndices.first );    
            ASSERT_TRUE( 4 == InDummyElement.AttributeIndices.second );    
        }

        Datacenter::Array<Datacenter::Attribute> Array_APIDummy() const noexcept
        {
            Datacenter::Array<Datacenter::Attribute> Array;

            {
                auto Attr{ Attribute_APIDummy() };
                Array.AddItem( std::move( Attr ) ) ;
            }
            
            {
                auto Attr{ Attribute_APIDummy() };
                Array.AddItem( std::move( Attr ) ) ;
            }

            {
                auto Attr{ Attribute_APIDummy() };
                Array.AddItem( std::move( Attr ) ) ;
            }

            return Array;
        }
        void Array_APIDummy_Validate( const Datacenter::Array<Datacenter::Attribute>& InDummyArray ) const noexcept
        {
            ASSERT_TRUE( 3 == InDummyArray.Count );
            ASSERT_TRUE( 3 == InDummyArray.Data.size() );

            for( uint32_t i = 0 ;i < 3; ++i )
            {
                const auto& Item{ InDummyArray[i] };
                ASSERT_TRUE( 23 == Item.NameIndex );    
                ASSERT_TRUE( 1 == Item.Value.first );    
                ASSERT_TRUE( 2 == Item.Value.second );  
            }
        }
    
        TestBlockArray BlockArray_APIDummy() const noexcept
        {
            TestBlockArray Array;
            Array.TotalBlockCount = 64;

            {
                auto Attr{ Attribute_APIDummy() };
                Array.AddItem( std::move( Attr ) ) ;
            }
            
            {
                auto Attr{ Attribute_APIDummy() };
                Array.AddItem( std::move( Attr ) ) ;
            }

            {
                auto Attr{ Attribute_APIDummy() };
                Array.AddItem( std::move( Attr ) ) ;
            }

            return Array;
        }
        void BlockArray_APIDummy_Validate( const TestBlockArray& InDummyArray ) const noexcept
        {
            ASSERT_TRUE( 64 == InDummyArray.TotalBlockCount );
            ASSERT_TRUE( 3 == InDummyArray.TotalUsedBlockCount );
            ASSERT_TRUE( 3 == InDummyArray.Data.size() );

            for( uint32_t i = 0 ;i < 3; ++i )
            {
                const auto& Item{ InDummyArray[i] };
                ASSERT_TRUE( 23 == Item.NameIndex );    
                ASSERT_TRUE( 1 == Item.Value.first );    
                ASSERT_TRUE( 2 == Item.Value.second );  
            }
        }
        
        Datacenter::StringBlock StringBlock_APIDummy() const noexcept
        {
            Datacenter::StringBlock Block;
          
            EXPECT_TRUE( true == Block.AllocateBlock() );

            {
                SKL::DC::TBlockIndex Index{ 0 };
                EXPECT_TRUE( nullptr != Block.TryAddString( L"ASD", 3, Index ) );
                EXPECT_TRUE( 0 == Index );
            }
            
            {
                SKL::DC::TBlockIndex Index{ 0 };
                EXPECT_TRUE( nullptr != Block.TryAddString( L"ASD", 3, Index ) );
                EXPECT_TRUE( 4 == Index );
            }

            EXPECT_TRUE( 8 == Block.BlockUsedSize );

            return Block;
        }
        void StringBlock_APIDummy_Validate( const Datacenter::StringBlock& InDummyStringBlock ) const noexcept
        {
            ASSERT_TRUE( SKL::DC::CStringsBlockSize == InDummyStringBlock.BlockTotalSize );
            ASSERT_TRUE( 8 == InDummyStringBlock.BlockUsedSize );

            ASSERT_TRUE( 0 == wcscmp( L"ASD", InDummyStringBlock.GetString( 0 ) ) );
            ASSERT_TRUE( 0 == wcscmp( L"ASD", InDummyStringBlock.GetString( 4 ) ) );
        }
    
        Datacenter::StringEntry StringEntry_APIDummy() const noexcept
        {
            Datacenter::StringEntry StringEntry;
            StringEntry.Indices = { 1 , 2 };
            return StringEntry;
        }
        void StringEntry_APIDummy_Validate( const Datacenter::StringEntry& InDummyStringEntry ) const noexcept
        {
            ASSERT_TRUE( 1 == InDummyStringEntry.Indices.first );    
            ASSERT_TRUE( 2 == InDummyStringEntry.Indices.second );    
        }
        
        BuildDatacenter::StringMap StringMap_APIDummy() const noexcept
        {
            BuildDatacenter::StringMap StringMap;

            SKL::DC::TStringIndex Index{ SKL::DC::CInvalidStringIndex };
            EXPECT_TRUE( true == StringMap.InsertString( L"ASB", 3, Index ) );
            
            EXPECT_TRUE( 0 == Index );

            SKL::DC::TBlockIndices Indices{ SKL::DC::CInvalidBlockIndex, SKL::DC::CInvalidBlockIndex };
            EXPECT_TRUE( true == StringMap.InsertString( L"ASD", 3, Indices ) );

            EXPECT_TRUE( 0 == Indices.first );
            EXPECT_TRUE( 4 == Indices.second );

            return StringMap;
        }
        void StringMap_APIDummy_Validate( const BuildDatacenter::StringMap& InDummyStringMap ) const noexcept
        {
            ASSERT_TRUE( 1 == InDummyStringMap.StringBlocks.Size() );    
            ASSERT_TRUE( 2 == InDummyStringMap.AllStrings.Size() );    
            ASSERT_TRUE( 0 == wcscmp( L"ASB", InDummyStringMap.GetString( 0, 0 ) ) );
            ASSERT_TRUE( 0 == wcscmp( L"ASD", InDummyStringMap.GetString( 0, 4 ) ) );
        }
    };

    struct TestDatacenterAdapter: public SKL::DC::DatacenterXMLAdapter
    {
        void SetIsForClientOrServer( bool bIsForClientOrServer ) noexcept { SetFilterIndex( bIsForClientOrServer ? 0 : 1 ); }
        bool IsForClientOrServer() const noexcept { return GetFilterIndex() == 0; }

        std::vector<std::string> ScanForFilesInDirectory( const char* InRootDirectory, size_t& OutMaxFileSize, const std::vector<std::string>& InEtensions ) noexcept override
        {
            return SKL::ScanForFilesInDirectory( InRootDirectory, OutMaxFileSize, InEtensions );
        }

        const wchar_t* ConvertUtf8ToUtf16( const char* InStr, size_t InStringLength ) noexcept override
        {
            if( true == SKL::GMultiByteToWideChar( InStr, InStringLength, Utf16Buffer.get(), CBuffersLength ) )
            {
                return Utf16Buffer.get();
            }

            return nullptr;
        }
        
        const char* ConvertUtf16ToUtf8( const wchar_t* InStr, size_t InStringLengthInWChars ) noexcept override
        {
            if( true == SKL::GWideCharToMultiByte( InStr, InStringLengthInWChars, Utf8Buffer.get(), CBuffersLength ) )
            {
                return Utf8Buffer.get();
            }

            return nullptr;
        }

        SKL::DC::TLanguage ParseLanguageFromUtf8String( const char* InLanguageStr ) const noexcept override
        {
            if( 0 == SKL_STRICMP( "INT", InLanguageStr, 3 ) )
            {
                return SKL::DC::CInternationalLanguage;
            }

            SKLL_TRACE_MSG_FMT( "Unknown language %s", InLanguageStr );

            return SKL::DC::CInvalidLanguage;
        }

        const char* GetLanguageString( SKL::DC::TLanguage InLanguage ) const noexcept override
        {
            if( SKL::DC::CInternationalLanguage == InLanguage )
            {
                return "INT";
            }

            return nullptr;
        }

        bool ShouldSkipAttributeByName( const std::string_view& InString ) const noexcept override
        {
            if( true == IsForClientOrServer() )
            {
                return InString.starts_with( "_" );
            }
            else
            {
                return InString.ends_with( "_" );
            }
        }

        bool ShouldSkipElementByName( const std::string_view& InString ) const noexcept override
        {
            if( true == IsForClientOrServer() )
            {
                return InString.starts_with( "_" );
            }
            else
            {
                return InString.ends_with( "_" );
            }
        }

        const wchar_t* CleanAndConvertToUtf16ElementName( const std::string_view& InString ) noexcept override
        {
            auto Utf8Buffer{ GetUtf8Buffer() };
            SKL_ASSERT( InString.length() < Utf8Buffer.second );
            InString.copy( Utf8Buffer.first, InString.length() );
            Utf8Buffer.first[ InString.length() ] = '\0';

            char* InStrPtr{ Utf8Buffer.first };

            if( true == IsForClientOrServer() )
            {
                if( true == InString.starts_with( '_' ) )
                {
                    ++InStrPtr;
                }
            }
            else
            {
                if( true == InString.ends_with( '_' ) )
                {
                    InStrPtr[InString.length() - 1] = '\0';
                }
            }

            return ConvertUtf8ToUtf16( InStrPtr, Utf8Buffer.second );
        }

        const wchar_t* CleanAndConvertToUtf16AttributeName( const std::string_view& InString ) noexcept override
        {
            auto Utf8Buffer{ GetUtf8Buffer() };
            SKL_ASSERT( InString.length() < Utf8Buffer.second );
            InString.copy( Utf8Buffer.first, InString.length() );
            Utf8Buffer.first[ InString.length() ] = '\0';

            char* InStrPtr{ Utf8Buffer.first };

            if( true == IsForClientOrServer() )
            {
                if( true == InString.starts_with( '_' ) )
                {
                    ++InStrPtr;
                }
            }
            else
            {
                if( true == InString.ends_with( '_' ) )
                {
                    InStrPtr[InString.length() - 1] = '\0';
                }
            }

            return ConvertUtf8ToUtf16( InStrPtr, Utf8Buffer.second );
        }
    };

    TEST_F( DatacenterTestsFixture, Attribute_API )
    {
        SKL::BufferStream Stream{ 4096 };

        {
            auto DummyAttr{ Attribute_APIDummy() };
            ASSERT_TRUE( true == DummyAttr.Serialize( Stream.GetStreamBase(), false ) );
            ASSERT_TRUE( Datacenter::GetAttributeSerialSize() == Stream.GetPosition() );
        }

        {
            Stream.SetPosition( 0 );

            Datacenter::Attribute Attr;
            ASSERT_TRUE( true == Attr.Serialize( Stream.GetStreamBase(), true ) );
            ASSERT_TRUE( Datacenter::GetAttributeSerialSize() == Stream.GetPosition() );
            Attribute_APIDummy_Validate( Attr );
        }
    }
    
    TEST_F( DatacenterTestsFixture, Element_API )
    {
        SKL::BufferStream Stream{ 4096 };

        {
            auto DummyElement{ Element_APIDummy() };
            ASSERT_TRUE( true == DummyElement.Serialize( Stream.GetStreamBase(), false ) );
            ASSERT_TRUE( Datacenter::GetElementSerialSize() == Stream.GetPosition() );
        }

        {
            Stream.SetPosition( 0 );

            Datacenter::Element Element;
            ASSERT_TRUE( true == Element.Serialize( Stream.GetStreamBase(), true ) );
            ASSERT_TRUE( Datacenter::GetElementSerialSize() == Stream.GetPosition() );
            Element_APIDummy_Validate( Element );
        }
    }
    
    TEST_F( DatacenterTestsFixture, Array_API )
    {
        SKL::BufferStream Stream{ 4096 };

        {
            auto DummyArray{ Array_APIDummy() };
            ASSERT_TRUE( true == DummyArray.Serialize( Stream.GetStreamBase(), false ) );
            ASSERT_TRUE( sizeof( uint32_t ) + ( 3 * Datacenter::GetAttributeSerialSize() ) == Stream.GetPosition() );
        }

        {
            Stream.SetPosition( 0 );

            Datacenter::Array<Datacenter::Attribute> Array;
            ASSERT_TRUE( true == Array.Serialize( Stream.GetStreamBase(), true ) );
            ASSERT_TRUE( sizeof( uint32_t ) + ( 3 * Datacenter::GetAttributeSerialSize() ) == Stream.GetPosition() );
            Array_APIDummy_Validate( Array );
        }
    }
    
    TEST_F( DatacenterTestsFixture, BlockArray_API )
    {
        SKL::BufferStream Stream{ 4096 };

        {
            auto DummyArray{ BlockArray_APIDummy() };
            ASSERT_TRUE( true == DummyArray.Serialize( Stream.GetStreamBase(), false ) );
            ASSERT_TRUE( ( sizeof( uint32_t ) * 2 ) + ( 64 * Datacenter::GetAttributeSerialSize() ) == Stream.GetPosition() );
        }

        {
            Stream.SetPosition( 0 );

            TestBlockArray Array;
            ASSERT_TRUE( true == Array.Serialize( Stream.GetStreamBase(), true ) );
            ASSERT_TRUE( ( sizeof( uint32_t ) * 2 ) + ( 64 * Datacenter::GetAttributeSerialSize() ) == Stream.GetPosition() );
            BlockArray_APIDummy_Validate( Array );
        }
    }

    TEST_F( DatacenterTestsFixture, StringBlock_API )
    {
        SKL::BufferStream Stream{ 4096 * 1024 };

        {
            auto DummyStringBlock{ StringBlock_APIDummy() };
            ASSERT_TRUE( true == DummyStringBlock.Serialize( Stream.GetStreamBase(), false ) );
            ASSERT_TRUE( ( sizeof( uint16_t ) * 2 ) + ( sizeof( wchar_t ) * SKL::DC::CStringsBlockSize ) == Stream.GetPosition() );
        }

        {
            Stream.SetPosition( 0 );

            Datacenter::StringBlock StringBlock;
            ASSERT_TRUE( true == StringBlock.Serialize( Stream.GetStreamBase(), true ) );
            ASSERT_TRUE( ( sizeof( uint16_t ) * 2 ) + ( sizeof( wchar_t ) * SKL::DC::CStringsBlockSize ) == Stream.GetPosition() );
            StringBlock_APIDummy_Validate( StringBlock );
        }
    }
    
    TEST_F( DatacenterTestsFixture, StringEntry_API )
    {
        SKL::BufferStream Stream{ 4096 };

        {
            auto DummyStringEntry{ StringEntry_APIDummy() };
            ASSERT_TRUE( true == DummyStringEntry.Serialize( Stream.GetStreamBase(), false ) );
            ASSERT_TRUE( sizeof( SKL::DC::TBlockIndices ) == Stream.GetPosition() );
        }

        {
            Stream.SetPosition( 0 );

            Datacenter::StringEntry StringEntry;
            ASSERT_TRUE( true == StringEntry.Serialize( Stream.GetStreamBase(), true ) );
            ASSERT_TRUE( sizeof( SKL::DC::TBlockIndices ) == Stream.GetPosition() );
            StringEntry_APIDummy_Validate( StringEntry );
        }
    }
    
    TEST_F( DatacenterTestsFixture, StringMap_API )
    {
        SKL::BufferStream Stream{ 4096 * 1024 };

        {
            auto DummyStringMap{ StringMap_APIDummy() };
            ASSERT_TRUE( true == DummyStringMap.Serialize( Stream.GetStreamBase(), false ) );
            ASSERT_TRUE( 
                    (
                        (
                            sizeof( uint32_t ) + ( sizeof( SKL::DC::TBlockIndices ) * 2 )
                        )
                        +
                        ( 
                            sizeof( uint32_t ) + ( ( ( sizeof( uint16_t ) * 2 ) + ( sizeof( wchar_t ) * SKL::DC::CStringsBlockSize ) ) )
                        )
                    )
                    == Stream.GetPosition() );
        }

        {
            Stream.SetPosition( 0 );

            BuildDatacenter::StringMap StringMap;
            ASSERT_TRUE( true == StringMap.Serialize( Stream.GetStreamBase(), true ) );
            ASSERT_TRUE( 
                    (
                        (
                            sizeof( uint32_t ) + ( sizeof( SKL::DC::TBlockIndices ) * 2 )
                        )
                        +
                        ( 
                            sizeof( uint32_t ) + ( ( ( sizeof( uint16_t ) * 2 ) + ( sizeof( wchar_t ) * SKL::DC::CStringsBlockSize ) ) )
                        )
                    )
                    == Stream.GetPosition() );
            StringMap_APIDummy_Validate( StringMap );
        }
    }
    
    TEST_F( DatacenterTestsFixture, Attribute_ValueAPI )
    {
       Attribute_ValueAPIDummy_Validate();
    }

    TEST_F( DatacenterTestsFixture, Datacenter_API )
    {
        {
            SKL::BufferStream Stream{ 4096 * 1024 };
            Datacenter DC;
            DC.SetStream( &Stream.GetStreamBase() );
        }

        {
            SKL::BufferStream Stream{ 4096 * 1024 };
            BuildDatacenter DC{};
            DC.SetStream( &Stream.GetStreamBase() );

            auto& ValuesMap = DC.GetValuesMap();
            auto& NamesMap = DC.GetNamesMap();
            auto& ElementsBlock = DC.GetElementsBlock();
            auto& AttributesBlock = DC.GetAttributesBlock();
        }
    }

    TEST_F( DatacenterTestsFixture, Client_Builder_API )
    {
        {
             SKL::DC::Builder      DCBuilder{};
             SKL::DC::TFilterIndex FilterIndex{ 0 }; //For client

             TestDatacenterAdapter* Adapter{ new TestDatacenterAdapter() };
             SKL_ASSERT( nullptr != Adapter );

             Adapter->SetTargetDirectory( "./xml/" );
             DCBuilder.SetAdapter( Adapter );
             DCBuilder.SetVersion( 1 );
             DCBuilder.SetFormatVersion( 2 );
         
             const auto BuildResult{ DCBuilder.Build( FilterIndex ) };
             ASSERT_TRUE( true == BuildResult );

             SKL::DC::Builder::MyDatacenter& DC{ DCBuilder.GetDatacenter() };

             SKL::BufferStream Stream{ 4096 * 1024 };
             DC.SetStream( &Stream.GetStreamBase() );
             ASSERT_TRUE( true == DC.Serialize( false ) );
             ASSERT_TRUE( true == DC.SaveToFile( "./Datacenter_Client.bin" ) );
        }
        
        {
            SKL::DC::Datacenter<false> DC{};

            auto Stream{ SKL::BufferStream::OpenFile( "./Datacenter_Client.bin" ) };
            ASSERT_TRUE( true == !!Stream );

            DC.SetStream( &Stream.value().GetStreamBase() );

            ASSERT_TRUE( true == DC.Serialize( true ) );
            
            const SKL::DC::Datacenter<false>::Element* RootElement{ DC.GetRootElement() };
            ASSERT_TRUE( true == RootElement->IsNamed( L"__root__" ) );
            ASSERT_TRUE( 0 < RootElement->GetChildren().size() );

            auto ClientSettings{ DC.GetAllByName( L"ClientSettings" ) };
            ASSERT_TRUE( 1 == ClientSettings.size() );

            auto* ClientSettingsElement{ ClientSettings[0] };
            ASSERT_TRUE( nullptr != ClientSettingsElement );
            ASSERT_TRUE( 2 == ClientSettingsElement->GetAttributes().size() );

            auto* ClientSettingsElement_Attr1{ ClientSettingsElement->GetAttributes()[0] };
            ASSERT_TRUE( nullptr != ClientSettingsElement_Attr1 );
            ASSERT_TRUE( true == ClientSettingsElement_Attr1->IsNamed( L"version" ) );
            ASSERT_TRUE( true == ClientSettingsElement_Attr1->IsValue( L"1" ) );
            
            auto* ClientSettingsElement_Attr2{ ClientSettingsElement->GetAttributes()[1] };
            ASSERT_TRUE( nullptr != ClientSettingsElement_Attr2 );
            ASSERT_TRUE( true == ClientSettingsElement_Attr2->IsNamed( L"name" ) );
            ASSERT_TRUE( true == ClientSettingsElement_Attr2->IsValue( L"Skylake Client" ) );
        }
    }
    
    TEST_F( DatacenterTestsFixture, Server_Builder_API )
    {
        {
             SKL::DC::Builder      DCBuilder{};
             SKL::DC::TFilterIndex FilterIndex{ 1 }; //For server

             TestDatacenterAdapter* Adapter{ new TestDatacenterAdapter() };
             SKL_ASSERT( nullptr != Adapter );

             Adapter->SetTargetDirectory( "./xml/" );
             DCBuilder.SetAdapter( Adapter );
             DCBuilder.SetVersion( 1 );
             DCBuilder.SetFormatVersion( 2 );
         
             const auto BuildResult{ DCBuilder.Build( FilterIndex ) };
             ASSERT_TRUE( true == BuildResult );

             SKL::DC::Builder::MyDatacenter& DC{ DCBuilder.GetDatacenter() };

             SKL::BufferStream Stream{ 4096 * 1024 };
             DC.SetStream( &Stream.GetStreamBase() );
             ASSERT_TRUE( true == DC.Serialize( false ) );
             ASSERT_TRUE( true == DC.SaveToFile( "./Datacenter_Server.bin" ) );
        }
        
        {
            SKL::DC::Datacenter<false> DC{};

            auto Stream{ SKL::BufferStream::OpenFile( "./Datacenter_Server.bin" ) };
            ASSERT_TRUE( true == !!Stream );

            DC.SetStream( &Stream.value().GetStreamBase() );

            ASSERT_TRUE( true == DC.Serialize( true ) );

            const SKL::DC::Datacenter<false>::Element* RootElement{ DC.GetRootElement() };
            ASSERT_TRUE( true == RootElement->IsNamed( L"__root__" ) );
            ASSERT_TRUE( 0 < RootElement->GetChildren().size() );

            auto ClientSettings{ DC.GetAllByName( L"ClientSettings" ) };
            ASSERT_TRUE( 1 == ClientSettings.size() );

            auto* ClientSettingsElement{ ClientSettings[0] };
            ASSERT_TRUE( nullptr != ClientSettingsElement );
            ASSERT_TRUE( 2 == ClientSettingsElement->GetAttributes().size() );

            auto* ClientSettingsElement_Attr1{ ClientSettingsElement->GetAttributes()[0] };
            ASSERT_TRUE( nullptr != ClientSettingsElement_Attr1 );
            ASSERT_TRUE( true == ClientSettingsElement_Attr1->IsNamed( L"version" ) );
            ASSERT_TRUE( true == ClientSettingsElement_Attr1->IsValue( L"1" ) );
            
            auto* ClientSettingsElement_Attr2{ ClientSettingsElement->GetAttributes()[1] };
            ASSERT_TRUE( nullptr != ClientSettingsElement_Attr2 );
            ASSERT_TRUE( true == ClientSettingsElement_Attr2->IsNamed( L"name" ) );
            ASSERT_TRUE( true == ClientSettingsElement_Attr2->IsValue( L"Skylake Client" ) );
        }
    }
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}