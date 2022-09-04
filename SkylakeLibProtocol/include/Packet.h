//!
//! \file Packet.h
//! 
//! \brief Packet abstraction
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    using TPacketOpcode    = uint16_t;
    using TPacketSize      = uint16_t;
    using TPacketOffset    = TPacketSize;
    using TPacketStringRef = TPacketOffset;

    constexpr TPacketOpcode CInvalidOpcode = 0;
    constexpr TPacketOpcode CRoutedPacketOpcode = 1; 

    struct PacketHeader
    {
        TPacketSize   Size;
        TPacketOpcode Opcode;
    };

    constexpr TPacketSize CPacketHeaderSize = static_cast<TPacketSize>( sizeof( PacketHeader ) );
    constexpr TPacketSize CPacketMaximumSize = static_cast<TPacketSize>( std::numeric_limits<TPacketSize>::max() );
    constexpr TPacketSize CPacketMaximumBodySize = CPacketMaximumSize - CPacketHeaderSize;

    constexpr TPacketSize CCalculate_PacketMaximumUsableBodySize() noexcept
    {
        return CPacketMaximumBodySize 
            - static_cast<TPacketSize>( sizeof( PacketHeader ) ) // Routed packet wrapping header 
            - static_cast<TPacketSize>( sizeof( TEntityIdBase ) );
    }

    constexpr TPacketSize CPacketMaximumUsableBodySize = CCalculate_PacketMaximumUsableBodySize();
    static_assert( CPacketMaximumSize > CPacketHeaderSize );
}