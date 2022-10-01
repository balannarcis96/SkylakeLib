#include <gtest/gtest.h>

#include <SkylakeLibStandalone.h>

namespace SkylakePROTOCOLTests
{
    enum class MyOpcodes: SKL::TPacketOpcode
    {
        Invalid = SKL::CInvalidOpcode
        , DO_NOT_USE = SKL::CRoutedPacketOpcode

        , HEADER_ONLY_PACKET_1
        , HEADER_ONLY_PACKET_2
        , FIXED_LENGTH_PACKET_1
        , FIXED_LENGTH_PACKET_2
        , DYNAMIC_LENGTH_PACKET_1
        , DYNAMIC_LENGTH_PACKET_2

        , Max
    };

    struct HEADER_ONLY_PACKET_1_Packet : SKL::HeaderOnlyPacketBuildContext<HEADER_ONLY_PACKET_1_Packet, static_cast<SKL::TPacketOpcode>( MyOpcodes::HEADER_ONLY_PACKET_1 )> {};
    struct FIXED_LENGTH_PACKET_1_Packet : SKL::FixedLengthPacketBuildContext<FIXED_LENGTH_PACKET_1_Packet, static_cast<SKL::TPacketOpcode>( MyOpcodes::FIXED_LENGTH_PACKET_1 )> 
    {
        uint32_t A;
        uint32_t B;
        uint32_t C;
    };
    struct DYNAMIC_LENGTH_PACKET_1_Packet final : SKL::DynamicLengthPacketBuildContext<DYNAMIC_LENGTH_PACKET_1_Packet, static_cast<SKL::TPacketOpcode>( MyOpcodes::DYNAMIC_LENGTH_PACKET_1 )>
    {
        uint32_t A;
        uint32_t B;
        uint32_t C;
        const char *String{ nullptr };
        const wchar_t *WString{ nullptr };

        SKL::TPacketSize CalculateBodySize() const noexcept override
        {
            SKL::TPacketSize Result{ 0 };
            Result += static_cast<SKL::TPacketSize>( sizeof( uint32_t ) ) * 3;
            Result += CalculateNullableStringNeededSize( String , 128 );
            Result += CalculateNullableWStringNeededSize( WString , 128 );
            return Result;
        }
        SKL::RStatus WritePacket( SKL::StreamBase& InStream ) const noexcept override
        {
            auto Writer{ SKL::IStreamWriter<true>::FromStreamBase( InStream ) };

            Writer->WriteT( A );
            Writer->WriteT( B );
            Writer->WriteT( C );

            if( nullptr == String )
            {
                Writer->WriteT<char>( '\0' );
            }
            else
            {
                Writer->WriteString( String, 128 );
            }

            if( nullptr == WString )
            {
                Writer->WriteT<wchar_t>( L'\0' );
            }
            else
            {
                Writer->WriteWString( WString, 128 );
            }

            return SKL::RSuccess;
        }
        SKL::RStatus ReadPacket( SKL::StreamBase& InStream ) noexcept override
        {
            auto Reader{ SKL::IStreamReader<true>::FromStreamBase( InStream ) };

            A = Reader->ReadT<decltype( A )>();
            B = Reader->ReadT<decltype( B )>();
            C = Reader->ReadT<decltype( C )>();
            String = Reader->GetFrontAsStringAndAdvance();
            WString = Reader->GetFrontAsWStringAndAdvance();

            return SKL::RSuccess;
        }
    };

    DEFINE_HEADER_ONLY_PACKET( HeaderOnlyMacroTest, MyOpcodes::HEADER_ONLY_PACKET_2 );
    DEFINE_FIXED_LENGTH_PACKET( FixedLengthMacroTest, MyOpcodes::FIXED_LENGTH_PACKET_2,
    {
        uint32_t A;
        uint32_t B;
        uint32_t C;
    } );
    DEFINE_DYNAMIC_PACKET( DynamicMacroTest, MyOpcodes::DYNAMIC_LENGTH_PACKET_2,
    {
        //Prerequisites
        uint32_t A;
        uint32_t B;
        uint32_t C;
        const char *String{ nullptr };
        const wchar_t *WString{ nullptr };
     
        PAKCET_CalculateBodySize()
        {
            SKL::TPacketSize Result{ 0 };
            Result += static_cast<SKL::TPacketSize>( sizeof( uint32_t ) ) * 3;
            Result += CalculateNullableStringNeededSize( String , 128 );
            Result += CalculateNullableWStringNeededSize( WString , 128 );

            return Result;
        }

        PAKCET_WritePacket()
        {
            auto Writer{ SKL::IStreamWriter<true>::FromStreamBase( InStream ) };
            
            Writer->WriteT( A );
            Writer->WriteT( B );
            Writer->WriteT( C );

            if( nullptr == String )
            {
                Writer->WriteT<char>( '\0' );
            }
            else
            {
                Writer->WriteString( String, 128 );
            }

            if( nullptr == WString )
            {
                Writer->WriteT<wchar_t>( L'\0' );
            }
            else
            {
                Writer->WriteWString( WString, 128 );
            }

            return SKL::RSuccess;
        }

        PAKCET_ReadPacket()
        {
            auto Reader{ SKL::IStreamReader<true>::FromStreamBase( InStream ) };

            A = Reader->ReadT<decltype( A )>();
            B = Reader->ReadT<decltype( B )>();
            C = Reader->ReadT<decltype( C )>();
            String = Reader->GetFrontAsStringAndAdvance();
            WString = Reader->GetFrontAsWStringAndAdvance();

            return SKL::RSuccess;
        }
    });

    TEST( SkylakePROTOCOLTests, HeaderOnlyPacketBuildContext_API )
    {
        auto Buffer = std::make_unique<uint8_t[]>( 1024 );
        SKL::BinaryStream Stream{ Buffer.get(), 1024U, 0U, false };
        ASSERT_EQ( SKL::RSuccess, HEADER_ONLY_PACKET_1_Packet::BuildPacket( Stream.GetStream() ) );
        ASSERT_TRUE( SKL::CPacketHeaderSize == Stream.GetPosition() );

        Stream.Reset();
        ASSERT_TRUE( 0 == Stream.GetPosition() );

        const auto& Header = Stream.BuildObjectRef<SKL::PacketHeader>();
        ASSERT_TRUE( Header.Size == SKL::CPacketHeaderSize );
        ASSERT_TRUE( Header.Opcode == HEADER_ONLY_PACKET_1_Packet::Base::Traits::Opcode );

        auto ReadHeader = Stream.ReadT<SKL::PacketHeader>();
        ASSERT_TRUE( ReadHeader.Size == SKL::CPacketHeaderSize );
        ASSERT_TRUE( ReadHeader.Opcode == HEADER_ONLY_PACKET_1_Packet::Base::Traits::Opcode );
        ASSERT_TRUE( SKL::CPacketHeaderSize == Stream.GetPosition() );
    }

    TEST( SkylakePROTOCOLTests, HeaderOnlyPacketBuildContext_API_2 )
    {
        auto Buffer = std::make_unique<uint8_t[]>( 1024 );
        SKL::BinaryStream Stream{ Buffer.get(), 1024U, 0U, false };
        ASSERT_EQ( SKL::RSuccess, HeaderOnlyMacroTest_Packet::BuildPacket( Stream.GetStream() ) );
        ASSERT_TRUE( SKL::CPacketHeaderSize == Stream.GetPosition() );

        Stream.Reset();
        ASSERT_TRUE( 0 == Stream.GetPosition() );

        const auto& Header = Stream.BuildObjectRef<SKL::PacketHeader>();
        ASSERT_TRUE( Header.Size == SKL::CPacketHeaderSize );
        ASSERT_TRUE( Header.Opcode == HeaderOnlyMacroTest_Packet::Base::Traits::Opcode );

        auto ReadHeader = Stream.ReadT<SKL::PacketHeader>();
        ASSERT_TRUE( ReadHeader.Size == SKL::CPacketHeaderSize );
        ASSERT_TRUE( ReadHeader.Opcode == HeaderOnlyMacroTest_Packet::Base::Traits::Opcode );
        ASSERT_TRUE( SKL::CPacketHeaderSize == Stream.GetPosition() );
    }

    TEST( SkylakePROTOCOLTests, FixedLengthPacketBuildContext_API )
    {
        auto Buffer = std::make_unique<uint8_t[]>( 1024 );
        SKL::BinaryStream Stream{ Buffer.get(), 1024U, 0U, false };

        FIXED_LENGTH_PACKET_1_Packet Packet{ .A = 55, .B = 23, .C = 11 };
        ASSERT_TRUE( sizeof( FIXED_LENGTH_PACKET_1_Packet ) == sizeof( uint32_t ) * 3 );
        ASSERT_EQ( SKL::RSuccess, Packet.BuildPacket( Stream.GetStream() ) );

        constexpr SKL::TPacketSize ExpectedWrittenSize{ static_cast<SKL::TPacketSize>( sizeof( FIXED_LENGTH_PACKET_1_Packet ) ) + SKL::CPacketHeaderSize };

        ASSERT_TRUE( ExpectedWrittenSize == Stream.GetPosition() );

        Stream.Reset();
        ASSERT_TRUE( 0 == Stream.GetPosition() );

        const auto& Header = Stream.BuildObjectRef<SKL::PacketHeader>();
        ASSERT_TRUE( Header.Size == ExpectedWrittenSize );
        ASSERT_TRUE( Header.Opcode == FIXED_LENGTH_PACKET_1_Packet::Base::Traits::Opcode );

        auto ReadHeader = Stream.ReadT<SKL::PacketHeader>();
        ASSERT_TRUE( ReadHeader.Size == ExpectedWrittenSize );
        ASSERT_TRUE( ReadHeader.Opcode == FIXED_LENGTH_PACKET_1_Packet::Base::Traits::Opcode );
        ASSERT_TRUE( SKL::CPacketHeaderSize == Stream.GetPosition() );

        const auto& ReadPacket = Stream.ReadT<FIXED_LENGTH_PACKET_1_Packet>();
        ASSERT_TRUE( 55 == ReadPacket.A );
        ASSERT_TRUE( 23 == ReadPacket.B );
        ASSERT_TRUE( 11 == ReadPacket.C );
        ASSERT_TRUE( ExpectedWrittenSize == Stream.GetPosition() );
    }
    
    TEST( SkylakePROTOCOLTests, FixedLengthPacketBuildContext_API_2 )
    {
        auto Buffer = std::make_unique<uint8_t[]>( 1024 );
        SKL::BinaryStream Stream{ Buffer.get(), 1024U, 0U, false };

        FixedLengthMacroTest_Packet Packet{ .A = 55, .B = 23, .C = 11 };
        ASSERT_TRUE( sizeof( FixedLengthMacroTest_Packet ) == sizeof( uint32_t ) * 3 );
        ASSERT_EQ( SKL::RSuccess, Packet.BuildPacket( Stream.GetStream() ) );

        constexpr SKL::TPacketSize ExpectedWrittenSize{ static_cast<SKL::TPacketSize>( sizeof( FixedLengthMacroTest_Packet ) ) + SKL::CPacketHeaderSize };

        ASSERT_TRUE( ExpectedWrittenSize == Stream.GetPosition() );

        Stream.Reset();
        ASSERT_TRUE( 0 == Stream.GetPosition() );

        const auto& Header = Stream.BuildObjectRef<SKL::PacketHeader>();
        ASSERT_TRUE( Header.Size == ExpectedWrittenSize );
        ASSERT_TRUE( Header.Opcode == FixedLengthMacroTest_Packet::Base::Traits::Opcode );

        auto ReadHeader = Stream.ReadT<SKL::PacketHeader>();
        ASSERT_TRUE( ReadHeader.Size == ExpectedWrittenSize );
        ASSERT_TRUE( ReadHeader.Opcode == FixedLengthMacroTest_Packet::Base::Traits::Opcode );
        ASSERT_TRUE( SKL::CPacketHeaderSize == Stream.GetPosition() );

        const auto& ReadPacket = Stream.ReadT<FixedLengthMacroTest_Packet>();
        ASSERT_TRUE( 55 == ReadPacket.A );
        ASSERT_TRUE( 23 == ReadPacket.B );
        ASSERT_TRUE( 11 == ReadPacket.C );
        ASSERT_TRUE( ExpectedWrittenSize == Stream.GetPosition() );
    }

    TEST( SkylakePROTOCOLTests, DynamicLengthPacketBuildContext_API )
    {
        auto Buffer = std::make_unique<uint8_t[]>( 1024 );
        SKL::BinaryStream Stream{ Buffer.get(), 1024U, 0U, false };

        DYNAMIC_LENGTH_PACKET_1_Packet Packet{};
        Packet.A = 55;
        Packet.B = 23;
        Packet.C = 11;

        ASSERT_EQ( SKL::RSuccess, Packet.BuildPacket( Stream.GetStream() ) );

        const SKL::TPacketSize ExpectedWrittenSize{ static_cast<SKL::TPacketSize>( Packet.CalculateBodySize() + SKL::CPacketHeaderSize ) };
        ASSERT_TRUE( ExpectedWrittenSize == Stream.GetPosition() );

        Stream.Reset();
        ASSERT_TRUE( 0 == Stream.GetPosition() );

        const auto& Header = Stream.BuildObjectRef<SKL::PacketHeader>();
        ASSERT_TRUE( Header.Size == ExpectedWrittenSize );
        ASSERT_TRUE( Header.Opcode == DYNAMIC_LENGTH_PACKET_1_Packet::Base::Traits::Opcode );

        auto ReadHeader = Stream.ReadT<SKL::PacketHeader>();
        ASSERT_TRUE( ReadHeader.Size == ExpectedWrittenSize );
        ASSERT_TRUE( ReadHeader.Opcode == DYNAMIC_LENGTH_PACKET_1_Packet::Base::Traits::Opcode );
        ASSERT_TRUE( SKL::CPacketHeaderSize == Stream.GetPosition() );

        DYNAMIC_LENGTH_PACKET_1_Packet ReadPacket;
        ReadPacket.ReadPacket( Stream.GetStream() );
        ASSERT_TRUE( 55 == ReadPacket.A );
        ASSERT_TRUE( 23 == ReadPacket.B );
        ASSERT_TRUE( 11 == ReadPacket.C );
        ASSERT_TRUE( 0 == SKL_STRLEN( ReadPacket.String, 128 ) );
    }

    TEST( SkylakePROTOCOLTests, DynamicLengthPacketBuildContext_API_2 )
    {
        auto Buffer = std::make_unique<uint8_t[]>( 1024 );
        SKL::BinaryStream Stream{ Buffer.get(), 1024U, 0U, false };

        DYNAMIC_LENGTH_PACKET_1_Packet Packet{};
        Packet.A = 55;
        Packet.B = 23;
        Packet.C = 11;
        Packet.String = "ASDASDASDASD";
        Packet.WString = L"ASDASDASDASD";

        ASSERT_EQ( SKL::RSuccess, Packet.BuildPacket( Stream.GetStream() ) );

        const SKL::TPacketSize ExpectedWrittenSize{ static_cast<SKL::TPacketSize>( Packet.CalculateBodySize() + SKL::CPacketHeaderSize ) };
        ASSERT_TRUE( ExpectedWrittenSize == Stream.GetPosition() );

        Stream.Reset();
        ASSERT_TRUE( 0 == Stream.GetPosition() );

        const auto& Header = Stream.BuildObjectRef<SKL::PacketHeader>();
        ASSERT_TRUE( Header.Size == ExpectedWrittenSize );
        ASSERT_TRUE( Header.Opcode == DYNAMIC_LENGTH_PACKET_1_Packet::Base::Traits::Opcode );

        auto ReadHeader = Stream.ReadT<SKL::PacketHeader>();
        ASSERT_TRUE( ReadHeader.Size == ExpectedWrittenSize );
        ASSERT_TRUE( ReadHeader.Opcode == DYNAMIC_LENGTH_PACKET_1_Packet::Base::Traits::Opcode );
        ASSERT_TRUE( SKL::CPacketHeaderSize == Stream.GetPosition() );

        DYNAMIC_LENGTH_PACKET_1_Packet ReadPacket;
        ReadPacket.ReadPacket( Stream.GetStream() );
        ASSERT_TRUE( 55 == ReadPacket.A );
        ASSERT_TRUE( 23 == ReadPacket.B );
        ASSERT_TRUE( 11 == ReadPacket.C );
        ASSERT_TRUE( 12 == SKL_STRLEN( ReadPacket.String, 128 ) );
        ASSERT_TRUE( 0 == SKL_STRCMP( "ASDASDASDASD", ReadPacket.String, 128 ) );
        ASSERT_TRUE( 12 == SKL_WSTRLEN( ReadPacket.WString, 128 ) );
        ASSERT_TRUE( 0 == SKL_WSTRCMP( L"ASDASDASDASD", ReadPacket.WString, 128 ) );
    }
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}