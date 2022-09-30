#include <gtest/gtest.h>

#include <SkylakeDatacenter.h>

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
                ASSERT_TRUE( true == SKL::FIsNearlyEqual( 23.555f, Attr.GetFloat(), 0.0005f ) );    
            }
            
            {
                Datacenter::Attribute Attr;
                Attr.CachedValueRef = L"23.5555";
                ASSERT_TRUE( true == SKL::FIsNearlyEqual( 23.555, Attr.GetDouble(), 0.0005 ) );    
            }
            
            {
                Datacenter::Attribute Attr;
                Attr.CachedValueRef = L"23.555, 23.555";

                float Point2D[2]{ 0.0f, 0.0f };
                ASSERT_TRUE( true == Attr.Get2DPoint( Point2D ) );

                ASSERT_TRUE( true == SKL::FIsNearlyEqual( 23.555f, Point2D[0], 0.0005f ) );    
                ASSERT_TRUE( true == SKL::FIsNearlyEqual( 23.555f, Point2D[1], 0.0005f ) );    
            }
            
            {
                Datacenter::Attribute Attr;
                Attr.CachedValueRef = L"23.555, 23.555";

                double Point2D[2]{ 0.0f, 0.0f };
                ASSERT_TRUE( true == Attr.Get2DPointD( Point2D ) );

                ASSERT_TRUE( true == SKL::FIsNearlyEqual( 23.555, Point2D[0], 0.0005 ) );    
                ASSERT_TRUE( true == SKL::FIsNearlyEqual( 23.555, Point2D[1], 0.0005 ) );    
            }

            {
                Datacenter::Attribute Attr;
                Attr.CachedValueRef = L"23.555, 23.555, 23.555";

                float Point3D[3]{ 0.0f, 0.0f, 0.0f };
                ASSERT_TRUE( true == Attr.Get3DPoint( Point3D ) );

                ASSERT_TRUE( true == SKL::FIsNearlyEqual( 23.555f, Point3D[0], 0.0005f ) );    
                ASSERT_TRUE( true == SKL::FIsNearlyEqual( 23.555f, Point3D[1], 0.0005f ) );    
                ASSERT_TRUE( true == SKL::FIsNearlyEqual( 23.555f, Point3D[2], 0.0005f ) );    
            }
            
            {
                Datacenter::Attribute Attr;
                Attr.CachedValueRef = L"23.555, 23.555, 23.555";

                double Point3D[3]{ 0.0, 0.0, 0.0 };
                ASSERT_TRUE( true == Attr.Get3DPointD( Point3D ) );

                ASSERT_TRUE( true == SKL::FIsNearlyEqual( 23.555, Point3D[0], 0.0005 ) );    
                ASSERT_TRUE( true == SKL::FIsNearlyEqual( 23.555, Point3D[1], 0.0005 ) );    
                ASSERT_TRUE( true == SKL::FIsNearlyEqual( 23.555, Point3D[3], 0.0005 ) );    
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
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}