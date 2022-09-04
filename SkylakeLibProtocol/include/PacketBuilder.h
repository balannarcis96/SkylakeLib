//!
//! \file Packet.h
//! 
//! \brief Packet builder abstraction
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
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
                    const auto Result   { GetPacketData().WritePacket( InStream ) };

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
                    auto& Writer = *reinterpret_cast<IStreamWriter<true>*>( &InStream );
                    const auto Result{ Writer.Write( reinterpret_cast<const uint8_t*>( &GetPacketData() ), static_cast<uint32_t>( sizeof( typename Traits::PacketDataType ) ), false ) };

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

            return RFail;
        }
    
    protected:
        SKL_FORCEINLINE static constexpr void WritePacketHeader( StreamBase& InStream ) noexcept
        {
            auto& Writer{ *reinterpret_cast<IStreamWriter<true>*>( &InStream ) };

            PacketHeader& Header{ Writer.BuildObjectRef<PacketHeader>() };
            Header.Size   = 0;
            Header.Opcode = Traits::Opcode;

            Writer.Forward(  static_cast<uint32_t>( sizeof( PacketHeader ) ) );
        }
        SKL_FORCEINLINE static constexpr void WritePacketHeader( StreamBase& InStream, TPacketSize InSize ) noexcept
        {
            auto& Writer{ *reinterpret_cast<IStreamWriter<true>*>( &InStream ) };

            PacketHeader& Header{ Writer.BuildObjectRef<PacketHeader>() };
            Header.Size   = InSize;
            Header.Opcode = Traits::Opcode;

            Writer.Forward(  static_cast<uint32_t>( sizeof( PacketHeader ) ) );
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
        SKL_FORCEINLINE static void CommitPacket( StreamBase& InStream ) noexcept
        {
            auto& Header = *reinterpret_cast<PacketHeader*>( InStream.Buffer.Buffer );
            SKL_ASSERT( Header.Opcode == Traits::Opcode );
            Header.Size = InStream.GetPosition();
        }
        SKL_FORCEINLINE static void CommitPacket( StreamBase& InStream, TPacketSize InSize ) noexcept
        {
            auto& Header = *reinterpret_cast<PacketHeader*>( InStream.Buffer.Buffer );
            SKL_ASSERT( Header.Opcode == Traits::Opcode );
            Header.Size = InSize;
        }
        SKL_FORCEINLINE typename Traits::Super& GetSuper() noexcept
        {
            return *static_cast<typename Traits::Super*>( this );
        }
        SKL_FORCEINLINE const typename Traits::Super& GetSuper() const noexcept
        {
            return *static_cast<const typename Traits::Super*>( this );
        }

        template<typename InSuper, TPacketOpcode InOpcode>
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

        //! InSupper must implement this:
        // SKL_FORCEINLINE constexpr TPacketSize CalculateBodySize() const noexcept
        // {
        //     return //Caclulate need packet size
        // }

        //! InSupper must implement this:
        // SKL_FORCEINLINE constexpr RStatus WritePacket( StreamBase& InStream ) const noexcept
        // {
        //     // write dynamic packet
        // }

        //! InSupper must implement this:
        // SKL_FORCEINLINE constexpr RStatus ReadPacket( StreamBase& InStream ) const noexcept
        // {
        //     // write dynamic packet
        // }

        SKL_FORCEINLINE static constexpr RStatus BuildFromStream( StreamBase& InStream ) noexcept
        {
            return Base::GetPacketData().ReadPacket( InStream );
        }

        SKL_FORCEINLINE constexpr const InSuper& GetData() const noexcept
        {
            return *static_cast<const InSuper*>( this );
        }
        SKL_FORCEINLINE static constexpr TPacketSize CalculateNullableStringNeededSize( const char* InStr, size_t MaxCharacters ) noexcept
        {
            if( nullptr == InStr )
            {
                return 1;
            }

            const auto StrLength = SKL_STRLEN( InStr, MaxCharacters );
            return static_cast<TPacketSize>( StrLength ) + 1;
        }
        SKL_FORCEINLINE static constexpr TPacketSize CalculateStringNeededSize( const char* InStr, size_t MaxCharacters ) noexcept
        {
            SKL_ASSERT( nullptr != InStr );

            const auto StrLength = SKL_STRLEN( InStr, MaxCharacters );
            return static_cast<TPacketSize>( StrLength ) + 1;
        }
        SKL_FORCEINLINE static constexpr TPacketSize CalculateReferencedStringNeededSize( const char* InStr, size_t MaxCharacters ) noexcept
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
        SKL_FORCEINLINE static constexpr TPacketSize CalculateWStringNeededSize( const wchar_t* InStr, size_t MaxCharacters ) noexcept
        {
            SKL_ASSERT( nullptr != InStr );

            const auto StrLength = SKL_WSTRLEN( InStr, MaxCharacters );
            return ( static_cast<TPacketSize>( StrLength ) * 2 ) + 2;
        }
        SKL_FORCEINLINE static constexpr TPacketSize CalculateReferencedWStringNeededSize( const wchar_t* InStr, size_t MaxCharacters ) noexcept
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

        SKL_FORCEINLINE constexpr const InSuper& GetData() const noexcept
        {
            return *reinterpret_cast<const InSuper*>( this );
        }
        SKL_FORCEINLINE constexpr TPacketSize CalculateBodySize() const noexcept
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
}