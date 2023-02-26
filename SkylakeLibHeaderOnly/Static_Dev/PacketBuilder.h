//!
//! \file Packet.h
//! 
//! \brief Packet builder abstraction
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
//! \invariant [SKL_Packet_Invariant_1]: All offset values in the packet must be body based not buffer based
//!                                      [Body based]: all offsets are relative to right after the header
//!                                      [Buffer based]: all offsets are relative to right where the header starts
//! 
#pragma once

namespace SKL
{
    using TBuildPacketContextFlags = uint32_t;

    enum class EPacketContextFlags: TBuildPacketContextFlags
    {
        None                  = 0,
        FixedLength           = 1 << 0,
        WriteHeader           = 1 << 1,
        List                  = 1 << 2,
        HeaderOnly            = 1 << 3,
        HasCustomWriteMethod  = 1 << 4,
    };

    template<EPacketContextFlags Flag, EPacketContextFlags ...Flags> 
    constexpr TBuildPacketContextFlags PacketBuildContext_BuildFlags() noexcept
    {
        if constexpr( sizeof...( Flags ) > 0 )
        {
            return static_cast<TBuildPacketContextFlags>( Flag ) | PacketBuildContext_BuildFlags<Flags...>();
        }
        else
        {
            return static_cast<TBuildPacketContextFlags>( Flag );
        }
    }
    constexpr bool TestPacketBuildContextFlags( const TBuildPacketContextFlags Flags, const TBuildPacketContextFlags TestFlags ) noexcept
    {
        return ( Flags & TestFlags ) == TestFlags;
    }
    constexpr bool TestPacketBuildContextFlags( const TBuildPacketContextFlags Flags, const EPacketContextFlags TestFlag ) noexcept
    {
        return TestPacketBuildContextFlags( Flags, static_cast<TBuildPacketContextFlags>( TestFlag ) );
    }
    
    SKL_FORCEINLINE inline void CommitPacket( StreamBase& InStream ) noexcept
    {
        SKL_ASSERT( CPacketHeaderSize <= InStream.GetBufferLength() );

        PacketHeader& Header{ InStream.GetBufferAsTypeRef<PacketHeader>() };
        Header.Size = static_cast<TPacketSize>( InStream.GetPosition() );
    }

    SKL_FORCEINLINE inline void CommitPacket( StreamBase& InStream, TPacketSize InSize ) noexcept
    {
        SKL_ASSERT( CPacketHeaderSize <= InStream.GetBufferLength() );
        SKL_ASSERT( InSize <= InStream.GetBufferLength() );

        PacketHeader& Header{ InStream.GetBufferAsTypeRef<PacketHeader>() };
        Header.Size = static_cast<TPacketSize>( InStream.GetPosition() );
    }

    template<
        typename                 InSuper
      , TPacketOpcode            InOpcode 
      , TBuildPacketContextFlags InFlags 
      , typename                 InPacketData = InSuper>
    struct PacketBuildContext
    {
        static_assert( InOpcode != CInvalidOpcode, "Invalid Opcode!" );
        static_assert( InOpcode != CRoutedPacketOpcode, "Cannot use the RoutePacketOpcode as a packet opcode!" );

        struct Traits
        {
            using                                     Super                 = InSuper;
            using                                     MyType                = PacketBuildContext<InSuper, InOpcode, InFlags, InPacketData>;
            using                                     PacketDataType        = InPacketData;
            using                                     EntityIdType          = TEntityIdBase;
            static constexpr TBuildPacketContextFlags Flags                 = InFlags;
            static constexpr TPacketOpcode            Opcode                = InOpcode;
            static constexpr bool                     bIsFixedLength        = TestPacketBuildContextFlags( Traits::Flags, EPacketContextFlags::FixedLength );
            static constexpr bool                     bIsWriteHeader        = TestPacketBuildContextFlags( Traits::Flags, EPacketContextFlags::WriteHeader );
            static constexpr bool                     bIsList               = TestPacketBuildContextFlags( Traits::Flags, EPacketContextFlags::List );
            static constexpr bool                     bIsHeaderOnly         = TestPacketBuildContextFlags( Traits::Flags, EPacketContextFlags::HeaderOnly );
            static constexpr bool                     bHasCustomWriteMethod = TestPacketBuildContextFlags( Traits::Flags, EPacketContextFlags::HasCustomWriteMethod );
        };

        PacketBuildContext() noexcept = default;
        ~PacketBuildContext() noexcept = default;

        template<bool bForceBodyOnly = false, bool bCommitPacket = true>
        SKL_FORCEINLINE RStatus BuildPacket( StreamBase& InStream ) const noexcept
        {
            static_assert( true == Traits::bIsFixedLength || true == Traits::bHasCustomWriteMethod || true == Traits::bIsHeaderOnly );
            static_assert( false == Traits::bIsFixedLength || sizeof( typename Traits::PacketDataType ) <= CPacketMaximumUsableBodySize );

            if constexpr( true == Traits::bIsWriteHeader && false == bForceBodyOnly )
            {
                WritePacketHeader( InStream );
            }

            if constexpr( false == Traits::bIsHeaderOnly )
            {
                if constexpr( true == Traits::bHasCustomWriteMethod )
                {
                    const auto StartSize{ InStream.GetPosition() };

                    // [SKL_Packet_Invariant_1] Write through a transaction stream so all offsets are body based not packet based
                    StreamBase TransactionStream{ BinaryStreamTransaction::CreateTransactionStream( InStream ) };

                    const RStatus Result{ GetPacketData().WritePacket( TransactionStream ) };

                    SKL_ASSERT( ( static_cast<uint64_t>( TransactionStream.GetPosition() ) + InStream.GetPosition() ) <= static_cast<uint64_t>( InStream.GetBufferLength() )  )

                    BinaryStreamTransaction::CommitTransactionStream( TransactionStream, InStream );

                    SKL_ASSERT( InStream.GetPosition() - StartSize <= CPacketMaximumUsableBodySize );

                    if constexpr( bCommitPacket )
                    {
                        if( RSuccess == Result ) SKL_LIKELY
                        {
                            CommitPacket( InStream );
                        }
                    }

                    return Result;
                }
                else if constexpr( true == Traits::bIsFixedLength )
                {
                    IByteStreamObjectWriter& Writer{ IByteStreamObjectWriter::FromStreamBaseRef( InStream ) };
                    const auto               Result{ Writer.Write( reinterpret_cast<const uint8_t*>( &GetPacketData() )
                                                                 , static_cast<uint32_t>( sizeof( typename Traits::PacketDataType ) )
                                                                 , false ) };

                    if constexpr( bCommitPacket )
                    {
                        if( true == Result ) SKL_LIKELY
                        {
                            CommitPacket( InStream );
                        }
                    }

                    return RSTATUS_FROM_BOOL( Result );
                }
                else
                {
                    SKL_ASSERT_ALLWAYS_MSG( false, "Unknown write method" );
                }
            }

            //return RFail;
        }
    
    protected:
        SKL_FORCEINLINE static constexpr void WritePacketHeader( StreamBase& InStream ) noexcept
        {
            IByteStreamObjectWriter& Writer{ IByteStreamObjectWriter::FromStreamBaseRef( InStream ) };
            PacketHeader&            Header{ Writer.BuildObjectRef<PacketHeader>() };

            Header.Size   = 0;
            Header.Opcode = Traits::Opcode;

            Writer.Forward( static_cast<uint32_t>( sizeof( PacketHeader ) ) );
        }
        SKL_FORCEINLINE static constexpr void WritePacketHeader( StreamBase& InStream, TPacketSize InSize ) noexcept
        {
            IByteStreamObjectWriter& Writer{ IByteStreamObjectWriter::FromStreamBaseRef( InStream ) };
            PacketHeader&            Header{ Writer.BuildObjectRef<PacketHeader>() };

            Header.Size   = InSize;
            Header.Opcode = Traits::Opcode;

            Writer.Forward( static_cast<uint32_t>( sizeof( PacketHeader ) ) );
        }
        SKL_FORCEINLINE const typename Traits::PacketDataType& GetPacketData() const noexcept
        {
            SKL_ASSERT( false == Traits::bIsHeaderOnly );
            return GetSuper().GetData();
        }
        SKL_FORCEINLINE constexpr TPacketSize CalculatedNeededSize() const noexcept
        {
           if constexpr( true == Traits::bIsHeaderOnly )
           {
                return CPacketHeaderSize;
           }
           else
           {
                const auto BodySize{ GetSuper().CalculateBodySize() };
                return BodySize + CPacketHeaderSize;
           }
        }
        SKL_FORCEINLINE typename Traits::Super& GetSuper() noexcept
        {
            return *static_cast<typename Traits::Super*>( this );
        }
        SKL_FORCEINLINE const typename Traits::Super& GetSuper() const noexcept
        {
            return *static_cast<const typename Traits::Super*>( this );
        }

        template<typename _InSuper, TPacketOpcode _InOpcode>
        friend struct HeaderOnlyPacketBuildContext;
    };

    template<
        typename                   InSuper
        , TPacketOpcode            InOpcode
        , TBuildPacketContextFlags InAdditionalFlags = static_cast<TBuildPacketContextFlags>( EPacketContextFlags::None )
        , typename                 InPacketData = InSuper>
    struct DynamicLengthPacketBuildContext 
        : PacketBuildContext<
            InSuper
          , InOpcode
          , PacketBuildContext_BuildFlags<EPacketContextFlags::WriteHeader, EPacketContextFlags::HasCustomWriteMethod, static_cast<EPacketContextFlags>( InAdditionalFlags )>()>
    {
        using Base = PacketBuildContext<
            InSuper
            , InOpcode
            , PacketBuildContext_BuildFlags<EPacketContextFlags::WriteHeader, EPacketContextFlags::HasCustomWriteMethod, static_cast<EPacketContextFlags>( InAdditionalFlags )>()>;

        virtual ~DynamicLengthPacketBuildContext() = default;

        //! Calculate the body size for this packet
        virtual TPacketSize CalculateBodySize() const noexcept = 0;

        //! Custom write method
        virtual RStatus WritePacket( StreamBase& InStream ) const noexcept = 0;

        //! Custom read method
        virtual RStatus ReadPacket( StreamBase& InStream ) noexcept = 0;
        
        //! Is this packet broadcastable
        SKL_FORCEINLINE SKL_NODISCARD bool IsBroadcastable() const noexcept 
        { 
            const TPacketSize PacketBodySize              { CalculateBodySize() };
            const TPacketSize SpaceLeftInTheTransferBuffer{ CPacketMaximumUsableBodySize - PacketBodySize };

            return SpaceLeftInTheTransferBuffer >= CMinimumMinSlackNeededByBroadcastablePacket;
        }
        
        //! How many target entity ids can be put alongside this packet in the transfer buffer
        SKL_FORCEINLINE SKL_NODISCARD TPacketSize GetNoOfMaxBroadcastTargetEntities() const noexcept 
        { 
            const TPacketSize PacketBodySize              { CalculateBodySize() };
            const TPacketSize SpaceLeftInTheTransferBuffer{ CPacketMaximumUsableBodySize - PacketBodySize };

            return SpaceLeftInTheTransferBuffer / static_cast<TPacketSize>( sizeof( TEntityIdBase ) );
        }

        SKL_FORCEINLINE SKL_NODISCARD static constexpr RStatus BuildFromStream( StreamBase& InStream ) noexcept
        {
            return Base::GetPacketData().ReadPacket( InStream );
        }

        SKL_FORCEINLINE SKL_NODISCARD constexpr const InSuper& GetData() const noexcept
        {
            return *static_cast<const InSuper*>( this );
        }
        SKL_FORCEINLINE SKL_NODISCARD static constexpr TPacketSize CalculateNullableStringNeededSize( const char* InStr, size_t MaxCharacters ) noexcept
        {
            if( nullptr == InStr )
            {
                return 1;
            }

            const auto StrLength = SKL_STRLEN( InStr, MaxCharacters );
            return static_cast<TPacketSize>( StrLength ) + 1;
        }
        SKL_FORCEINLINE SKL_NODISCARD static constexpr TPacketSize CalculateStringNeededSize( const char* InStr, size_t MaxCharacters ) noexcept
        {
            SKL_ASSERT( nullptr != InStr );

            const auto StrLength = SKL_STRLEN( InStr, MaxCharacters );
            return static_cast<TPacketSize>( StrLength ) + 1;
        }
        SKL_FORCEINLINE SKL_NODISCARD static constexpr TPacketSize CalculateReferencedStringNeededSize( const char* InStr, size_t MaxCharacters ) noexcept
        {
            const auto StrLength = SKL_STRLEN( InStr, MaxCharacters );
            return static_cast<TPacketSize>( StrLength + 1 + static_cast<TPacketSize>( sizeof( TPacketStringRef ) ) );
        }

        SKL_FORCEINLINE static constexpr TPacketSize CalculateNullableWStringNeededSize( const wchar_t* InStr, size_t MaxCharacters ) noexcept
        {
            if( nullptr == InStr )
            {
                return 2;
            }

            const auto StrLength = SKL_WSTRLEN( InStr, MaxCharacters );
            return ( static_cast<TPacketSize>( StrLength ) * 2 ) + 2;
        }
        SKL_FORCEINLINE SKL_NODISCARD static constexpr TPacketSize CalculateWStringNeededSize( const wchar_t* InStr, size_t MaxCharacters ) noexcept
        {
            SKL_ASSERT( nullptr != InStr );

            const auto StrLength = SKL_WSTRLEN( InStr, MaxCharacters );
            return ( static_cast<TPacketSize>( StrLength ) * 2 ) + 2;
        }
        SKL_FORCEINLINE SKL_NODISCARD static constexpr TPacketSize CalculateReferencedWStringNeededSize( const wchar_t* InStr, size_t MaxCharacters ) noexcept
        {
            const auto StrLength = SKL_WSTRLEN( InStr, MaxCharacters );
            return static_cast<TPacketSize>( ( StrLength * 2 ) + 2 + static_cast<TPacketSize>( sizeof( TPacketStringRef ) ) );
        }
    };

    template<
        typename                   InSuper
        , TPacketOpcode            InOpcode>
    struct FixedLengthPacketBuildContext 
        : PacketBuildContext<InSuper, InOpcode, PacketBuildContext_BuildFlags<EPacketContextFlags::WriteHeader, EPacketContextFlags::FixedLength>()>
    {
        using Base = PacketBuildContext<InSuper
                                      , InOpcode
                                      , PacketBuildContext_BuildFlags<EPacketContextFlags::WriteHeader, EPacketContextFlags::FixedLength>()>;

        //! Is this packet broadcastable
        SKL_FORCEINLINE SKL_NODISCARD constexpr bool IsBroadcastable() const noexcept 
        { 
            constexpr TPacketSize PacketBodySize              { CalculateBodySize() };
            constexpr TPacketSize SpaceLeftInTheTransferBuffer{ CPacketMaximumUsableBodySize - PacketBodySize };

            return SpaceLeftInTheTransferBuffer >= CMinimumMinSlackNeededByBroadcastablePacket;
        }
        
        //! How many target entity ids can be put alongside this packet in the transfer buffer
        SKL_FORCEINLINE SKL_NODISCARD constexpr TPacketSize GetNoOfMaxBroadcastTargetEntities() const noexcept 
        { 
            constexpr TPacketSize PacketBodySize              { CalculateBodySize() };
            constexpr TPacketSize SpaceLeftInTheTransferBuffer{ CPacketMaximumUsableBodySize - PacketBodySize };

            return SpaceLeftInTheTransferBuffer / static_cast<TPacketSize>( sizeof( TEntityIdBase ) );
        }

        SKL_FORCEINLINE SKL_NODISCARD constexpr const InSuper& GetData() const noexcept
        {
            return *reinterpret_cast<const InSuper*>( this );
        }
        SKL_FORCEINLINE SKL_NODISCARD constexpr TPacketSize CalculateBodySize() const noexcept
        {
            return static_cast<TPacketSize>( sizeof( InSuper ) );
        }
    };

    template<
        typename                   InSuper
        , TPacketOpcode            InOpcode>
    struct HeaderOnlyPacketBuildContext 
        : protected PacketBuildContext<
            InSuper
          , InOpcode
          , PacketBuildContext_BuildFlags<EPacketContextFlags::WriteHeader, EPacketContextFlags::HeaderOnly>()>
    {
        using Base = PacketBuildContext<InSuper
                                      , InOpcode
                                      , PacketBuildContext_BuildFlags<EPacketContextFlags::WriteHeader, EPacketContextFlags::HeaderOnly>()>;

        HeaderOnlyPacketBuildContext() = delete;
        
        //! Is this packet broadcastable
        SKL_FORCEINLINE SKL_NODISCARD constexpr bool IsBroadcastable() const noexcept 
        { 
            constexpr TPacketSize PacketBodySize              { CalculateBodySize() };
            constexpr TPacketSize SpaceLeftInTheTransferBuffer{ CPacketMaximumUsableBodySize - PacketBodySize };

            return SpaceLeftInTheTransferBuffer >= CMinimumMinSlackNeededByBroadcastablePacket;
        }
        
        //! How many target entity ids can be put alongside this packet in the transfer buffer
        SKL_FORCEINLINE SKL_NODISCARD constexpr TPacketSize GetNoOfMaxBroadcastTargetEntities() const noexcept 
        { 
            constexpr TPacketSize PacketBodySize              { CalculateBodySize() };
            constexpr TPacketSize SpaceLeftInTheTransferBuffer{ CPacketMaximumUsableBodySize - PacketBodySize };

            return SpaceLeftInTheTransferBuffer / static_cast<TPacketSize>( sizeof( TEntityIdBase ) );
        }

        SKL_FORCEINLINE static constexpr RStatus BuildPacket( StreamBase& InStream ) noexcept
        {
            Base::WritePacketHeader( InStream, CalculateBodySize() );
            return RSuccess;
        }
        SKL_FORCEINLINE static constexpr TPacketSize CalculateBodySize() noexcept
        {
            return CPacketHeaderSize;
        }
    };

#define DEFINE_NAMED_HEADER_ONLY_PACKET( Name, PacketOpcode ) \
    struct Name##_Packet : SKL::HeaderOnlyPacketBuildContext<Name##_Packet, static_cast<SKL::TPacketOpcode>( PacketOpcode )> {};

#define DEFINE_NAMED_FIXED_LENGTH_PACKET( Name, PacketOpcode, Body ) \
    struct Name##_Packet : SKL::FixedLengthPacketBuildContext<Name##_Packet, static_cast<SKL::TPacketOpcode>( PacketOpcode )> \
    Body; \
    static_assert( alignof( Name##_Packet ) <= SKL::CPacketAlignment, "Packet [" #Name "_Packet] Must be (max) aligned to CPacketAlignment bytes" ); 

#define DEFINE_NAMED_DYNAMIC_PACKET( Name, PacketOpcode, Body ) \
    struct Name##_Packet final: SKL::DynamicLengthPacketBuildContext<Name##_Packet, static_cast<SKL::TPacketOpcode>( PacketOpcode )> \
    Body
    
#define DEFINE_HEADER_ONLY_PACKET( PacketOpcode ) \
    struct PacketOpcode##_Packet : SKL::HeaderOnlyPacketBuildContext<PacketOpcode##_Packet, static_cast<SKL::TPacketOpcode>( PacketOpcode )> {};

#define DEFINE_FIXED_LENGTH_PACKET( PacketOpcode, Body ) \
    struct PacketOpcode##_Packet : SKL::FixedLengthPacketBuildContext<PacketOpcode##_Packet, static_cast<SKL::TPacketOpcode>( PacketOpcode )> \
    Body; \
    static_assert( alignof( PacketOpcode##_Packet ) <= SKL::CPacketAlignment, "Packet [" #PacketOpcode "_Packet] Must be (max) aligned to CPacketAlignment bytes" ); 

#define DEFINE_DYNAMIC_PACKET( PacketOpcode, Body ) \
    struct PacketOpcode##_Packet : SKL::DynamicLengthPacketBuildContext<PacketOpcode##_Packet, static_cast<SKL::TPacketOpcode>( PacketOpcode )> \
    Body

#define PAKCET_CalculateBodySize() \
    SKL_FORCEINLINE SKL::TPacketSize CalculateBodySize() const noexcept 

#define PAKCET_WritePacket() \
    SKL_FORCEINLINE SKL::RStatus WritePacket( SKL::StreamBase& InStream ) const noexcept 

#define PAKCET_ReadPacket() \
    SKL_FORCEINLINE SKL::RStatus ReadPacket( SKL::StreamBase& InStream ) noexcept 

#define DEFINE_DYNAMIC_PACKET_EX( Name, PacketOpcode, Prerequisites, CalculateRequiredSizeStub, WriteStub, ReadStub ) \
    struct Name##_Packet final : SKL::DynamicLengthPacketBuildContext<Name##_Packet, static_cast<SKL::TPacketOpcode>( PacketOpcode )> \
    {   \
        struct \
        Prerequisites; \
        SKL::TPacketSize CalculateBodySize() const noexcept override \
        CalculateRequiredSizeStub \
        SKL::RStatus WritePacket( SKL::StreamBase& InStream ) const noexcept override \
        WriteStub \
        SKL::RStatus ReadPacket( SKL::StreamBase& InStream ) noexcept override \
        ReadStub \
    }; \
    static_assert( alignof( Name##_Packet ) <= SKL::CPacketAlignment, "Packet [" #Name "_Packet] Must be (max) aligned to CPacketAlignment bytes" ); 
}
