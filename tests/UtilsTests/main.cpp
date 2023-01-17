#include <gtest/gtest.h>

#include <SkylakeLib.h>

namespace Utils_Tests_Suite
{
    constexpr uint32_t CBufferSize { 1024 }; // multiple of sizeof( uint32_t )

    struct MyTrivialType
    {
        int a = 1, b = 2, c = 3;
    };

    const char* FileNamePtr{ nullptr };

    TEST( Utils_Tests_Suite, GRand_API_Test_Case )
    {
        constexpr size_t IterCount{ 1024 };

        auto RandBuffer  { std::make_unique<uint32_t[]>( IterCount ) };
        auto RandBuffer2 { std::make_unique<uint32_t[]>( IterCount ) };
        auto RandBufferF { std::make_unique<float[]>( IterCount ) };
        auto RandBufferD { std::make_unique<double[]>( IterCount ) };

        memset( RandBuffer.get(), 0, sizeof( uint32_t ) * IterCount );
        memset( RandBuffer2.get(), 0, sizeof( uint32_t ) * IterCount );
        memset( RandBufferF.get(), 0, sizeof( float ) * IterCount );
        memset( RandBufferD.get(), 0, sizeof( double ) * IterCount );

        for( size_t i = 0; i < IterCount; ++i )
        {
            RandBuffer[ i ]  = SKL::GRand::NextRandom();
            RandBuffer2[ i ] = SKL::GRand::NextRandomInRange( 0, std::numeric_limits<uint32_t>::max() / 2 );
            RandBufferF[ i ] = SKL::GRand::NextRandomF();
            RandBufferD[ i ] = SKL::GRand::NextRandomD();
        }
        
        for( size_t i = 0; i < IterCount; ++i )
        {
            for( size_t j = 0; j < IterCount; ++j )
            {
                if( j == i ) continue;  
                ASSERT_TRUE( RandBuffer[ i ] != RandBuffer[ j ] );
                ASSERT_TRUE( RandBuffer2[ i ] != RandBuffer2[ j ] );
                ASSERT_TRUE( RandBufferF[ i ] != RandBufferF[ j ] );
                ASSERT_TRUE( RandBufferD[ i ] != RandBufferD[ j ] );
            }
        }
    }

    TEST( Utils_Tests_Suite, BufferStream_API_Test_Case )
    {
        SKL::BufferStream Stream{ CBufferSize };
        ASSERT_TRUE( nullptr != Stream.GetBuffer() );
        ASSERT_TRUE( Stream.GetFront() == Stream.GetBuffer() );
        ASSERT_TRUE( reinterpret_cast<void*>( &Stream.GetStream() ) == &Stream );
        ASSERT_TRUE( false == Stream.IsEOS() );
        ASSERT_TRUE( 0 == Stream.GetPosition() );
        ASSERT_TRUE( Stream.GetBufferSize() == CBufferSize );
        ASSERT_TRUE( Stream.GetRemainingSize() == CBufferSize );

        Stream.ForwardToEnd();
        ASSERT_TRUE( true == Stream.IsEOS() );
        ASSERT_TRUE( Stream.GetBufferSize() == Stream.GetPosition() );
        Stream.ForwardToEnd( Stream.GetBufferSize() );
        ASSERT_TRUE( false == Stream.IsEOS() );
        ASSERT_TRUE( 0 == Stream.GetPosition() );
        Stream.Forward( Stream.GetBufferSize() );
        ASSERT_TRUE( true == Stream.IsEOS() );
        ASSERT_TRUE( Stream.GetBufferSize() == Stream.GetPosition() );
        Stream.Reset();
        ASSERT_TRUE( false == Stream.IsEOS() );
        ASSERT_TRUE( 0 == Stream.GetPosition() );

        Stream.WriteT( static_cast<uint32_t>( 1 ) );
        ASSERT_TRUE( sizeof( uint32_t ) == Stream.GetPosition() );
        ASSERT_TRUE( Stream.GetBufferSize() - static_cast<uint32_t>( sizeof( uint32_t ) ) == Stream.GetRemainingSize() );

        Stream.Reset();
        ASSERT_TRUE( false == Stream.IsEOS() );
        ASSERT_TRUE( 0 == Stream.GetPosition() );
        ASSERT_TRUE( 1 == Stream.ReadT<uint32_t>() );
        ASSERT_TRUE( sizeof( uint32_t ) == Stream.GetPosition() );
        ASSERT_TRUE( Stream.GetBufferSize() - static_cast<uint32_t>( sizeof( uint32_t ) ) == Stream.GetRemainingSize() );

        Stream.Reset();
        for( uint32_t i = 0; i < CBufferSize / sizeof( uint32_t ); ++ i )
        {
            Stream.WriteT( i );
        }
        ASSERT_TRUE( true == Stream.IsEOS() );
        ASSERT_TRUE( Stream.GetBufferSize() == Stream.GetPosition() );
        Stream.Reset();
        for( uint32_t i = 0; i < CBufferSize / sizeof( uint32_t ); ++ i )
        {
            ASSERT_TRUE( i == Stream.ReadT<uint32_t>() );
        }
        ASSERT_TRUE( true == Stream.IsEOS() );
        ASSERT_TRUE( Stream.GetBufferSize() == Stream.GetPosition() );
    }

    TEST( Utils_Tests_Suite, BufferStream_API_Test_Case_2 )
    {
        SKL::BufferStream Stream{ CBufferSize };
        SKL_ASSERT( nullptr != Stream.GetBuffer() );

        Stream.WriteT<MyTrivialType>( {} );
        ASSERT_TRUE( sizeof( MyTrivialType ) == Stream.GetPosition() );
        ASSERT_TRUE( Stream.GetBufferSize() - static_cast<uint32_t>( sizeof( MyTrivialType ) ) == Stream.GetRemainingSize() );

        Stream.Reset();
        ASSERT_TRUE( false == Stream.IsEOS() );
        ASSERT_TRUE( 0 == Stream.GetPosition() );

        auto& Ref{  Stream.BuildObjectRef<MyTrivialType>() };
        ASSERT_TRUE( 1 == Ref.a );
        ASSERT_TRUE( 2 == Ref.b );
        ASSERT_TRUE( 3 == Ref.c );
        ASSERT_TRUE( 0 == Stream.GetPosition() );

        auto Instance{ Stream.ReadT<MyTrivialType>() };
        ASSERT_TRUE( 1 == Instance.a );
        ASSERT_TRUE( 2 == Instance.b );
        ASSERT_TRUE( 3 == Instance.c );
        ASSERT_TRUE( sizeof( MyTrivialType ) == Stream.GetPosition() );
        ASSERT_TRUE( Stream.GetBufferSize() - static_cast<uint32_t>( sizeof( MyTrivialType ) ) == Stream.GetRemainingSize() );
    }

    TEST( Utils_Tests_Suite, BufferStream_CSTR_API_Test_Case_1 )
    {
        SKL::BufferStream Stream{ CBufferSize };
        SKL_ASSERT( nullptr != Stream.GetBuffer() );

        Stream.WriteString( "TEST_STRING" );
        ASSERT_TRUE( 12 == Stream.GetPosition() );

        const char* StrPtr{ reinterpret_cast<const char*>( Stream.GetBuffer() ) };
        ASSERT_TRUE( std::string{ StrPtr } == "TEST_STRING" );

        Stream.Reset();
        ASSERT_TRUE( 11 == Stream.GetFrontAsString_Size() );
        ASSERT_TRUE( 0 == SKL_STRCMP( "TEST_STRING", Stream.GetFrontAsString(), Stream.GetRemainingSize() ) );
    }

    TEST( Utils_Tests_Suite, BufferStream_CSTR_API_Test_Case_2 )
    {
        const char* MyStr { "TEST_STRING" };

        SKL::BufferStream Stream{ CBufferSize };
        SKL_ASSERT( nullptr != Stream.GetBuffer() );

        Stream.WriteString( MyStr, 12 );
        ASSERT_TRUE( 12 == Stream.GetPosition() );

        const char* StrPtr{ reinterpret_cast<const char*>( Stream.GetBuffer() ) };
        ASSERT_TRUE( std::string{ StrPtr } == "TEST_STRING" );

        Stream.Reset();
        ASSERT_TRUE( 11 == Stream.GetFrontAsString_Size() );
        ASSERT_TRUE( 0 == SKL_STRCMP( "TEST_STRING", Stream.GetFrontAsString(), Stream.GetRemainingSize() ) );
    }

    TEST( Utils_Tests_Suite, BufferStream_WCSTR_API_Test_Case_1 )
    {
        SKL::BufferStream Stream{ CBufferSize };
        SKL_ASSERT( nullptr != Stream.GetBuffer() );

        Stream.WriteWString( L"TEST_STRING" );
        ASSERT_TRUE( 12 * 2 == Stream.GetPosition() );

        const wchar_t* StrPtr{ reinterpret_cast<const wchar_t*>( Stream.GetBuffer() ) };
        ASSERT_TRUE( std::wstring{ StrPtr } == L"TEST_STRING" );

        Stream.Reset();
        ASSERT_TRUE( 11 == Stream.GetFrontAsWString_Size() );
        ASSERT_TRUE( 0 == SKL_WSTRCMP( L"TEST_STRING", Stream.GetFrontAsWString(), Stream.GetRemainingSize() / 2 ) );
    }

    TEST( Utils_Tests_Suite, BufferStream_WCSTR_API_Test_Case_2 )
    {
        const wchar_t* MyStr { L"TEST_STRING" };

        SKL::BufferStream Stream{ CBufferSize };
        SKL_ASSERT( nullptr != Stream.GetBuffer() );

        Stream.WriteWString( MyStr, 12 );
        ASSERT_TRUE( 12 * 2 == Stream.GetPosition() );

        const wchar_t* StrPtr{ reinterpret_cast<const wchar_t*>( Stream.GetBuffer() ) };
        ASSERT_TRUE( std::wstring{ StrPtr } == L"TEST_STRING" );

        Stream.Reset();
        ASSERT_TRUE( 11 == Stream.GetFrontAsWString_Size() );
        ASSERT_TRUE( 0 == SKL_WSTRCMP( L"TEST_STRING", Stream.GetFrontAsWString(), Stream.GetRemainingSize() / 2 ) );
    }

    TEST( Utils_Tests_Suite, BufferStream_File_API_Test_Case_2 )
    {
        auto Stream{ SKL::BufferStream::OpenFile( FileNamePtr ) };
        ASSERT_TRUE( true == Stream.has_value() );
        ASSERT_TRUE( true == Stream->IsValid() );

        SKL::BufferStream Stream2{ std::move( *Stream ) };
        ASSERT_TRUE( true == Stream2.IsValid() );
        ASSERT_TRUE( false == Stream->IsValid() );

    }
    
    TEST( Utils_Tests_Suite, STR_CMP_API_Test_Case )
    {
        ASSERT_TRUE( true == SKL::StringEqual( "", "" ) );
        ASSERT_TRUE( true == SKL::StringEqual( "1", "1" ) );
        ASSERT_TRUE( true == SKL::StringEqual( "asdasd", "asdasd" ) );
        ASSERT_TRUE( false == SKL::StringEqual( "asdasd", "asdAsd" ) );

        ASSERT_TRUE( true == SKL::StringEqual( L"", L"" ) );
        ASSERT_TRUE( true == SKL::StringEqual( L"1", L"1" ) );
        ASSERT_TRUE( true == SKL::StringEqual( L"asdasd", L"asdasd" ) );
        ASSERT_TRUE( false == SKL::StringEqual( L"asdasd", L"asdAsd" ) );
    }
    
    TEST( Utils_Tests_Suite, std_aligned_unique_ptr_Test_Case )
    {
        {
            std::aligned_unique_ptr<MyTrivialType, 16> ptr{ nullptr };
            ptr = std::make_unique_aligned<MyTrivialType, 16>();
            ASSERT_NE( nullptr, ptr.get() );
            ASSERT_TRUE( ( reinterpret_cast<uint64_t>( ptr.get() ) % 16 ) == 0 );
        }

        {
            std::cacheline_unique_ptr<MyTrivialType> ptr{ nullptr };
            ptr = std::make_unique_cacheline<MyTrivialType>();
            ASSERT_NE( nullptr, ptr.get() );
            ASSERT_TRUE( ( reinterpret_cast<uint64_t>( ptr.get() ) % SKL_CACHE_LINE_SIZE ) == 0 );
        }
    }
    
    TEST( Utils_Tests_Suite, std_rw_lock_Test_Case )
    {
        {
            std::rw_lock lock;

            {
                lock.lock();

                EXPECT_EQ( false, lock.try_lock() );
                EXPECT_EQ( false, lock.try_lock_shared() );

                lock.unlock();
            }
            
            {
                EXPECT_EQ( true, lock.try_lock() );
                EXPECT_EQ( false, lock.try_lock_shared() );
                lock.unlock();
            }
            
            {
                EXPECT_EQ( true, lock.try_lock_shared() );
                EXPECT_EQ( true, lock.try_lock_shared() );
                EXPECT_EQ( true, lock.try_lock_shared() );

                EXPECT_EQ( false, lock.try_lock() );

                lock.unlock_shared();
                lock.unlock_shared();
                lock.unlock_shared();

                EXPECT_EQ( true, lock.try_lock() );
                lock.unlock();
            }

            {
                std::unique_lock guard{ lock };

                EXPECT_EQ( false, lock.try_lock() );
                EXPECT_EQ( false, lock.try_lock_shared() );
            }

            {
                std::shared_lock guard{ lock };

                EXPECT_EQ( false, lock.try_lock() );
                EXPECT_EQ( true, lock.try_lock_shared() );
                EXPECT_EQ( true, lock.try_lock_shared() );
                lock.unlock_shared();
                lock.unlock_shared();
            }
            
            {
                std::unique_lock guard{ lock };

                EXPECT_EQ( false, lock.try_lock() );
                EXPECT_EQ( false, lock.try_lock_shared() );
            }

            {
                std::shared_lock guard{ lock };

                EXPECT_EQ( false, lock.try_lock() );
                EXPECT_EQ( true, lock.try_lock_shared() );
                EXPECT_EQ( true, lock.try_lock_shared() );
                lock.unlock_shared();
                lock.unlock_shared();
            }
        }
    }
}

int main( int argc, char** argv )
{
    Utils_Tests_Suite::FileNamePtr = argv[0];

    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}