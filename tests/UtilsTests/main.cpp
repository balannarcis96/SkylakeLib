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

namespace SkylakeNetBufferTests
{
    TEST( SkylakeNetBufferTests, AsyncNetBuffer_general_API )
    {
        using MyBuffer = SKL::AsyncNetBuffer<16>; 

        struct PacketType
        {
            int32_t a, b, c;
        };

        {
            MyBuffer Buffer;

            ASSERT_EQ( Buffer.GetPosition(), 0U );
            ASSERT_EQ( Buffer.GetStream()->GetPosition(), 0U );
            ASSERT_EQ( Buffer.GetStreamInterface().GetPosition(), 0U );
            ASSERT_NE( Buffer.GetInterface().Buffer, nullptr );
            ASSERT_EQ( Buffer.GetInterface().Buffer, SKL::EditAsyncNetBuffer( Buffer ).GetBuffer() );
            ASSERT_EQ( Buffer.GetInterface().Length, MyBuffer::GetTotalBufferSize() );

            Buffer.GetStream()->WriteT( SKL::PacketHeader{ .Size = 16U, .Opcode = SKL::CBroadcastPacketOpcode } );
            Buffer.GetStream()->WriteT( 5 );
            Buffer.GetStream()->WriteT( 6 );
            Buffer.GetStream()->WriteT( SKL::PacketHeader{ .Size = 4U, .Opcode = SKL::CBroadcastPacketOpcode } );

            { // GetPacketHeader()
                ASSERT_EQ( Buffer.GetPacketHeader().Opcode, SKL::CBroadcastPacketOpcode );
                ASSERT_EQ( Buffer.GetPacketHeader().Size, 4U );
                Buffer.GetPacketHeader().Opcode = SKL::CRoutedPacketOpcode;
                ASSERT_EQ( Buffer.GetPacketHeader().Opcode, SKL::CRoutedPacketOpcode );
                Buffer.GetPacketHeader().Opcode = SKL::CBroadcastPacketOpcode;
                ASSERT_EQ( Buffer.GetPacketHeader().Opcode, SKL::CBroadcastPacketOpcode );
            }
            
            { // DoesThePacketIndicateTheBody()
                ASSERT_EQ( Buffer.DoesThePacketIndicateTheBody(), false );
            }
        }

        {
            ASSERT_EQ( MyBuffer::GetTotalBufferSize(), SKL::IAsyncNetBuffer<16>::GetTotalBufferSize() );
            ASSERT_EQ( MyBuffer::GetPacketBufferSize(), SKL::IAsyncNetBuffer<16>::GetPacketBufferSize() );
            ASSERT_EQ( MyBuffer::GetPacketBodyBufferSize(), SKL::IAsyncNetBuffer<16>::GetPacketBodyBufferSize() );
        }

        {
            MyBuffer Buffer;

            ASSERT_EQ( Buffer.GetPosition(), 0U );
            ASSERT_EQ( Buffer.GetStream()->GetPosition(), 0U );
            ASSERT_EQ( Buffer.GetStreamInterface().GetPosition(), 0U );
            ASSERT_NE( Buffer.GetInterface().Buffer, nullptr );
            ASSERT_EQ( Buffer.GetInterface().Buffer, SKL::EditAsyncNetBuffer( Buffer ).GetBuffer() );
            ASSERT_EQ( Buffer.GetInterface().Length, MyBuffer::GetTotalBufferSize() );

            Buffer.GetStream()->WriteT( SKL::PacketHeader{ .Size = 28U, .Opcode = SKL::CBroadcastPacketOpcode } );
            Buffer.GetStream()->WriteT( 5 );
            Buffer.GetStream()->WriteT( 6 );
            Buffer.GetStream()->WriteT( SKL::PacketHeader{ .Size = 16U, .Opcode = SKL::CBroadcastPacketOpcode } );
            Buffer.GetStream()->WriteT( 5 );
            Buffer.GetStream()->WriteT( 6 );
            Buffer.GetStream()->WriteT( 7 );

            { // GetPacketHeader()
                ASSERT_EQ( Buffer.GetPacketHeader().Opcode, SKL::CBroadcastPacketOpcode );
                ASSERT_EQ( Buffer.GetPacketHeader().Size, 16U );
                Buffer.GetPacketHeader().Opcode = SKL::CRoutedPacketOpcode;
                ASSERT_EQ( Buffer.GetPacketHeader().Opcode, SKL::CRoutedPacketOpcode );
                Buffer.GetPacketHeader().Opcode = SKL::CBroadcastPacketOpcode;
                ASSERT_EQ( Buffer.GetPacketHeader().Opcode, SKL::CBroadcastPacketOpcode );
            }
            
            { // DoesThePacketIndicateTheBody()
                ASSERT_EQ( Buffer.DoesThePacketIndicateTheBody(), true );
            }

            { // CastToPacketType()
                PacketType& Packet{ Buffer.CastToPacketType<PacketType>() };

                ASSERT_EQ( Packet.a, 5 );
                ASSERT_EQ( Packet.b, 6 );
                ASSERT_EQ( Packet.c, 7 );
            }
            
            { // EditAsyncNetBuffer()
                ASSERT_EQ( Buffer.GetPosition(), 28U );
                auto& GeneralEditInterface = SKL::EditAsyncNetBuffer( Buffer );
                ASSERT_EQ( GeneralEditInterface.GetStream().GetPosition(), 28U );

                { //GetBuffer() GetPacketBuffer() GetPacketBodyBuffer()
                    ASSERT_NE( nullptr, GeneralEditInterface.GetBuffer() );
                    ASSERT_NE( nullptr, GeneralEditInterface.GetPacketBuffer() );
                    ASSERT_EQ( GeneralEditInterface.GetBuffer() + SKL::IAsyncNetBuffer<16>::CPacketHeaderOffset, GeneralEditInterface.GetPacketBuffer() );
                    ASSERT_NE( nullptr, GeneralEditInterface.GetPacketBodyBuffer() );
                }

                { // GetStream() GetSuper()
                    ASSERT_EQ( GeneralEditInterface.GetStream().GetBuffer(), GeneralEditInterface.GetBuffer() );
                    ASSERT_EQ( GeneralEditInterface.GetStream().GetPosition(), GeneralEditInterface.GetSuper().GetPosition() );
                    ASSERT_EQ( &GeneralEditInterface.GetSuper(), &Buffer );
                }
                
                { // GetCurrentlyReceivedByteCount()
                    ASSERT_EQ( GeneralEditInterface.GetCurrentlyReceivedByteCount(), Buffer.GetPosition() );
                    ASSERT_EQ( GeneralEditInterface.GetCurrentlyReceivedByteCount(), GeneralEditInterface.GetSuper().GetPosition() );
                }

                { // GetPacketHeader()
                    ASSERT_EQ( GeneralEditInterface.GetPacketHeader().Opcode, SKL::CBroadcastPacketOpcode );
                    ASSERT_EQ( GeneralEditInterface.GetPacketHeader().Size, 16U );
                    GeneralEditInterface.GetPacketHeader().Opcode = SKL::CRoutedPacketOpcode;
                    ASSERT_EQ( GeneralEditInterface.GetPacketHeader().Opcode, SKL::CRoutedPacketOpcode );
                    GeneralEditInterface.GetPacketHeader().Opcode = SKL::CBroadcastPacketOpcode;
                    ASSERT_EQ( GeneralEditInterface.GetPacketHeader().Opcode, SKL::CBroadcastPacketOpcode );
                }
            }
        }
    }

    TEST( SkylakeNetBufferTests, AsyncNetBuffer_IRoutedAsyncNetBuffer_API )
    {
        using MyBuffer = SKL::AsyncNetBuffer<16>; 

        struct PacketType
        {
            int32_t a, b, c;
        };

        {
            MyBuffer Buffer;
            
            auto& EditRoutedBuffer = SKL::EditRoutingAsyncNetBuffer( Buffer );
            
            { // HasValidRoutingData()
                ASSERT_FALSE( EditRoutedBuffer.HasValidRoutingData() );
            }

            ASSERT_EQ( EditRoutedBuffer.GetStream().GetFront(), EditRoutedBuffer.GetBuffer() );
            ASSERT_EQ( EditRoutedBuffer.GetSuper().GetStream()->GetFront(), EditRoutedBuffer.GetBuffer() );

            SKL::EditAsyncNetBuffer( Buffer ).PrepareForReceivingHeader();

            ASSERT_EQ( EditRoutedBuffer.GetStream().GetFront(), EditRoutedBuffer.GetPacketBuffer() );
            ASSERT_EQ( EditRoutedBuffer.GetSuper().GetStream()->GetFront(), EditRoutedBuffer.GetPacketBuffer() );

            ASSERT_EQ( 0U, Buffer.GetPosition() );
            ASSERT_EQ( EditRoutedBuffer.GetPacketBuffer(), Buffer.GetStream()->GetBuffer() );
            ASSERT_EQ( EditRoutedBuffer.GetPacketBuffer(), Buffer.GetStreamInterface().GetBuffer() );
            ASSERT_EQ( SKL::CPacketHeaderSize, Buffer.GetStream()->GetBufferSize() );
            ASSERT_EQ( SKL::CPacketHeaderSize, Buffer.GetStream()->GetBufferUnitSize() );
            ASSERT_EQ( SKL::CPacketHeaderSize, Buffer.GetStreamInterface().GetBufferSize() );
            ASSERT_EQ( SKL::CPacketHeaderSize, Buffer.GetStreamInterface().GetBufferUnitSize() );

            // Write the packet header
            Buffer.GetStream()->WriteT( SKL::PacketHeader{ .Size = 28U, .Opcode = SKL::CRoutedPacketOpcode } );
            ASSERT_EQ( SKL::CPacketHeaderSize, Buffer.GetPosition() );

            { // GetRoutingHeader()
                ASSERT_EQ( 0U, EditRoutedBuffer.GetRoutingHeader().Size );
                ASSERT_EQ( SKL::CInvalidOpcode, EditRoutedBuffer.GetRoutingHeader().Opcode );
            }

            { // HasValidRoutingData()
                ASSERT_FALSE( EditRoutedBuffer.HasValidRoutingData() );
            }

            ASSERT_EQ( 28U, EditRoutedBuffer.GetPacketHeader().Size );
            ASSERT_EQ( SKL::CRoutedPacketOpcode, EditRoutedBuffer.GetPacketHeader().Opcode );
            ASSERT_EQ( EditRoutedBuffer.GetSuper().GetStream()->GetFront(), EditRoutedBuffer.GetPacketBodyBuffer() );

            EditRoutedBuffer.PrepareFoReceivingRoutedPacketBody();
            
            ASSERT_EQ( EditRoutedBuffer.GetPacketHeader().Size, EditRoutedBuffer.GetRoutingHeader().Size );
            ASSERT_EQ( EditRoutedBuffer.GetPacketHeader().Opcode, EditRoutedBuffer.GetRoutingHeader().Opcode );
            ASSERT_EQ( EditRoutedBuffer.GetStream().GetPosition(), SKL::CPacketHeaderSize );
            ASSERT_EQ( EditRoutedBuffer.GetStream().GetBuffer(), EditRoutedBuffer.GetRoutingBodyBuffer() );
            ASSERT_EQ( EditRoutedBuffer.GetStream().GetBufferLength(), 28U - SKL::CPacketHeaderSize );
            ASSERT_EQ( EditRoutedBuffer.GetSuper().GetInterface().Length, 28U - SKL::CPacketHeaderSize );

            ASSERT_EQ( EditRoutedBuffer.GetSuper().GetStream()->GetBuffer(), EditRoutedBuffer.GetRoutingBodyBuffer() );
            ASSERT_EQ( EditRoutedBuffer.GetSuper().GetStream()->GetBufferSize(), 28U - SKL::CPacketHeaderSize );
            ASSERT_EQ( EditRoutedBuffer.GetSuper().GetStream()->GetBufferUnitSize(), 28U - SKL::CPacketHeaderSize );
            
            ASSERT_EQ( Buffer.GetStream()->GetBuffer(), EditRoutedBuffer.GetRoutingBodyBuffer() );
            ASSERT_EQ( Buffer.GetStream()->GetBufferSize(), 28U - SKL::CPacketHeaderSize );
            ASSERT_EQ( Buffer.GetStream()->GetBufferUnitSize(), 28U - SKL::CPacketHeaderSize );

            ASSERT_EQ( EditRoutedBuffer.GetSuper().GetStreamInterface().GetBuffer(), EditRoutedBuffer.GetRoutingBodyBuffer() );
            ASSERT_EQ( EditRoutedBuffer.GetSuper().GetStreamInterface().GetBufferSize(), 28U - SKL::CPacketHeaderSize );
            ASSERT_EQ( EditRoutedBuffer.GetSuper().GetStreamInterface().GetBufferUnitSize(), 28U - SKL::CPacketHeaderSize );
            
            ASSERT_EQ( Buffer.GetStreamInterface().GetBuffer(), EditRoutedBuffer.GetRoutingBodyBuffer() );
            ASSERT_EQ( Buffer.GetStreamInterface().GetBufferSize(), 28U - SKL::CPacketHeaderSize );
            ASSERT_EQ( Buffer.GetStreamInterface().GetBufferUnitSize(), 28U - SKL::CPacketHeaderSize );

            ASSERT_EQ( &EditRoutedBuffer.GetStream(), &Buffer.GetStreamBase() );
            ASSERT_EQ( EditRoutedBuffer.GetBuffer(), Buffer.GetBuffer() );

            // Write the routed packet entityId
            *reinterpret_cast<SKL::TEntityIdBase*>( EditRoutedBuffer.GetSuper().GetInterface().Buffer ) = SKL::TEntityIdBase( 5 );

            ASSERT_EQ( EditRoutedBuffer.GetSuper().GetStream()->GetPosition(), SKL::CPacketHeaderSize );
            ASSERT_EQ( EditRoutedBuffer.GetSuper().GetStreamInterface().GetPosition(), SKL::CPacketHeaderSize );

            const auto Id = EditRoutedBuffer.GetEntityId();
            ASSERT_EQ( SKL::TEntityIdBase( 5 ), Id );
        }
        
        {
            MyBuffer Buffer;

            SKL::EditAsyncNetBuffer( Buffer ).PrepareForReceivingHeader();

            *reinterpret_cast<SKL::PacketHeader*>( Buffer.GetStreamBase().GetBuffer() ) = SKL::PacketHeader{ .Size = 28U, .Opcode = SKL::CRoutedPacketOpcode };
            
            auto& EditRoutedBuffer = SKL::EditRoutingAsyncNetBuffer( Buffer );
            
            { // GetRoutingHeader()
                ASSERT_EQ( 0U, EditRoutedBuffer.GetRoutingHeader().Size );
                ASSERT_EQ( SKL::CInvalidOpcode, EditRoutedBuffer.GetRoutingHeader().Opcode );
            }

            { // HasValidRoutingData()
                ASSERT_FALSE( EditRoutedBuffer.HasValidRoutingData() );
            }

            EditRoutedBuffer.PrepareFoReceivingRoutedPacketBody();
            
            // Write the routed packet entityId but don't edit the underlying stream
            *reinterpret_cast<SKL::TEntityIdBase*>( Buffer.GetStreamBase().GetBuffer() ) = 5;

            {
                const auto [hasReceivedWholePacket, bProcessedSuccessfully] = SKL::EditAsyncNetBuffer( Buffer ).ConfirmReceivedExactAmmount( 8U );
                ASSERT_TRUE( bProcessedSuccessfully );
                ASSERT_FALSE( hasReceivedWholePacket );
            }

            *reinterpret_cast<SKL::TPacketSize*>( Buffer.GetStreamBase().GetBuffer() ) = 16U;

            {
                const auto [hasReceivedWholePacket, bProcessedSuccessfully] = SKL::EditAsyncNetBuffer( Buffer ).ConfirmReceivedExactAmmount( 2U );
                ASSERT_TRUE( bProcessedSuccessfully );
                ASSERT_FALSE( hasReceivedWholePacket );
            }
            
            *reinterpret_cast<SKL::TPacketOpcode*>( Buffer.GetStreamBase().GetBuffer() ) = 55U;
            *reinterpret_cast<int32_t*>( Buffer.GetStreamBase().GetBuffer() + 2U )       = 1;
            *reinterpret_cast<int32_t*>( Buffer.GetStreamBase().GetBuffer() + 6U )       = 2;
            *reinterpret_cast<int32_t*>( Buffer.GetStreamBase().GetBuffer() + 10U )      = 3;

            {
                const auto [hasReceivedWholePacket, bProcessedSuccessfully] = SKL::EditAsyncNetBuffer( Buffer ).ConfirmReceivedExactAmmount( 14U );
                ASSERT_TRUE( bProcessedSuccessfully );
                ASSERT_TRUE( hasReceivedWholePacket );
            }

            ASSERT_EQ( EditRoutedBuffer.GetEntityId(), 5U );
            ASSERT_EQ( EditRoutedBuffer.GetPacketHeader().Size, 16U );
            ASSERT_EQ( EditRoutedBuffer.GetPacketHeader().Opcode, 55U );
            ASSERT_EQ( EditRoutedBuffer.CastToPacketType<PacketType>().a, 1 );
            ASSERT_EQ( EditRoutedBuffer.CastToPacketType<PacketType>().b, 2 );
            ASSERT_EQ( EditRoutedBuffer.CastToPacketType<PacketType>().c, 3 );
        }
    }
    
    TEST( SkylakeNetBufferTests, AsyncNetBuffer_IRoutedAsyncNetBuffer_API2 )
    {
        using MyBuffer = SKL::AsyncNetBuffer<16>; 

        struct PacketType
        {
            int32_t a, b, c;
        };

        {
            MyBuffer Buffer;

            SKL::EditAsyncNetBuffer( Buffer ).PrepareForReceivingHeader();

            *reinterpret_cast<SKL::PacketHeader*>( Buffer.GetStreamBase().GetBuffer() ) = SKL::PacketHeader{ .Size = 28U, .Opcode = SKL::CRoutedPacketOpcode };
            
            auto& EditRoutedBuffer = SKL::EditRoutingAsyncNetBuffer( Buffer );
            
            { // GetRoutingHeader()
                ASSERT_EQ( 0U, EditRoutedBuffer.GetRoutingHeader().Size );
                ASSERT_EQ( SKL::CInvalidOpcode, EditRoutedBuffer.GetRoutingHeader().Opcode );
            }

            { // HasValidRoutingData()
                ASSERT_FALSE( EditRoutedBuffer.HasValidRoutingData() );
            }

            EditRoutedBuffer.PrepareFoReceivingRoutedPacketBody();
            
            // Write the routed packet entityId but don't edit the underlying stream
            *reinterpret_cast<SKL::TEntityIdBase*>( Buffer.GetStreamBase().GetBuffer() ) = 5;

            {
                const auto [hasReceivedWholePacket, bProcessedSuccessfully] = SKL::EditAsyncNetBuffer( Buffer ).ConfirmReceivedExactAmmount( 8U );
                ASSERT_TRUE( bProcessedSuccessfully );
                ASSERT_FALSE( hasReceivedWholePacket );
            }

            *reinterpret_cast<SKL::TPacketSize*>( Buffer.GetStreamBase().GetBuffer() ) = 16U;
            ASSERT_EQ( EditRoutedBuffer.GetEntityId(), 5U );

            {
                const auto [hasReceivedWholePacket, bProcessedSuccessfully] = SKL::EditAsyncNetBuffer( Buffer ).ConfirmReceivedExactAmmount( 2U );
                ASSERT_TRUE( bProcessedSuccessfully );
                ASSERT_FALSE( hasReceivedWholePacket );
            }

            ASSERT_EQ( EditRoutedBuffer.GetEntityId(), 5U );

            {
                const auto [hasReceivedWholePacket, bProcessedSuccessfully] = SKL::EditAsyncNetBuffer( Buffer ).ConfirmReceivedExactAmmount( 15U );
                ASSERT_FALSE( bProcessedSuccessfully );
                ASSERT_FALSE( hasReceivedWholePacket );
            }
        }
    }
    
    TEST( SkylakeNetBufferTests, AsyncNetBuffer_IBroadcastAsyncNetBuffer_API )
    {
        using MyBuffer   = SKL::AsyncNetBuffer<16>; 
        using MyEntityId = uint32_t;
        using MyBroadcastAsyncNetBuffer = SKL::IBroadcastAsyncNetBuffer<MyEntityId, 16>;

        struct PacketType
        {
            int32_t a, b, c;
        };

        constexpr SKL::TPacketSize CSize = 
                  SKL::CPacketHeaderSize
                + 1U // Type
                + 3U // Payload
                + 2U // Offset
                + 2U // Count
                + SKL::CPacketHeaderSize
                + sizeof( PacketType )
                + sizeof( MyEntityId ) * 2U;

        {
            MyBuffer Buffer;

            SKL::EditAsyncNetBuffer( Buffer ).PrepareForReceivingHeader();

            *reinterpret_cast<SKL::PacketHeader*>( Buffer.GetStreamBase().GetBuffer() ) = SKL::PacketHeader{ .Size = CSize, .Opcode = SKL::CBroadcastPacketOpcode };
            
            MyBroadcastAsyncNetBuffer& EditBroadcastBuffer = SKL::EditBroadcastAsyncNetBuffer<MyEntityId>( Buffer );
            
            { // GetBroadcastHeader()
                ASSERT_EQ( 0U, EditBroadcastBuffer.GetBroadcastHeader().Size );
                ASSERT_EQ( SKL::CInvalidOpcode, EditBroadcastBuffer.GetBroadcastHeader().Opcode );
            }

            { // HasValidBroadcastData()
                ASSERT_FALSE( EditBroadcastBuffer.HasValidBroadcastData() );
            }

            EditBroadcastBuffer.PrepareFoReceivingBroadcastPacketBody();
            
            ASSERT_EQ( EditBroadcastBuffer.GetPacketHeader().Size, EditBroadcastBuffer.GetBroadcastHeader().Size );
            ASSERT_EQ( EditBroadcastBuffer.GetPacketHeader().Opcode, EditBroadcastBuffer.GetBroadcastHeader().Opcode );
            ASSERT_EQ( EditBroadcastBuffer.GetStream().GetPosition(), SKL::CPacketHeaderSize );
            ASSERT_EQ( EditBroadcastBuffer.GetStream().GetBuffer(), EditBroadcastBuffer.GetBroadcastBodyBuffer() );
            ASSERT_EQ( EditBroadcastBuffer.GetStream().GetBufferLength(), CSize - SKL::CPacketHeaderSize );
            ASSERT_EQ( EditBroadcastBuffer.GetSuper().GetInterface().Length, CSize - SKL::CPacketHeaderSize );

            ASSERT_EQ( EditBroadcastBuffer.GetSuper().GetStream()->GetBuffer(), EditBroadcastBuffer.GetBroadcastBodyBuffer() );
            ASSERT_EQ( EditBroadcastBuffer.GetSuper().GetStream()->GetBufferSize(), CSize - SKL::CPacketHeaderSize );
            ASSERT_EQ( EditBroadcastBuffer.GetSuper().GetStream()->GetBufferUnitSize(), CSize - SKL::CPacketHeaderSize );
            
            ASSERT_EQ( Buffer.GetStream()->GetBuffer(), EditBroadcastBuffer.GetBroadcastBodyBuffer() );
            ASSERT_EQ( Buffer.GetStream()->GetBufferSize(), CSize - SKL::CPacketHeaderSize );
            ASSERT_EQ( Buffer.GetStream()->GetBufferUnitSize(), CSize - SKL::CPacketHeaderSize );

            ASSERT_EQ( EditBroadcastBuffer.GetSuper().GetStreamInterface().GetBuffer(), EditBroadcastBuffer.GetBroadcastBodyBuffer() );
            ASSERT_EQ( EditBroadcastBuffer.GetSuper().GetStreamInterface().GetBufferSize(), CSize - SKL::CPacketHeaderSize );
            ASSERT_EQ( EditBroadcastBuffer.GetSuper().GetStreamInterface().GetBufferUnitSize(), CSize - SKL::CPacketHeaderSize );
            
            ASSERT_EQ( Buffer.GetStreamInterface().GetBuffer(), EditBroadcastBuffer.GetBroadcastBodyBuffer() );
            ASSERT_EQ( Buffer.GetStreamInterface().GetBufferSize(), CSize - SKL::CPacketHeaderSize );
            ASSERT_EQ( Buffer.GetStreamInterface().GetBufferUnitSize(), CSize - SKL::CPacketHeaderSize );

            // write type and payload ( 10 , 5 )
            EditBroadcastBuffer.SetBroadcastType( 0x0AU );
            EditBroadcastBuffer.SetBroadcastPayload( 0x00FFFFFFU );

            ASSERT_EQ( EditBroadcastBuffer.GetBroadcastType(), 10U );
            ASSERT_EQ( EditBroadcastBuffer.GetBroadcastPayload(), 16777215U );

            {
                const auto [hasReceivedWholePacket, bProcessedSuccessfully] = SKL::EditAsyncNetBuffer( Buffer ).ConfirmReceivedExactAmmount( 4U );
                ASSERT_TRUE( bProcessedSuccessfully );
                ASSERT_FALSE( hasReceivedWholePacket );
            }
             
            ASSERT_EQ( EditBroadcastBuffer.GetBroadcastType(), 10U );
            ASSERT_EQ( EditBroadcastBuffer.GetBroadcastPayload(), 16777215U );

            constexpr SKL::TPacketOffset Offset = MyBuffer::CPacketBodyOffset + sizeof( PacketType );

            // write count
            constexpr SKL::TPacketSize Count = ( CSize - Offset ) / sizeof( MyEntityId );
            *reinterpret_cast<SKL::TPacketSize*>( Buffer.GetStreamBase().GetBuffer() ) = Count;

            {
                const auto [hasReceivedWholePacket, bProcessedSuccessfully] = SKL::EditAsyncNetBuffer( Buffer ).ConfirmReceivedExactAmmount( 2U );
                ASSERT_TRUE( bProcessedSuccessfully );
                ASSERT_FALSE( hasReceivedWholePacket );
            }
            
            ASSERT_EQ( EditBroadcastBuffer.GetBroadcastTargetsCount(), Count );
            
            // write offset
            *reinterpret_cast<SKL::TPacketOffset*>( Buffer.GetStreamBase().GetBuffer() ) = Offset;

            {
                const auto [hasReceivedWholePacket, bProcessedSuccessfully] = SKL::EditAsyncNetBuffer( Buffer ).ConfirmReceivedExactAmmount( 2U );
                ASSERT_TRUE( bProcessedSuccessfully );
                ASSERT_FALSE( hasReceivedWholePacket );
            }
            
            ASSERT_EQ( EditBroadcastBuffer.GetBroadcastTargetsOffset(), Offset );

            // write packet
            *reinterpret_cast<SKL::TPacketSize*>( Buffer.GetStreamBase().GetBuffer() )        = 16U;
            *reinterpret_cast<SKL::TPacketOpcode*>( Buffer.GetStreamBase().GetBuffer() + 2U ) = 55U;
            *reinterpret_cast<int32_t*>( Buffer.GetStreamBase().GetBuffer() + 4U )            = 1;
            *reinterpret_cast<int32_t*>( Buffer.GetStreamBase().GetBuffer() + 8U )            = 2;
            *reinterpret_cast<int32_t*>( Buffer.GetStreamBase().GetBuffer() + 12U )           = 3;

            {
                const auto [hasReceivedWholePacket, bProcessedSuccessfully] = SKL::EditAsyncNetBuffer( Buffer ).ConfirmReceivedExactAmmount( 16U );
                ASSERT_TRUE( bProcessedSuccessfully );
                ASSERT_FALSE( hasReceivedWholePacket );
            }

            ASSERT_EQ( EditBroadcastBuffer.GetPacketHeader().Size, 16U );
            ASSERT_EQ( EditBroadcastBuffer.GetPacketHeader().Opcode, 55U );
            ASSERT_EQ( EditBroadcastBuffer.CastToPacketType<PacketType>().a, 1 );
            ASSERT_EQ( EditBroadcastBuffer.CastToPacketType<PacketType>().b, 2 );
            ASSERT_EQ( EditBroadcastBuffer.CastToPacketType<PacketType>().c, 3 );
            
            ASSERT_EQ( EditBroadcastBuffer.GetStream().GetPosition(), Offset );
            ASSERT_EQ( Buffer.GetStreamBase().GetBuffer(), Buffer.GetBuffer() + Offset );

            // write targets
            *reinterpret_cast<uint32_t*>( Buffer.GetStreamBase().GetBuffer() )     = 162U;
            *reinterpret_cast<uint32_t*>( Buffer.GetStreamBase().GetBuffer() +4U ) = 798U;

            ASSERT_EQ( *reinterpret_cast<uint32_t*>( Buffer.GetStreamBase().GetBuffer() ), 162U );
            ASSERT_EQ( *reinterpret_cast<uint32_t*>( Buffer.GetStreamBase().GetBuffer() + 4U ), 798U );
            ASSERT_EQ( Buffer.GetStreamBase().GetBuffer() + 8U, Buffer.GetBuffer() + EditBroadcastBuffer.GetBroadcastHeader().Size );
            ASSERT_EQ( *reinterpret_cast<uint32_t*>( Buffer.GetStreamBase().GetBuffer() + 4U ), 798U );

            {
                const auto [hasReceivedWholePacket, bProcessedSuccessfully] = SKL::EditAsyncNetBuffer( Buffer ).ConfirmReceivedExactAmmount( 8U );
                ASSERT_TRUE( bProcessedSuccessfully );
                ASSERT_TRUE( hasReceivedWholePacket );
            }
            
            ASSERT_EQ( EditBroadcastBuffer.GetBroadcastTargetsOffset(), Offset );
            auto Targets = EditBroadcastBuffer.GetBroadcastTargets();
            ASSERT_EQ( Targets.size(), 2U );
            ASSERT_EQ( Targets[0], 162U );
            ASSERT_EQ( Targets[1], 798U );
        }
    }
}

int main( int argc, char** argv )
{
    Utils_Tests_Suite::FileNamePtr = argv[0];

    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}
