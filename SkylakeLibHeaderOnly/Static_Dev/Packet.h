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

    struct PacketHeader
    {
        TPacketSize   Size;
        TPacketOpcode Opcode;
    };

    constexpr TPacketOpcode CInvalidOpcode          = 0;
    constexpr TPacketOpcode CRoutedPacketOpcode     = 1; 
    constexpr auto          CPacketAlignment        = SKL_ALIGNMENT;
    constexpr TPacketOpcode CClientOpcodeStartValue = 2;

    constexpr TPacketSize   CPacketHeaderSize      = static_cast<TPacketSize>( sizeof( PacketHeader ) );
    constexpr TPacketSize   CPacketMaximumSize     = static_cast<TPacketSize>( std::numeric_limits<TPacketSize>::max() );
    constexpr TPacketSize   CPacketMaximumBodySize = CPacketMaximumSize - CPacketHeaderSize;
    constexpr TPacketOpcode CPacketOpcodeMaxValue  = static_cast<TPacketOpcode>( std::numeric_limits<TPacketOpcode>::max() );

    constexpr TPacketSize CCalculate_PacketMaximumUsableBodySize() noexcept
    {
        return CPacketMaximumBodySize 
            - static_cast<TPacketSize>( sizeof( PacketHeader ) ) // Routed packet wrapping header 
            - static_cast<TPacketSize>( sizeof( TEntityIdBase ) );
    }

    constexpr TPacketSize CPacketMaximumUsableBodySize = CCalculate_PacketMaximumUsableBodySize();
    static_assert( CPacketMaximumSize > CPacketHeaderSize );

    struct PacketArrayHeader
    {
        TPacketSize   Count { 0 }; //!< Count of item in the array
        TPacketOffset Offset{ 0 }; //!< Offset in the packet where the array items start
    };

    struct PacketArrayItemHeader
    {
        TPacketOffset OffsetToBase{ 0 }; //!< Offset in the packet to the base
        TPacketOffset OffsetToNext{ 0 }; //!< Offset in the packet to the next item in the array
    };
    
    template<typename T>
    struct PacketArrayItem: PacketArrayItemHeader
    {
        T Item{}; //!< Item

        SKL_FORCEINLINE T& operator->() noexcept
        {
            return Item;
        }
    };
}
