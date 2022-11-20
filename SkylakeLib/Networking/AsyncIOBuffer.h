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
            memset( &OsOpaqueType, 0, sizeof( AsyncIOOpaqueType ) );
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
        TDispatch OnDispatch;           //!< The functor to dispatch when the async IO operation is completed
        uint8_t   Buffer[ BufferSize ]; //!< The buffer to carry the data
    };

#define SKL_ASYNCIO_BUFFER_TRANSACTION( BufferPtr )          \
    if( auto Transaction{ NewTask->NewTransaction() }; true )
}

namespace SKL
{
    constexpr size_t CPacketHeaderToBodyPaddingSize = static_cast<size_t>( SKL_ALIGNMENT ) - CPacketHeaderSize;

    template<size_t CompletionTaskSize = 16> 
    struct AsyncNetBuffer: public AsyncIOBuffer<CPacketMaximumSize + CPacketHeaderToBodyPaddingSize, CompletionTaskSize>
    {
        using Base = AsyncIOBuffer<CPacketMaximumSize + CPacketHeaderToBodyPaddingSize, CompletionTaskSize>;

        static constexpr size_t CPacketHeaderOffset = static_cast<size_t>( SKL_ALIGNMENT ) - CPacketHeaderSize;
        static constexpr size_t CPacketBodyOffset   = SKL_ALIGNMENT;

        static constexpr uint32_t CPacketReceiveHeaderState = 0;
        static constexpr uint32_t CPacketReceiveBodyState   = 1;
        static constexpr uint32_t CPacketSendState          = 2;

        // The buffer description
        // ----------------------------------------------------------------
        // | Buffer    | [00 00 00 00] [00 00 00 00] [00 00 00 00 00 ...] |
        // ----------------------------------------------------------------
        // | Purpose   |     State        Header          Packet Body     |
        // ----------------------------------------------------------------
        // | Size      |   4 bytes       4 bytes          65531 bytes     |
        // ----------------------------------------------------------------
        // | Alignment |   8 bytes       4 bytes          8 bytes         |
        // ----------------------------------------------------------------
        // The 4 bytes padding is introduced to force the body to be 8 bytes aligned
        
        SKL_FORCEINLINE SKL_NODISCARD uint32_t GetCurrentState() const noexcept
        {
            return *reinterpret_cast<const uint32_t*>( this->Buffer );
        }
        
        SKL_FORCEINLINE void SetCurrentState( uint32_t State ) noexcept
        {
            return *reinterpret_cast<uint32_t*>( this->Buffer ) = State;
        }
        
        SKL_FORCEINLINE SKL_NODISCARD uint8_t* GetPacketHeaderBuffer() noexcept
        {
            return this->Buffer + CPacketHeaderOffset;
        }
        
        SKL_FORCEINLINE SKL_NODISCARD uint8_t* GetPacketBodyBuffer() noexcept
        {
            return this->Buffer + CPacketBodyOffset;
        }
        
        SKL_FORCEINLINE SKL_NODISCARD PacketHeader& GetPacketHeader() noexcept
        {
            return *reinterpret_cast<PacketHeader*>( GetPacketHeaderBuffer() );
        }
    
        SKL_NODISCARD constexpr static uint32_t GetPacketTotalBufferSize() noexcept
        {
            return CPacketMaximumSize;
        }

        SKL_NODISCARD constexpr static uint32_t GetPacketBodyBufferSize() noexcept
        {
            return CPacketMaximumBodySize;
        }

        template<typename TPacketBody>
        SKL_FORCEINLINE SKL_NODISCARD TPacketBody& CastToPacketType() noexcept
        {
            return *reinterpret_cast<TPacketBody*>( GetPacketBodyBuffer() );
        }
    
        //! get the number of bytes currently received in this buffer
        SKL_FORCEINLINE SKL_NODISCARD uint32_t GetCurrentlyReceivedByteCount() const noexcept { return this->Stream.GetPosition(); }

        //! call this to prepare the buffer for new receive request
        SKL_FORCEINLINE void PrepareForReceiving() noexcept
        {
            this->Stream.Position      = 0;
            this->Stream.Buffer.Buffer = GetPacketHeaderBuffer();
            this->Stream.Buffer.Length = GetPacketTotalBufferSize();
        }
        
        //! confirm n bytes as received into the buffer
        //! \returns true if has received an entire packet and the extra data(if any) was written into the @OutExtraDataStream
        //! \returns false if the received packet is not complete, post a new recv request with the same buffer
        SKL_NODISCARD bool ConfirmReceivedAmmount( uint32_t NoOfBytesTransferred, StreamBase& OutExtraDataStream ) noexcept
        {
            // acknowledge received bytes count
            this->Stream.Position      += NoOfBytesTransferred;
            this->Stream.Buffer.Buffer += NoOfBytesTransferred;
            this->Stream.Buffer.Length -= NoOfBytesTransferred;

            // get the total received count
            const uint32_t CurrentlyReceived{ GetCurrentlyReceivedByteCount() };

            // we must receive at least the header
            if( CurrentlyReceived < CPacketHeaderSize )
            {
                return false;
            }

            // do we have the whole packet received
            const uint32_t ExpectedPacketSize  { GetPacketHeader().Size };
            const bool     bHasReceivedExpected{ CurrentlyReceived >= ExpectedPacketSize };

            // check for extra received data
            if( CurrentlyReceived > ExpectedPacketSize )
            {
                // calculate the extra received data size
                const uint32_t       ExtraDataSize{ CurrentlyReceived - ExpectedPacketSize };
                IStreamObjectWriter& Writer       { IStreamObjectWriter::FromStreamBaseRef( OutExtraDataStream ) };

                SKLL_INF_FMT( "AsyncNetBuffer::ConfirmReceivedAmmount() ExtraBytesReceived: %u bytes", ExtraDataSize );

                // write the extra data into the OutExtraDataStream stream
                const bool bResult{ Writer.Write( this->Stream.Buffer.Buffer, ExtraDataSize ) };
                SKL_ASSERT( bResult );
            }

            return bHasReceivedExpected;
        }
        
        //! place new data into the buffer and check if a packet was received or a new received request is needed
        //! \returns true if has received an entire packet and the extra data(if any) was written into the @InOutDataStream
        //! \returns false if the received packet is not complete, post a new recv request with the same buffer
        SKL_FORCEINLINE bool PlaceDataIntoBufferBeforeReceiving( StreamBase& InOutDataStream ) noexcept
        {
            SKL_ASSERT( 0 < InOutDataStream.GetPosition() );

            IStreamObjectReader& Reader{ IStreamObjectReader::FromStreamBaseRef( InOutDataStream ) };
            const uint32_t InDataSize  { InOutDataStream.GetPosition() };

            // reset the in buffer position
            InOutDataStream.Position = 0;

            // copy in the data
            const bool bResult{ Reader.Read( GetPacketHeaderBuffer(), InDataSize ) };
            SKL_ASSERT( bResult );

            // reset the stream interface to the full buffer
            PrepareForReceiving();

            // reset the out buffer position
            InOutDataStream.Position = 0;

            return ConfirmReceivedAmmount( InDataSize, InOutDataStream );
        }

        //! after a packet was received completely, call this method to prepare this buffer to be used in the packet's processing
        SKL_FORCEINLINE void PrepareForPacketProcessing() noexcept
        {
            this->Stream.Position      = 0;
            this->Stream.Buffer.Buffer = GetPacketHeaderBuffer();
            this->Stream.Buffer.Length = GetPacketHeader().Size;
        }
    };
}