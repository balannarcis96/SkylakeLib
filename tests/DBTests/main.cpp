#include <gtest/gtest.h>

#include <SkylakeLibDB.h>

namespace SkylakeDBTests
{
    class SkylakeDBTestsFixture: public ::testing::Test
    {
        void SetUp() override
        {
            EXPECT_TRUE( true == SKL::DB::DBLibGuard::IsValidLib() );
        }

        void TearDown() override
        {
            EXPECT_TRUE( true == SKL::DB::DBLibGuard::IsValidLib() );
        }
    };

    TEST( SkylakeDBTests, DBString_BasicAPI )
    {
        {
            auto Str{  SKL::DB::DBString<32>::FromUtf8( "TEST_STR" ) };
            ASSERT_TRUE( 0 == Str.GetUtf16SizeNoConvert() );
            ASSERT_TRUE( std::string{ "TEST_STR" } == Str.GetUtf8() );
            ASSERT_TRUE( std::wstring{ L"TEST_STR" } == Str.GetUtf16() );
            ASSERT_TRUE( 8 == Str.GetUtf8SizeNoConvert() );
            ASSERT_TRUE( 8 == Str.GetUtf16SizeNoConvert() );
            ASSERT_TRUE( Str == "TEST_STR" );
            ASSERT_TRUE( Str == L"TEST_STR" );
        }

        {
            auto Str{  SKL::DB::DBString<32>::FromUtf16( L"TEST_STR" ) };
            ASSERT_TRUE( 0 == Str.GetUtf8SizeNoConvert() );
            ASSERT_TRUE( std::wstring{ L"TEST_STR" } == Str.GetUtf16() );
            ASSERT_TRUE( std::string{ "TEST_STR" } == Str.GetUtf8() );
            ASSERT_TRUE( 8 == Str.GetUtf8SizeNoConvert() );
            ASSERT_TRUE( 8 == Str.GetUtf16SizeNoConvert() );
            ASSERT_TRUE( Str == "TEST_STR" );
            ASSERT_TRUE( Str == L"TEST_STR" );
        }

        {
            auto Str{  SKL::DB::DBString<32>::FromUtf16( L"TEST_STR" ) };
            ASSERT_TRUE( std::wstring{ L"TEST_STR" } == Str.GetUtf16() );
            ASSERT_TRUE( 0 == Str.GetUtf8SizeNoConvert() );
            ASSERT_TRUE( 8 == Str.GetUtf16SizeNoConvert() );
            ASSERT_TRUE( 8 == Str.GetUtf8Size() );
        }

        {
            auto Str{  SKL::DB::DBString<32>::FromUtf8( "TEST_STR" ) };
            ASSERT_TRUE( std::string{ "TEST_STR" } == Str.GetUtf8() );
            ASSERT_TRUE( 0 == Str.GetUtf16SizeNoConvert() );
            ASSERT_TRUE( 8 == Str.GetUtf8SizeNoConvert() );
            ASSERT_TRUE( 8 == Str.GetUtf16Size() );
        }

        {
            auto Str{  SKL::DB::DBString<32>::FromUtf16( L"TEST_STR" ) };
            
            wchar_t Buffer[64];
            Str.CopyUtf16Into( Buffer, 64 );
            ASSERT_TRUE( ( 0 == SKL_WSTRCMP( Buffer, L"TEST_STR", 64 ) ) );

            char Buffer2[64];
            Str.CopyUtf8Into( Buffer2, 64 );
            ASSERT_TRUE( ( 0 == SKL_STRCMP( Buffer2, "TEST_STR", 64 ) ) );
        }

        {
            auto Str{  SKL::DB::DBString<32>::FromUtf8( "TEST_STR" ) };

            wchar_t Buffer[64];
            Str.CopyUtf16Into( Buffer, 64 );
            ASSERT_TRUE( ( 0 == SKL_WSTRCMP( Buffer, L"TEST_STR", 64 ) ) );

            char Buffer2[64];
            Str.CopyUtf8Into( Buffer2, 64 );
            ASSERT_TRUE( ( 0 == SKL_STRCMP( Buffer2, "TEST_STR", 64 ) ) );
        }
    }

    TEST_F( SkylakeDBTestsFixture, DBConnection_BasicAPI )
    {
        
    }
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}