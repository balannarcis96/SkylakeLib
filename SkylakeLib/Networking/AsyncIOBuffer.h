//!
//! \file AsyncIOBuffer.h
//! 
//! \brief Async IO buffer abstraction
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    constexpr size_t CTaskMinimumSize = 1;
    
    //! 
    //! \brief Single level dispatched async IO task
    //! 
    //! \important Do not temper! Any modifications that will affect sizeof(IAsyncIOTask) will break the task abstraction.
    //! 
    struct IAsyncIOTask
    {   
        using TDispatchFunctionPtr = void( ASD_CDECL* )( IAsyncIOTask&, uint32_t ) noexcept;
        using TDispatchProto       = ASD::UniqueFunctorWrapper<CTaskMinimumSize, TDispatchFunctionPtr>;

        IAsyncIOTask( IBuffer BufferInterface ) noexcept : Stream { 0, BufferInterface.Length, BufferInterface.Buffer, true }
        {
            OsOpaqueType.Reset();
        }
        ~IAsyncIOTask() noexcept 
        {
            Clear();
        }

        //! Dispatch this task
        SKL_FORCEINLINE void Dispatch( uint32_t NumberOfBytesTransferred ) noexcept
        {
            SKL_ASSERT( false == CastSelfToProto().IsNull() );
            CastSelfToProto().Dispatch( *this, NumberOfBytesTransferred );
        }
        
        //! Is this task valid
        SKL_FORCEINLINE bool IsNull() const noexcept
        {
            return CastSelfToProto().IsNull();
        }

        //! Clear the underlying functor
        SKL_FORCEINLINE void Clear() noexcept
        {
            CastSelfToProto().Destroy();
        }

        //! Get the interface to the internal buffer reference
        SKL_FORCEINLINE const IBuffer& GetInterface() const noexcept 
        { 
            return Stream.Buffer;
        }

        //! Get the interface to the internal buffer reference
        SKL_FORCEINLINE IBuffer& GetInterface() noexcept 
        { 
            return Stream.Buffer;
        }

        //! Cast self to Os opaque type
        SKL_FORCEINLINE AsyncIOOpaqueType* ToOSOpaqueObject() noexcept
        {
            return reinterpret_cast<AsyncIOOpaqueType*>( this );
        }

        //! Construct a new stream interface for this async IO buffer
        SKL_FORCEINLINE BinaryStreamInterface GetStreamInterface() noexcept
        {
            return BinaryStreamInterface{ &Stream };
        }

        //! Construct a new binary stream interface for this async IO buffer
        SKL_FORCEINLINE IBinaryStream<true>* GetStream() noexcept
        {
            return IBinaryStream<true>::FromStreamBase( Stream );
        }

        //! Construct stream transaction interface into this buffer at the current position
        SKL_FORCEINLINE BinaryStreamTransaction NewTransaction() noexcept
        {
            return BinaryStreamTransaction{ Stream };
        }

        //! Has reached buffer end (is end of buffer)
        SKL_FORCEINLINE bool IsEOS() const noexcept
        {
            return Stream.Position == Stream.Buffer.Length;
        }

        //! Get current stream position
        SKL_FORCEINLINE uint32_t GetPosition() const noexcept
        {
            return Stream.Position;
        }

        //! Set current stream position
        SKL_FORCEINLINE void SetPosition( uint32_t InPosition ) noexcept
        {
            SKL_ASSERT( InPosition < Stream.Buffer.Length );
            Stream.Position = InPosition;
        }

        //! Forward current stream position by InAmount
        SKL_FORCEINLINE void Forward( uint32_t InAmount ) noexcept
        {
            SKL_ASSERT( Stream.Position + InAmount <= Stream.Buffer.Length );
            Stream.Position += InAmount;
        }

    protected:
        const TDispatchProto& CastSelfToProto() const noexcept
        {
            return *reinterpret_cast<const TDispatchProto*>( 
                reinterpret_cast<const uint8_t*>( this ) + sizeof( IAsyncIOTask )
            );
        }

        TDispatchProto& CastSelfToProto() noexcept
        {
            return *reinterpret_cast<TDispatchProto*>( 
                reinterpret_cast<uint8_t*>( this ) + sizeof( IAsyncIOTask )
            );
        }

        AsyncIOOpaqueType OsOpaqueType; //!< Opaque object needed internally by the OS to perform the async IO operation
        StreamBase        Stream;       //!< Cached buffer data and manipulation info
    };

    template<uint32_t TBufferSize, size_t TCompletionTaskSize = 16> 
    struct AsyncIOBuffer : IAsyncIOTask
    {
        static constexpr uint32_t BufferSize       = TBufferSize;
        static constexpr size_t CompletionTaskSize = TCompletionTaskSize;

        static_assert( CompletionTaskSize % 8 == 0 );

        using TDispatch = ASD::UniqueFunctorWrapper<CompletionTaskSize, typename IAsyncIOTask::TDispatchFunctionPtr>;
        
        AsyncIOBuffer() noexcept 
            : IAsyncIOTask( IBuffer{ BufferSize, Buffer } ), OnDispatch{} 
        {
            SKL_ASSERT( 0 == ( reinterpret_cast<uint64_t>( Buffer ) % SKL_ALIGNMENT ) );
        }
        
        AsyncIOBuffer( IBuffer InInterface ) noexcept 
            : IAsyncIOTask( InInterface ), OnDispatch{} 
        {
            SKL_ASSERT( 0 == ( reinterpret_cast<uint64_t>( Buffer ) % SKL_ALIGNMENT ) );
        }

        ~AsyncIOBuffer() noexcept = default;

        //! Set the functor to be executed when the async IO operation is completed [void( ASD_CDECL* )( IAsyncIOTask&, uint32_t ) noexcept]
        template<typename TFunctor>
        SKL_FORCEINLINE void operator+=( TFunctor&& InFunctor ) noexcept
        {
            OnDispatch += std::forward<TFunctor>( InFunctor );
        }

        //! Set the functor to be executed when the async IO operation is completed [void( ASD_CDECL* )( IAsyncIOTask&, uint32_t ) noexcept]
        template<typename TFunctor>
        SKL_FORCEINLINE void SetCompletionHandler( TFunctor&& InFunctor ) noexcept
        {
            OnDispatch += std::forward<TFunctor>( InFunctor );
        }

    protected:
        TDispatch OnDispatch;         //!< The functor to dispatch when the async IO operation is completed
        uint8_t   Buffer[BufferSize]; //!< The buffer to carry the data
    };

#define SKL_ASYNCIO_BUFFER_TRANSACTION( BufferPtr )          \
    if( SKL::BinaryStreamTransaction Transaction{ BufferPtr->NewTransaction() }; true )
}

namespace SKL
{
    struct AsyncNetBuffer_Usage1
    {
        static constexpr size_t CPaddingSize    = 8U;                           //!< [Padding 1]
        static constexpr size_t CStateSize      = 4U;                           //!< [State]
        static constexpr size_t CHeaderSize     = CPacketHeaderSize;            //!< [Header]
        static constexpr size_t CPacketBodySize = CPacketMaximumUsableBodySize; //!< [Packet Body]
        
        static constexpr size_t CStateOffset        = CPaddingSize;
        static constexpr size_t CPacketHeaderOffset = CPaddingSize + CStateSize;
        static constexpr size_t CPacketBodyOffset   = CPacketHeaderOffset + CHeaderSize;
        
        static consteval size_t GetTotalPaddingSize() noexcept
        {
            return CPaddingSize 
                 + CStateSize;
        }

        static consteval size_t GetTotalBufferSize() noexcept
        {
            return CPaddingSize 
                 + CStateSize
                 + CHeaderSize
                 + CPacketBodySize;
        }

        //! Get the size of the buffer portion that we can receive the packet into
        static consteval size_t GetPacketBufferSize() noexcept
        {
            return CHeaderSize + CPacketBodySize;
        }
    };
    struct AsyncNetBuffer_Usage2
    {
        static constexpr size_t CRHeaderSize    = CPacketHeaderSize;            //!< [R Header]
        static constexpr size_t CEntityIdSize   = 8U;                           //!< [EntityId]
        static constexpr size_t CHeaderSize     = CPacketHeaderSize;            //!< [Header]
        static constexpr size_t CPacketBodySize = CPacketMaximumUsableBodySize; //!< [Packet Body]
        
        static constexpr size_t CRHeaderOffset      = 0U; 
        static constexpr size_t CEntityIdOffset     = CPacketHeaderSize; 
        static constexpr size_t CPacketHeaderOffset = CRHeaderSize + CEntityIdSize;
        static constexpr size_t CPacketBodyOffset   = CPacketHeaderOffset + CHeaderSize;
        
        static consteval size_t GetTotalPaddingSize() noexcept
        {
            return CRHeaderSize 
                 + CEntityIdSize;
        }

        static consteval size_t GetTotalBufferSize() noexcept
        {
            return CRHeaderSize 
                 + CEntityIdSize
                 + CHeaderSize
                 + CPacketBodySize;
        }
    };
    
    static_assert( AsyncNetBuffer_Usage1::GetTotalPaddingSize() == AsyncNetBuffer_Usage2::GetTotalPaddingSize() );
    static_assert( AsyncNetBuffer_Usage1::GetTotalBufferSize() == AsyncNetBuffer_Usage2::GetTotalBufferSize() );
    static_assert( AsyncNetBuffer_Usage1::CPacketHeaderOffset == AsyncNetBuffer_Usage2::CPacketHeaderOffset );
    static_assert( AsyncNetBuffer_Usage1::CPacketBodyOffset == AsyncNetBuffer_Usage2::CPacketBodyOffset );
    static_assert( CPacketMaximumSize == AsyncNetBuffer_Usage2::GetTotalBufferSize() );

    static constexpr size_t CAsyncNetBufferTotalBufferSize = AsyncNetBuffer_Usage1::GetTotalBufferSize();

    template<size_t CompletionTaskSize = 16>
    struct AsyncNetBuffer: public AsyncIOBuffer<CAsyncNetBufferTotalBufferSize, CompletionTaskSize>
    {
        using MyType = AsyncNetBuffer<CompletionTaskSize>;
        using Base   = AsyncIOBuffer<CAsyncNetBufferTotalBufferSize, CompletionTaskSize>;

        static constexpr size_t CPacketHeaderOffset = AsyncNetBuffer_Usage1::CPacketHeaderOffset;
        static constexpr size_t CPacketBodyOffset   = AsyncNetBuffer_Usage1::CPacketBodyOffset;

        static constexpr uint32_t CPacketReceiveHeaderState = 0;
        static constexpr uint32_t CPacketReceiveBodyState   = 1;
        static constexpr uint32_t CPacketSendState          = 2;

        AsyncNetBuffer() noexcept 
            : Base()
        {
            PacketHeader& RHeader{ GetRoutingHeader() };
            RHeader.Opcode = CRoutedPacketOpcode;
            RHeader.Size   = 0U;
        }
        ~AsyncNetBuffer() noexcept = default;

        // Buffer description
        // --------------------------------------------------------------------------------------------
        // | Buffer    | [00 00 00 00] [00 00 00 00] [00 00 00 00] [00 00 00 00] [00 00 00 00 00 ...] |
        // --------------------------------------------------------------------------------------------
        // | Usage 1   |                  Padding                 |    Header   |     Packet Body     |
        // --------------------------------------------------------------------------------------------
        // | Usage 2   |   R Header   |         EntityId          |    Header   |     Packet Body     |
        // --------------------------------------------------------------------------------------------
        // | Size      |   4 bytes    |   4 bytes   |   4 bytes   |   4 bytes   |      65519 bytes    |
        // --------------------------------------------------------------------------------------------
        // | Alignment |      8       |      4      |      8      |      4      |          8          |
        // --------------------------------------------------------------------------------------------
        // |<------------------------------------- 65536 bytes -------------------------------------->|
        // --------------------------------------------------------------------------------------------
        // Usage 1: Receive/Send buffer
        //        [Padding]     - Place holder
        //        [Header]      - Packet header
        //        [Packet Body] - Packet body (must be 8 bytes aligned)
        // Usage 2: Route buffer
        //        [R Header]    - Routing header
        //        [EntityId]    - Target entityId (used to identify the packet target)
        //        [Header]      - Packet header
        //        [Packet Body] - Packet body (must be 8 bytes aligned)
        
        //! Get the entity id [EntityId]
        SKL_FORCEINLINE SKL_NODISCARD TEntityIdBase GetEntityId() const noexcept
        {
            return *reinterpret_cast<const TEntityIdBase*>( this->Buffer + AsyncNetBuffer_Usage2::CEntityIdOffset );
        }
        
        //! Get the buffer pointer starting at the packet header 
        SKL_FORCEINLINE SKL_NODISCARD uint8_t* GetRoutingBuffer() noexcept
        {
            return this->Buffer + AsyncNetBuffer_Usage2::CRHeaderOffset;
        }
        
        //! Get the buffer pointer starting at the packet header 
        SKL_FORCEINLINE SKL_NODISCARD uint8_t* GetPacketBuffer() noexcept
        {
            return this->Buffer + CPacketHeaderOffset;
        }
        
        //! Get the buffer pointer starting at the packet body 
        SKL_FORCEINLINE SKL_NODISCARD uint8_t* GetPacketBodyBuffer() noexcept
        {
            return this->Buffer + CPacketBodyOffset;
        }
        
        //! Get the buffer pointer starting at the packet routing body 
        SKL_FORCEINLINE SKL_NODISCARD uint8_t* GetRoutingBodyBuffer() noexcept
        {
            return this->Buffer + AsyncNetBuffer_Usage2::CEntityIdOffset;
        }
        
        //! Get the buffer pointer starting at the packet header 
        SKL_FORCEINLINE SKL_NODISCARD const uint8_t* GetRoutingBuffer() const noexcept
        {
            return this->Buffer + AsyncNetBuffer_Usage2::CRHeaderOffset;
        }
        
        //! Get the buffer pointer starting at the packet header 
        SKL_FORCEINLINE SKL_NODISCARD const uint8_t* GetPacketBuffer() const noexcept
        {
            return this->Buffer + CPacketHeaderOffset;
        }
        
        //! Get the buffer pointer starting at the packet body 
        SKL_FORCEINLINE SKL_NODISCARD const uint8_t* GetPacketBodyBuffer() const noexcept
        {
            return this->Buffer + CPacketBodyOffset;
        }
        
        //! Get the buffer pointer starting at the packet routing body 
        SKL_FORCEINLINE SKL_NODISCARD const uint8_t* GetRoutingBodyBuffer() const noexcept
        {
            return this->Buffer + AsyncNetBuffer_Usage2::CEntityIdOffset;
        }
        
        //! Get the packet header
        SKL_FORCEINLINE SKL_NODISCARD PacketHeader& GetPacketHeader() noexcept
        {
            return *reinterpret_cast<PacketHeader*>( GetPacketBuffer() );
        }
    
        //! Get the packet header
        SKL_FORCEINLINE SKL_NODISCARD const PacketHeader& GetPacketHeader() const noexcept
        {
            return *reinterpret_cast<const PacketHeader*>( GetPacketBuffer() );
        }
    
        //! Get the routing header
        SKL_FORCEINLINE SKL_NODISCARD PacketHeader& GetRoutingHeader() noexcept
        {
            return *reinterpret_cast<PacketHeader*>( this->Buffer + AsyncNetBuffer_Usage2::CRHeaderOffset );
        }
    
        //! Get the routing header
        SKL_FORCEINLINE SKL_NODISCARD const PacketHeader& GetRoutingHeader() const noexcept
        {
            return *reinterpret_cast<const PacketHeader*>( this->Buffer + AsyncNetBuffer_Usage2::CRHeaderOffset );
        }
    
        //! Get the size of the entire buffer of any instance of this type
        SKL_FORCEINLINE SKL_NODISCARD consteval static uint32_t GetTotalBufferSize() noexcept
        {
            return AsyncNetBuffer_Usage1::GetTotalBufferSize();
        }

        //! Get the size of the buffer portion that we can receive the packet into
        SKL_FORCEINLINE SKL_NODISCARD consteval static uint32_t GetPacketBufferSize() noexcept
        {
            return AsyncNetBuffer_Usage1::GetPacketBufferSize();
        }

        //! Get the size of the buffer portion that we can receive the packet body into
        SKL_FORCEINLINE SKL_NODISCARD consteval static uint32_t GetPacketBodyBufferSize() noexcept
        {
            return AsyncNetBuffer_Usage1::CPacketBodySize;
        }

        //! Cast the packet body buffer to the give type
        template<typename TPacketBody>
        SKL_FORCEINLINE SKL_NODISCARD TPacketBody& CastToPacketType() noexcept
        {
            return *reinterpret_cast<TPacketBody*>( GetPacketBodyBuffer() );
        }
    
        //! get the number of bytes currently received in this buffer
        SKL_FORCEINLINE SKL_NODISCARD uint32_t GetCurrentlyReceivedByteCount() const noexcept { return this->Stream.GetPosition(); }

        //! Prepare the buffer for receiving a max of GetPacketBufferSize() bytes
        SKL_FORCEINLINE void PrepareForReceiving() noexcept
        {
            this->Stream.Position      = 0;
            this->Stream.Buffer.Buffer = GetPacketBuffer();
            this->Stream.Buffer.Length = GetPacketBufferSize();
        }
        
        //! Prepare the buffer for receiving a max of CPacketHeaderSize bytes and set State to CPacketReceiveHeaderState
        SKL_FORCEINLINE void PrepareForReceivingHeader() noexcept
        {
            this->Stream.Position      = 0;
            this->Stream.Buffer.Buffer = GetPacketBuffer();
            this->Stream.Buffer.Length = CPacketHeaderSize;
        }
        
        //! Prepare the buffer for receiving the packet body 
        //! \remarks to be used after a successful header receive has been performed
        SKL_FORCEINLINE void PrepareForReceivingBody() noexcept
        {
            this->Stream.Position       = CPacketHeaderSize;
            this->Stream.Buffer.Buffer += CPacketHeaderSize;
            this->Stream.Buffer.Length  = GetPacketHeader().Size - CPacketHeaderSize;
        }
        
        //! Confirm exact n bytes as received into the packet buffer
        //! \returns <bool hasReceivedWholePacket, bool bProcessedSuccessfully>
        SKL_NODISCARD std::pair<bool, bool> ConfirmReceivedExactAmmount( uint32_t NoOfBytesTransferred ) noexcept
        {
            SKL_ASSERT( NoOfBytesTransferred <= CPacketMaximumSize );

            // acknowledge received bytes count
            this->Stream.Position      += NoOfBytesTransferred;
            this->Stream.Buffer.Buffer += NoOfBytesTransferred;
            this->Stream.Buffer.Length -= NoOfBytesTransferred;

            // get the total received count
            const uint32_t CurrentlyReceived{ this->Stream.Position };

            // we must receive at least the header
            if( CurrentlyReceived < CPacketHeaderSize || CurrentlyReceived >= static_cast<uint32_t>( CPacketMaximumUsableUserPacketSize ) ) SKL_UNLIKELY
            {
                return { false, true };
            }

            PacketHeader Header;
            if( HasValidRoutingData() )
            {
                Header = GetRoutingHeader();
            }
            else
            {
                Header = GetPacketHeader();
            }
            
            // do we have the whole packet received
            const uint32_t ExpectedPacketSize  { Header.Size };
            const bool     bHasReceivedExpected{ CurrentlyReceived == ExpectedPacketSize };

            return { bHasReceivedExpected, CurrentlyReceived <= ExpectedPacketSize };
        }
        
        //! Confirm at most n bytes as received into the packet buffer
        //! \returns <bool hasReceivedWholePacket, bool bProcessedSuccessfully>
        SKL_NODISCARD std::pair<bool, bool> ConfirmReceivedAmmount( uint32_t NoOfBytesTransferred, StreamBase& OutExtraData ) noexcept
        {
            SKL_ASSERT( NoOfBytesTransferred <= CPacketMaximumSize );

            // acknowledge received bytes count
            this->Stream.Position      += NoOfBytesTransferred;
            this->Stream.Buffer.Buffer += NoOfBytesTransferred;
            this->Stream.Buffer.Length -= NoOfBytesTransferred;

            // get the total received count
            const uint32_t CurrentlyReceived{ this->Stream.Position };

            // we must receive at least the header
            if( CurrentlyReceived < CPacketHeaderSize || CurrentlyReceived >= static_cast<uint32_t>( CPacketMaximumUsableUserPacketSize ) ) SKL_UNLIKELY
            {
                return { false, true };
            }
                
            PacketHeader Header;
            if( HasValidRoutingData() )
            {
                Header = GetRoutingHeader();
            }
            else
            {
                Header = GetPacketHeader();
            }
            
            // do we have the whole packet received
            const uint32_t ExpectedPacketSize  { Header.Size };
            const bool     bHasReceivedExpected{ CurrentlyReceived >= ExpectedPacketSize };

            // if we have extra data received copy it over in the extra data stream
            if( CurrentlyReceived > ExpectedPacketSize )
            {
                const uint32_t           ExtraDataSize{ CurrentlyReceived - ExpectedPacketSize };
                IByteStreamObjectWriter& Writer       { IByteStreamObjectWriter::FromStreamBaseRef( OutExtraData ) };

                if( false == Writer.Write( GetPacketBuffer() + ExpectedPacketSize, ExtraDataSize, false ) ) SKL_UNLIKELY
                {
                    SKLL_WRN_FMT( "ConfirmReceivedAmmount( ... ) Failed to copy extra data %u", ExtraDataSize );
                    return { false, false };
                }

                PrepareForPacketProcessing();
            }

            return { bHasReceivedExpected, true };
        }
        
        //! After a packet was received completely, call this method to prepare the buffer to be used in the packet's processing
        SKL_FORCEINLINE void PrepareForPacketProcessing() noexcept
        {
            this->Stream.Position      = 0;
            this->Stream.Buffer.Buffer = GetPacketBuffer();
            this->Stream.Buffer.Length = GetPacketHeader().Size;
        }

        //! Prepare this buffer for sending the packet 
        SKL_FORCEINLINE void PrepareForSending() noexcept
        {
            this->Stream.Buffer.Buffer = GetPacketBuffer();
            this->Stream.Buffer.Length = this->Stream.Position;
        }
        
        //! Prepare this buffer for sending the packet with specific bytes count
        SKL_FORCEINLINE void PrepareForSending( uint32_t InSpecificByteCount ) noexcept
        {
            this->Stream.Buffer.Buffer = GetPacketBuffer();
            this->Stream.Buffer.Length = InSpecificByteCount;
        }

        //! Prepare this buffer for routing to TargetEntityId
        SKL_FORCEINLINE void PrepareForRouting( TEntityIdBase TargetEntityId ) noexcept
        {
            //1. Calculate the total routed packet size
            const TPacketSize TotalRoutingSize{ CalcultateTotalRoutingPacketSize() };

            //2. Update the routing header
            PacketHeader& RHeader{ GetRoutingHeader() };
            SKL_ASSERT( CRoutedPacketOpcode == RHeader.Opcode );
            RHeader.Size = TotalRoutingSize;

            //3. Update the entity id
            SetEntityId( TargetEntityId );

            //4. Prepare for sending routed packet
            this->Stream.Buffer.Buffer = GetRoutingBuffer();
            this->Stream.Buffer.Length = TotalRoutingSize;
        }
        
        //! Prepare this buffer for receiving a routed packet
        SKL_FORCEINLINE void PrepareFoReceivingRoutedPacketBody()
        {
            PacketHeader& RHeader{ GetRoutingHeader() };
            PacketHeader& PHeader{ GetPacketHeader() };

            SKL_ASSERT( CRoutedPacketOpcode == RHeader.Opcode );
            SKL_ASSERT( CRoutedPacketOpcode == PHeader.Opcode );

            //1. Copy header data
            RHeader.Size = PHeader.Size;

            SKL_ASSERT( RHeader.Size >= static_cast<TPacketSize>( AsyncNetBuffer_Usage1::CPacketBodyOffset ) );

            //2. Prepare for receiving the routed packet
            this->Stream.Position       = CPacketHeaderSize;
            this->Stream.Buffer.Buffer  = GetRoutingBodyBuffer();
            this->Stream.Buffer.Length  = RHeader.Size - CPacketHeaderSize;
        }

        //! Does this buffer contain valid routing data
        SKL_FORCEINLINE SKL_NODISCARD bool HasValidRoutingData() const noexcept
        {
            return 0U != GetRoutingHeader().Size;
        }

        // Reset buffer to initial state for reuse
        void Reset() noexcept
        {
            GetRoutingHeader().Size = 0U;
            this->ToOSOpaqueObject()->Reset();
        }

    private:
        SKL_FORCEINLINE SKL_NODISCARD TPacketSize CalcultateTotalRoutingPacketSize() const noexcept
        {
#if !SKL_BUILD_SHIPPING
            const size_t Result      { AsyncNetBuffer_Usage2::GetTotalPaddingSize() + static_cast<size_t>( GetPacketHeader().Size ) };
            const bool   bHasOverflow{ static_cast<size_t>( CPacketMaximumSize ) < Result };

            if( bHasOverflow ) SKL_UNLIKELY
            {
                SKLL_WRN_FMT( "AsyncNetBuffer::CalcultateTotalRoutingPacketSize() Overflow!! Routing packet size: %llu", Result );
            }
            SKL_ASSERT( false == bHasOverflow );

            return static_cast<TPacketSize>( Result );
#else
            return static_cast<TPacketSize>( AsyncNetBuffer_Usage2::GetTotalPaddingSize() ) + GetPacketHeader().Size;
#endif
        }

        //! Set the entity id [EntityId]
        SKL_FORCEINLINE void SetEntityId( TEntityIdBase Id ) noexcept
        {
            return *reinterpret_cast<TEntityIdBase*>( this->Buffer + AsyncNetBuffer_Usage2::CEntityIdOffset ) = Id;
        }
    };
}