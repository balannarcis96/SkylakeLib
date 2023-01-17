#include <gtest/gtest.h>

#include <SkylakeLibDB.h>

namespace SkylakeDBTests
{
    class SkylakeDBTestsFixture: public ::testing::Test
    {
        void SetUp() override
        {
            EXPECT_TRUE( true == SKL::DB::DBLibGuard::IsValidLib() );

            SKL::DB::DBConnectionSettings Settings
            {
                  .Username                    = "developer"
                , .Password                    = "123456aA!"
                , .Database                    = "sys"
                , .Host                        = "localhost"
                , .Port                        = 3306
                , .ReacquireConnectionMaxTries = 3
                , .ConnectionTimeoutMs         = 100
                , .bAutocommit                 = true       
            };

            EXPECT_TRUE( true == DBConnectionFactory.Initialize( std::move( Settings ) ) );
        }

        void TearDown() override
        {
            EXPECT_TRUE( true == SKL::DB::DBLibGuard::IsValidLib() );
        }

    protected:
        SKL::DB::DBConnectionFactory DBConnectionFactory{};
    };

    class SkylakeDBTestsFixture2: public ::testing::Test
    {
        void SetUp() override
        {
            EXPECT_TRUE( true == SKL::DB::DBLibGuard::IsValidLib() );

            SKL::DB::DBConnectionSettings Settings
            {
                  .Username                    = "developer"
                , .Password                    = "123456aA!"
                , .Database                    = "skylake_db"
                , .Host                        = "localhost"
                , .Port                        = 3306
                , .ReacquireConnectionMaxTries = 3
                , .ConnectionTimeoutMs         = 100
                , .bAutocommit                 = true       
            };

            EXPECT_TRUE( true == DBConnectionFactory.Initialize( std::move( Settings ) ) );
        }

        void TearDown() override
        {
            EXPECT_TRUE( true == SKL::DB::DBLibGuard::IsValidLib() );
        }

    protected:
        SKL::DB::DBConnectionFactory DBConnectionFactory{};
    };

    TEST( SkylakeDBTestsSuite, DBString_BasicAPI_Test )
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

    TEST_F( SkylakeDBTestsFixture, DISABLED_DBConnection_BasicAPI_Test )
    {
        {
            auto Connection{ DBConnectionFactory.TryOpenNewConnection()  };
            ASSERT_TRUE( nullptr != Connection );
            ASSERT_TRUE( true == Connection->IsOpen() );
            const auto CountResult{ Connection->Execute( "UPDATE sys_config SET value='101' WHERE variable='statement_performance_analyzer.limit'" ) };
            ASSERT_TRUE( -1 != CountResult );
        }
        
        {
            auto Connection{ DBConnectionFactory.TryOpenNewConnection()  };
            ASSERT_TRUE( nullptr != Connection );
            ASSERT_TRUE( true == Connection->IsOpen() );
            const auto CountResult{ Connection->Execute( "UPDATE sys_config SET value='100' WHERE variable='statement_performance_analyzer.limit'", 88 ) };
            ASSERT_TRUE( -1 != CountResult );
        }
    }
    
    TEST_F( SkylakeDBTestsFixture, DISABLED_DBStatement_BasicAPI_Test )
    {
        {
            auto Connection{ DBConnectionFactory.TryOpenNewConnection()  };
            ASSERT_TRUE( nullptr != Connection );
            ASSERT_TRUE( true == Connection->IsOpen() );

            std::unique_ptr<SKL::DB::DBStatement> NewStatement{ new SKL::DB::DBStatement() };
            NewStatement->SetQuery( "SELECT * FROM sys_config" );
            ASSERT_TRUE( true == NewStatement->InitializeAndPrepare( Connection.get() ) );
            ASSERT_TRUE( true == NewStatement->IsInitialized() );
            
            SKL::DB::DBString<128> Variable;
            SKL::DB::DBString<128> Value;
            SKL::DB::DBString<128> SetBy;
            SKL::DB::DBTimeStamp   SetTime;

            NewStatement->BindOutputString( 1, Variable );
            NewStatement->BindOutputString( 2, Value );
            NewStatement->BindOutputDate( 3, &SetTime );
            NewStatement->BindOutputString( 4, Value );

            auto Result{ NewStatement->Execute() };
            ASSERT_TRUE( true == Result.IsValid() );
            ASSERT_TRUE( false == Result.IsEmpty() );
            ASSERT_TRUE( 6 == Result.GetNoOfRows() );

            while( true == Result.Next() )
            {
                SKLL_INF_FMT( "[ Variable: %s Value:%s SetTime:[Y:%u M:%u D:%u h:%u m:%u s:%u] SetBy:%s]"
                            , Variable.GetUtf8()
                            , Value.GetUtf8()
                            , SetTime.Year
                            , SetTime.Month
                            , SetTime.Day
                            , SetTime.Hour
                            , SetTime.Minute
                            , SetTime.Second
                            , SetBy.GetUtf8() );
            } 
        }
    }
    
    TEST_F( SkylakeDBTestsFixture2, DISABLED_DBStatement_BasicAPI_Test_2 )
    {
        {
            auto Connection{ DBConnectionFactory.TryOpenNewConnection()  };
            ASSERT_TRUE( nullptr != Connection );
            ASSERT_TRUE( true == Connection->IsOpen() );

            std::unique_ptr<SKL::DB::DBStatement> NewStatement{ new SKL::DB::DBStatement() };
            NewStatement->SetQuery( "SELECT email, lastOnlineUTC FROM accounts WHERE username=? AND password=?" );
            ASSERT_TRUE( true == NewStatement->InitializeAndPrepare( Connection.get() ) );
            ASSERT_TRUE( true == NewStatement->IsInitialized() );
            
            SKL::DB::DBString<128> Username = SKL::DB::DBString<128>::FromUtf8( "test123" );
            SKL::DB::DBString<128> Password = SKL::DB::DBString<128>::FromUtf8( "cc03e747a6afbbcbf8be7668acfebee5" );

            NewStatement->BindString( 1, Username );
            NewStatement->BindString( 2, Password );
            
            SKL::DB::DBString<128> Email;
            uint64_t               LastOnlineUTC { 0 };
            
            NewStatement->BindOutputString( 1, Email );
            NewStatement->BindOutput( 2, &LastOnlineUTC );

            auto Result{ NewStatement->Execute() };
            ASSERT_TRUE( true == Result.IsValid() );
            ASSERT_TRUE( false == Result.IsEmpty() );
            ASSERT_TRUE( 1 == Result.GetNoOfRows() );
            
            while( true == Result.Next() )
            {
                SKLL_INF_FMT( "[ Email: %s LastOnlineUTC:%llu"
                            , Email.GetUtf8()
                            , LastOnlineUTC );
            } 
        }
    }
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}