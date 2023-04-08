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

        IAsyncIOTask( IBuffer BufferInterface ) noexcept: Stream { 0, BufferInterface.Length, BufferInterface.Buffer, true }
        {
            OsOpaqueType.Reset();
        }
        ~IAsyncIOTask() noexcept { Clear(); }

        //! Dispatch this task
        SKL_FORCEINLINE void Dispatch( uint32_t NumberOfBytesTransferred ) noexcept
        {
            SKL_ASSERT( false == CastSelfToProto().IsNull() );
            CastSelfToProto().Dispatch( *this, NumberOfBytesTransferred );
        }
        
        //! Is this task valid
        SKL_FORCEINLINE SKL_NODISCARD bool IsNull() const noexcept { return CastSelfToProto().IsNull(); }

        //! Clear the underlying functor
        SKL_FORCEINLINE void Clear() noexcept { CastSelfToProto().Destroy(); }

        //! Get the interface to the internal buffer reference
        SKL_FORCEINLINE SKL_NODISCARD const IBuffer& GetInterface() const noexcept { return Stream.Buffer; }

        //! Get the interface to the internal buffer reference
        SKL_FORCEINLINE SKL_NODISCARD IBuffer& GetInterface() noexcept { return Stream.Buffer; }

        //! Cast self to Os opaque type
        SKL_FORCEINLINE SKL_NODISCARD AsyncIOOpaqueType* ToOSOpaqueObject() noexcept { return reinterpret_cast<AsyncIOOpaqueType*>( this ); }

        //! Construct a new stream interface for this async IO buffer
        SKL_FORCEINLINE SKL_NODISCARD BinaryStreamInterface GetStreamInterface() noexcept {  return BinaryStreamInterface{ &Stream }; }
        
        //! Get the stream object used to modify the buffer
        SKL_FORCEINLINE SKL_NODISCARD StreamBase& GetStreamBase() noexcept { return Stream; }

        //! Get the stream object used to modify the buffer
        SKL_FORCEINLINE SKL_NODISCARD StreamBase const& GetStreamBase() const noexcept { return Stream; }

        //! Construct a new binary stream interface for this async IO buffer
        SKL_FORCEINLINE SKL_NODISCARD IBinaryStream<uint8_t, true>* GetStream() noexcept { return IBinaryStream<uint8_t, true>::FromStreamBase( Stream ); }
        
        //! Construct a new object stream interface for this async IO buffer
        template<typename T>
        SKL_FORCEINLINE SKL_NODISCARD IObjectStream<T>* BuildObjectStream() noexcept { return IObjectStream<T>::FromStreamBase( Stream ); }

        //! Construct stream transaction interface into this buffer at the current position
        SKL_FORCEINLINE SKL_NODISCARD BinaryStreamTransaction NewTransaction() noexcept { return BinaryStreamTransaction{ Stream }; }

        //! Has reached buffer end (is end of buffer)
        SKL_FORCEINLINE SKL_NODISCARD bool IsEOS() const noexcept { return Stream.Position == Stream.Buffer.Length; }

        //! Get current stream position
        SKL_FORCEINLINE SKL_NODISCARD uint32_t GetPosition() const noexcept { return Stream.Position; }
        
        //! Copy everything from buffer base to the current position into the TargetStream
        SKL_FORCEINLINE SKL_NODISCARD bool CopyTo( IBinaryStream<uint8_t, true>* TargetStream ) noexcept { return TargetStream->Write( Stream.GetBuffer(), Stream.Position ); }

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
    struct AsyncIOBuffer : public IAsyncIOTask
    {
        static constexpr uint32_t BufferSize       = TBufferSize;
        static constexpr size_t CompletionTaskSize = TCompletionTaskSize;

        static_assert( CompletionTaskSize % 8 == 0 );

        using TDispatch = ASD::UniqueFunctorWrapper<CompletionTaskSize, typename IAsyncIOTask::TDispatchFunctionPtr>;
        
        AsyncIOBuffer() noexcept: IAsyncIOTask( IBuffer{ BufferSize, Buffer } ), OnDispatch{} 
        {
            SKL_ASSERT( 0 == ( reinterpret_cast<uint64_t>( Buffer ) % SKL_ALIGNMENT ) );
        }
        AsyncIOBuffer( IBuffer InInterface ) noexcept: IAsyncIOTask( InInterface ), OnDispatch{} 
        {
            SKL_ASSERT( 0 == ( reinterpret_cast<uint64_t>( Buffer ) % SKL_ALIGNMENT ) );
        }
        ~AsyncIOBuffer() noexcept = default;

        //! Set the functor to be executed when the async IO operation is completed [void( ASD_CDECL* )( IAsyncIOTask&, uint32_t ) noexcept]
        template<typename TFunctor>
        SKL_FORCEINLINE void operator+=( TFunctor&& InFunctor ) noexcept { OnDispatch += std::forward<TFunctor>( InFunctor ); }

        //! Set the functor to be executed when the async IO operation is completed [void( ASD_CDECL* )( IAsyncIOTask&, uint32_t ) noexcept]
        template<typename TFunctor>
        SKL_FORCEINLINE void SetCompletionHandler( TFunctor&& InFunctor ) noexcept { OnDispatch += std::forward<TFunctor>( InFunctor ); }
        
        //! Get the buffer ptr
        SKL_FORCEINLINE SKL_NODISCARD uint8_t* GetBuffer() noexcept { return Buffer; }

        //! Get the buffer ptr
        SKL_FORCEINLINE SKL_NODISCARD const uint8_t* GetBuffer() const noexcept { return Buffer; }
    
    protected:
        TDispatch OnDispatch;         //!< The functor to dispatch when the async IO operation is completed
        uint8_t   Buffer[BufferSize]; //!< The buffer to carry the data
    };

#define SKL_ASYNCIO_BUFFER_TRANSACTION( BufferPtr )          \
    if( SKL::BinaryStreamTransaction Transaction{ BufferPtr->NewTransaction() }; true )
}

namespace SKL
{
    template<size_t CompletionTaskSize>
    struct IAsyncNetBufferBase;
    
    template<size_t CompletionTaskSize>
    struct IAsyncNetBuffer;

    template<size_t CompletionTaskSize>
    struct IRoutedAsyncNetBuffer;

    template<typename TEntityIdType, size_t CompletionTaskSize>
    struct IBroadcastAsyncNetBuffer;

    template<size_t CompletionTaskSize = 16>
    struct AsyncNetBuffer: public AsyncIOBuffer<CPacketMaximumSize, CompletionTaskSize>
    {
        using MyType = AsyncNetBuffer<CompletionTaskSize>;
        using Base   = AsyncIOBuffer<CPacketMaximumSize, CompletionTaskSize>;

        static constexpr size_t CPacketHeaderOffset = 12U;
        static constexpr size_t CPacketBodyOffset   = CPacketHeaderOffset + CPacketHeaderSize;
        
        static constexpr uint32_t CPacketReceiveHeaderState = 0;
        static constexpr uint32_t CPacketReceiveBodyState   = 1;
        static constexpr uint32_t CPacketSendState          = 2;

        AsyncNetBuffer() noexcept;
        ~AsyncNetBuffer() noexcept = default;

        // Buffer description
        // --------------------------------------------------------------------------------------------
        // | Buffer    | [00 00 00 00] [00 00 00 00] [00 00 00 00] [00 00 00 00] [00 00 00 00 00 ...] |
        // --------------------------------------------------------------------------------------------
        // | Usage 1   |                  Padding                 |    Header   |     Packet Body     |
        // --------------------------------------------------------------------------------------------
        // | Usage 2   |   R Header   |         EntityId          |    Header   |     Packet Body     |
        // --------------------------------------------------------------------------------------------
        // | Usage 3   |   B Header   |  T   | CNT  | OFF  |   U  |    Header   |     Packet Body     |
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
        // Usage 3: Broadcast buffer
        //        [B Header]    - Broadcast header
        //        [T]           - Broadcast type
        //        [CNT]         - Target entities count
        //        [OFF]         - Target entities array offset
        //        [U]           - Unused
        //        [Header]      - Packet header
        //        [Packet Body] - Packet body (must be 8 bytes aligned)
        
        //! Get the size of the entire buffer of any instance of this type
        SKL_FORCEINLINE SKL_NODISCARD consteval static uint32_t GetTotalBufferSize() noexcept { return CPacketMaximumSize; }

        //! Get the size of the buffer portion that we can receive the packet into
        SKL_FORCEINLINE SKL_NODISCARD consteval static uint32_t GetPacketBufferSize() noexcept { return CPacketMaximumUsableBodySize + CPacketHeaderSize; }

        //! Get the size of the buffer portion that we can receive the packet body into
        SKL_FORCEINLINE SKL_NODISCARD consteval static uint32_t GetPacketBodyBufferSize() noexcept { return CPacketMaximumUsableBodySize; }

        //! Get the packet header
        SKL_FORCEINLINE SKL_NODISCARD PacketHeader& GetPacketHeader() noexcept { return *reinterpret_cast<PacketHeader*>( this->Buffer + CPacketHeaderOffset ); }
    
        //! Get the packet header
        SKL_FORCEINLINE SKL_NODISCARD const PacketHeader& GetPacketHeader() const noexcept { return *reinterpret_cast<const PacketHeader*>( this->Buffer + CPacketHeaderOffset ); }
    
        //! Deduce from the packet header if there is a packet body present
        SKL_FORCEINLINE SKL_NODISCARD bool DoesThePacketIndicateTheBody() const noexcept { return GetPacketHeader().Size > CPacketHeaderSize; }

        //! Cast the packet body buffer to the give type
        template<typename TPacketBody>
        SKL_FORCEINLINE SKL_NODISCARD TPacketBody& CastToPacketType() noexcept { return *reinterpret_cast<TPacketBody*>( this->Buffer + CPacketBodyOffset ); }
    
        //! Cast the packet body buffer to the give type
        template<typename TPacketBody>
        SKL_FORCEINLINE SKL_NODISCARD TPacketBody const& CastToPacketType() const noexcept { return *reinterpret_cast<const TPacketBody*>( this->Buffer + CPacketBodyOffset ); }
    
        //! Reset buffer to initial state for reuse
        void Reset() noexcept;

    private:
        friend IRoutedAsyncNetBuffer<CompletionTaskSize>;
        friend IAsyncNetBufferBase<CompletionTaskSize>;
        friend IAsyncNetBuffer<CompletionTaskSize>;

        template<typename, size_t> 
        friend struct IBroadcastAsyncNetBuffer;
    };

    template<size_t CompletionTaskSize>
    struct IAsyncNetBufferBase
    {
        using Super = AsyncNetBuffer<CompletionTaskSize>;

        static constexpr TPacketSize CBufferHeaderOffset              = 0U; 
        static constexpr TPacketSize CPacketBodySize                  = CPacketMaximumUsableBodySize; 
        static constexpr TPacketSize CSizeOfBufferPaddingBeforePacket = 12U; 
        static constexpr TPacketSize CPacketHeaderOffset              = CSizeOfBufferPaddingBeforePacket;
        static constexpr TPacketSize CPacketBodyOffset                = CPacketHeaderOffset + CPacketHeaderSize;
        
        //! Get the size of the entire buffer of any instance of this type
        SKL_FORCEINLINE SKL_NODISCARD consteval static uint32_t GetTotalBufferSize() noexcept { return static_cast<uint32_t>( CSizeOfBufferPaddingBeforePacket ) + CPacketBodySize + CPacketHeaderSize; } 

        //! Get the size of the buffer portion that we can receive the packet into
        SKL_FORCEINLINE SKL_NODISCARD consteval static uint32_t GetPacketBufferSize() noexcept { return CPacketBodySize + CPacketHeaderSize; }

        //! Get the size of the buffer portion that we can receive the packet body into
        SKL_FORCEINLINE SKL_NODISCARD consteval static uint32_t GetPacketBodyBufferSize() noexcept { return CPacketBodySize; }
        
        // This type cannot be used to create an object, its meant to be used for pointer/ref type
        IAsyncNetBufferBase() noexcept = delete;
        IAsyncNetBufferBase( const IAsyncNetBufferBase& ) = delete;
        IAsyncNetBufferBase& operator=( const IAsyncNetBufferBase& ) = delete;
        IAsyncNetBufferBase( IAsyncNetBufferBase&& ) = delete;
        IAsyncNetBufferBase& operator=( IAsyncNetBufferBase&& ) = delete;
        ~IAsyncNetBufferBase() = delete;
        
        //! Get the buffer ptr
        SKL_FORCEINLINE SKL_NODISCARD uint8_t* GetBuffer() noexcept { return GetSuper().Buffer; }
    
        //! Get the buffer ptr
        SKL_FORCEINLINE SKL_NODISCARD const uint8_t* GetBuffer() const noexcept { return GetSuper().Buffer; }
    
        //! Get the buffer header
        SKL_FORCEINLINE SKL_NODISCARD PacketHeader& GetBufferHeader() noexcept { return *reinterpret_cast<PacketHeader*>( GetBuffer() ); }
    
        //! Get the buffer header
        SKL_FORCEINLINE SKL_NODISCARD PacketHeader const& GetBufferHeader() const noexcept { return *reinterpret_cast<const PacketHeader*>( GetBuffer() ); }
    
        //! Get the packet header
        SKL_FORCEINLINE SKL_NODISCARD PacketHeader& GetPacketHeader() noexcept { return *reinterpret_cast<PacketHeader*>( GetBuffer() + CPacketHeaderOffset ); }
    
        //! Get the packet header
        SKL_FORCEINLINE SKL_NODISCARD PacketHeader const& GetPacketHeader() const noexcept { return *reinterpret_cast<const PacketHeader*>( GetBuffer() + CPacketHeaderOffset ); }
    
        //! Get the buffer pointer starting at the packet header 
        SKL_FORCEINLINE SKL_NODISCARD uint8_t* GetPacketBuffer() noexcept { return GetBuffer() + CPacketHeaderOffset; }
        
        //! Get the buffer pointer starting at the packet body 
        SKL_FORCEINLINE SKL_NODISCARD uint8_t* GetPacketBodyBuffer() noexcept { return GetBuffer() + CPacketBodyOffset; }
        
        //! Get the underlying stream object
        SKL_FORCEINLINE SKL_NODISCARD StreamBase& GetStream() noexcept { return GetSuper().Stream; }

        //! Get the underlying stream object
        SKL_FORCEINLINE SKL_NODISCARD StreamBase const& GetStream() const noexcept { return GetSuper().Stream; }

        //! Cast the packet body buffer to the give type
        template<typename TPacketBody>
        SKL_FORCEINLINE SKL_NODISCARD TPacketBody& CastToPacketType() noexcept { return *reinterpret_cast<TPacketBody*>( GetPacketBodyBuffer() ); }
    
        //! Get the number of bytes currently received in this buffer
        SKL_FORCEINLINE SKL_NODISCARD uint32_t GetCurrentlyReceivedByteCount() const noexcept { return GetStream().GetPosition(); }

        //! Get the AsyncIOBuffer that this interface is operating on
        SKL_FORCEINLINE SKL_NODISCARD Super& GetSuper() noexcept { return *reinterpret_cast<Super*>( this ); }

        //! Get the AsyncIOBuffer that this interface is operating on
        SKL_FORCEINLINE SKL_NODISCARD Super const& GetSuper() const noexcept { return *reinterpret_cast<const Super*>( this ); }
        
        //! Get the stream writer
        SKL_FORCEINLINE SKL_NODISCARD IByteStreamObjectWriter& GetWriter() noexcept { return IByteStreamObjectWriter::FromStreamBaseRef( GetStream() ); }
        
        //! Get the stream reader
        SKL_FORCEINLINE SKL_NODISCARD IByteStreamObjectReader& GetReader() noexcept { return IByteStreamObjectReader::FromStreamBaseRef( GetStream() ); }

        static_assert( CPacketMaximumSize == GetTotalBufferSize() );
    };

    template<size_t CompletionTaskSize>
    struct IAsyncNetBuffer: public IAsyncNetBufferBase<CompletionTaskSize>
    {
        using Base  = IAsyncNetBufferBase<CompletionTaskSize>;
        using Super = AsyncNetBuffer<CompletionTaskSize>;

        // This type cannot be used to create an object, its meant to be used for pointer/ref type
        IAsyncNetBuffer() noexcept = delete;
        IAsyncNetBuffer( const IAsyncNetBuffer& ) = delete;
        IAsyncNetBuffer& operator=( const IAsyncNetBuffer& ) = delete;
        IAsyncNetBuffer( IAsyncNetBuffer&& ) = delete;
        IAsyncNetBuffer& operator=( IAsyncNetBuffer&& ) = delete;
        ~IAsyncNetBuffer() = delete;
        
        SKL_FORCEINLINE SKL_NODISCARD bool IsReceivingExtendedPacket() const noexcept 
        {  
            PacketHeader BufferHeader{ this->GetBufferHeader() };
            return BufferHeader.Size != 0U && BufferHeader.Opcode != CInvalidOpcode;
        }

        //! Prepare the buffer for receiving a max of GetPacketBufferSize() bytes
        SKL_FORCEINLINE void PrepareForReceiving() noexcept
        {
            StreamBase& Stream{ this->GetStream() };
            Stream.Position      = 0U;
            Stream.Buffer.Buffer = this->GetPacketBuffer();
            Stream.Buffer.Length = this->GetPacketBufferSize();
        }
        
        //! Prepare the buffer for receiving a max of CPacketHeaderSize bytes and set State to CPacketReceiveHeaderState
        SKL_FORCEINLINE void PrepareForReceivingHeader() noexcept
        {
            StreamBase& Stream{ this->GetStream() };
            Stream.Position      = 0U;
            Stream.Buffer.Buffer = this->GetPacketBuffer();
            Stream.Buffer.Length = CPacketHeaderSize;
        }
        
        //! Prepare the buffer for receiving the packet body 
        //! \remarks to be used after a successful header receive has been performed
        SKL_FORCEINLINE void PrepareForReceivingBody() noexcept
        {
            StreamBase& Stream{ this->GetStream() };
            Stream.Position       = CPacketHeaderSize;
            Stream.Buffer.Buffer += CPacketHeaderSize;
            Stream.Buffer.Length  = this->GetPacketHeader().Size - CPacketHeaderSize;
        }
        
        //! Confirm exact n bytes as received into the packet buffer
        //! \returns <bool hasReceivedWholePacket, bool bProcessedSuccessfully>
        /*SKL_FORCEINLINE*/ SKL_NODISCARD std::pair<bool, bool> ConfirmReceivedExactAmmount( uint32_t NoOfBytesTransferred ) noexcept
        {
            SKL_ASSERT( NoOfBytesTransferred <= CPacketMaximumSize );

            // acknowledge received bytes count
            StreamBase& Stream{ this->GetStream() };
            Stream.Position      += NoOfBytesTransferred;
            Stream.Buffer.Buffer += NoOfBytesTransferred;
            Stream.Buffer.Length -= NoOfBytesTransferred;

            // get the total received count
            const uint32_t CurrentlyReceived{ Stream.Position };

            // we must receive at least the header
            if( CurrentlyReceived < CPacketHeaderSize || CurrentlyReceived >= static_cast<uint32_t>( CPacketMaximumUsableUserPacketSize ) ) SKL_UNLIKELY
            {
                return { false, true };
            }

            PacketHeader Header;
            if( IsReceivingExtendedPacket() )
            {
                Header = this->GetBufferHeader();
            }
            else
            {
                Header = this->GetPacketHeader();
            }
            
            // do we have the whole packet received
            const uint32_t ExpectedPacketSize  { Header.Size };
            const bool     bHasReceivedExpected{ CurrentlyReceived == ExpectedPacketSize };

            return { bHasReceivedExpected, CurrentlyReceived <= ExpectedPacketSize };
        }
        
        //! Confirm at most n bytes as received into the packet buffer
        //! \returns <bool hasReceivedWholePacket, bool bProcessedSuccessfully>
        SKL_FORCEINLINE SKL_NODISCARD std::pair<bool, bool> ConfirmReceivedAmmount( uint32_t NoOfBytesTransferred, StreamBase& OutExtraData ) noexcept
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
            if( IsReceivingExtendedPacket() )
            {
                Header = this->GetBufferHeader();
            }
            else
            {
                Header =  this->GetPacketHeader();
            }
            
            // do we have the whole packet received
            const uint32_t ExpectedPacketSize  { Header.Size };
            const bool     bHasReceivedExpected{ CurrentlyReceived >= ExpectedPacketSize };

            // if we have extra data received copy it over in the extra data stream
            if( CurrentlyReceived > ExpectedPacketSize )
            {
                const uint32_t           ExtraDataSize{ CurrentlyReceived - ExpectedPacketSize };
                IByteStreamObjectWriter& Writer       { IByteStreamObjectWriter::FromStreamBaseRef( OutExtraData ) };

                if( false == Writer.Write( this->GetPacketBuffer() + ExpectedPacketSize, ExtraDataSize, false ) ) SKL_UNLIKELY
                {
                    GLOG_DEBUG( "ConfirmReceivedAmmount( ... ) Failed to copy extra data %u", ExtraDataSize );
                    return { false, false };
                }

                PrepareForPacketProcessing();
            }

            return { bHasReceivedExpected, true };
        }
        
        //! Prepare this buffer for sending the packet 
        SKL_FORCEINLINE void PrepareForSending() noexcept
        {
            StreamBase& Stream{ this->GetStream() };
            Stream.Buffer.Buffer = this->GetPacketBuffer();
            Stream.Buffer.Length = Stream.Position;
        }
        
        //! Prepare this buffer for sending the packet with specific bytes count
        SKL_FORCEINLINE void PrepareForSending( uint32_t InSpecificByteCount ) noexcept
        {
            StreamBase& Stream{ this->GetStream() };
            Stream.Buffer.Buffer = this->GetPacketBuffer();
            Stream.Buffer.Length = InSpecificByteCount;
        }
        
        //! After a packet was received completely, call this method to prepare the buffer to be used in the packet's processing
        SKL_FORCEINLINE void PrepareForPacketProcessing() noexcept
        {
            StreamBase& Stream{ this->GetStream() };
            Stream.Position      = 0U;
            Stream.Buffer.Buffer = this->GetBuffer() + Base::CPacketHeaderOffset;
            Stream.Buffer.Length = this->GetPacketHeader().Size;
        }

        //! Setup the stream object for packet writing
        SKL_FORCEINLINE void PrepareForPacketWriting() noexcept
        {
            StreamBase& Stream{ this->GetStream() };

            Stream.Buffer.Buffer = this->GetPacketBuffer();
            Stream.Buffer.Length = Base::GetPacketBufferSize();
            Stream.Position      = 0U;
        }
        
        //! Setup the stream object for packet writing
        SKL_FORCEINLINE void EndPacketWritingAndPrepareForSending() noexcept
        {
            StreamBase& Stream{ this->GetStream() };

            this->GetPacketHeader().Size = static_cast<TPacketSize>( Stream.Position );
            Stream.Buffer.Buffer         = this->GetPacketBuffer();
            Stream.Buffer.Length         = Stream.Position;
        }
        
        //! Write into the underlying buffer through the current stream 
        //! \param InSourceBuffer Source buffer ptr
        //! \param WriteAmount No of bytes to copy
        //! \return ROperationOverflows if there is no space to copy WriteAmount of bytes in the underlying stream
        //! \return RFail if the memcpy operation failed
        //! \return RSuccess if the memcpy operation succeeded and the underlying stream position was advanced by WriteAmount
        SKL_NODISCARD RStatus Write( const uint8_t* InSourceBuffer, TPacketSize WriteAmount ) noexcept
        {
            StreamBase& Stream{ this->GetStream() };

            if( WriteAmount > Stream.GetRemainingSize() ) SKL_UNLIKELY
            {
                return ROperationOverflows;
            }

            if( errno_t( 0 ) != ::memcpy_s( Stream.GetFront(), Stream.GetRemainingSize(), InSourceBuffer, WriteAmount ) ) SKL_UNLIKELY
            {
                return RFail;
            }

            Stream.Position += WriteAmount;

            return RSuccess;
        }

        //! Build a new ref into the InBuffer interfaces as IAsyncNetBuffer
        SKL_FORCEINLINE SKL_NODISCARD static IAsyncNetBuffer& FromBuffer( Super& InBuffer ) noexcept { return reinterpret_cast<IAsyncNetBuffer&>( InBuffer ); }
        SKL_FORCEINLINE SKL_NODISCARD static const IAsyncNetBuffer& FromBuffer( const Super& InBuffer ) noexcept { return reinterpret_cast<const IAsyncNetBuffer&>( InBuffer ); }
        
        //! Build a new ptr into the InBuffer interfaces as IAsyncNetBuffer
        SKL_FORCEINLINE SKL_NODISCARD static IAsyncNetBuffer* FromBufferPtr( Super* InBuffer ) noexcept { return reinterpret_cast<IAsyncNetBuffer*>( InBuffer ); }
        SKL_FORCEINLINE SKL_NODISCARD static const IAsyncNetBuffer* FromBufferPtr( const Super* InBuffer ) noexcept { return reinterpret_cast<const IAsyncNetBuffer*>( InBuffer ); }

        //! Build a new ptr into the InBuffer interfaces as IAsyncNetBuffer and prepare the target stream for packet writing
        SKL_FORCEINLINE SKL_NODISCARD static IAsyncNetBuffer* FromBufferPtrForPacketWriting( Super* InBuffer ) noexcept 
        { 
            auto* Result = reinterpret_cast<IAsyncNetBuffer*>( InBuffer ); 
            Result->PrepareForPacketWriting();
            return Result;
        }
    };

    template<size_t CompletionTaskSize>
    struct IRoutedAsyncNetBuffer: public IAsyncNetBufferBase<CompletionTaskSize>
    {
        using Base  = IAsyncNetBufferBase<CompletionTaskSize>;
        using Super = AsyncNetBuffer<CompletionTaskSize>;

        static constexpr size_t CRHeaderOffset      = 0U; 
        static constexpr size_t CEntityIdOffset     = CPacketHeaderSize; 
        static constexpr size_t CEntityIdSize       = 8U;                          

        // This type cannot be used to create an object, its meant to be used for pointer/ref type
        IRoutedAsyncNetBuffer() noexcept = delete;
        IRoutedAsyncNetBuffer( const IRoutedAsyncNetBuffer& ) = delete;
        IRoutedAsyncNetBuffer& operator=( const IRoutedAsyncNetBuffer& ) = delete;
        IRoutedAsyncNetBuffer( IRoutedAsyncNetBuffer&& ) = delete;
        IRoutedAsyncNetBuffer& operator=( IRoutedAsyncNetBuffer&& ) = delete;
        ~IRoutedAsyncNetBuffer() = delete;
        
        //! Get the buffer pointer starting at the packet header 
        SKL_FORCEINLINE SKL_NODISCARD uint8_t* GetRoutingBuffer() noexcept { return this->GetBuffer() + CRHeaderOffset; }
        
        //! Get the buffer pointer starting at the packet routing body 
        SKL_FORCEINLINE SKL_NODISCARD uint8_t* GetRoutingBodyBuffer() noexcept { return this->GetBuffer() + CPacketHeaderSize; }

        //! Get the routing header
        SKL_FORCEINLINE SKL_NODISCARD PacketHeader& GetRoutingHeader() noexcept { return *reinterpret_cast<PacketHeader*>( this->GetBuffer() + CRHeaderOffset ); }
    
        //! Get the routing header
        SKL_FORCEINLINE SKL_NODISCARD PacketHeader const& GetRoutingHeader() const noexcept { return *reinterpret_cast<const PacketHeader*>( this->GetBuffer() + CRHeaderOffset ); }
    
        //! Get the entity id [EntityId]
        SKL_FORCEINLINE SKL_NODISCARD TEntityIdBase GetEntityId() const noexcept { return *reinterpret_cast<const TEntityIdBase*>( this->GetBuffer() + CEntityIdOffset ); }
        
        //! Does this buffer contain valid routing data
        SKL_FORCEINLINE SKL_NODISCARD bool HasValidRoutingData() const noexcept
        {
            const PacketHeader PHeader{ GetRoutingHeader() };
            return 0U != PHeader.Size && PHeader.Opcode == CRoutedPacketOpcode;
        }
        
        //! Prepare this buffer for routing to TargetEntityId
        void PrepareForRouting( TEntityIdBase TargetEntityId ) noexcept
        {
            //1. Calculate the total routed packet size
            const TPacketSize TotalRoutingSize{ CalcultateTotalRoutingPacketSize() };

            //2. Update the routing header
            PacketHeader& RHeader{ GetRoutingHeader() };
            RHeader.Opcode = CRoutedPacketOpcode;
            RHeader.Size   = TotalRoutingSize;

            //3. Update the entity id
            SetEntityId( TargetEntityId );

            //4. Prepare for sending routed packet
            StreamBase& Stream{ this->GetStream() };
            Stream.Buffer.Buffer = GetRoutingBuffer();
            Stream.Buffer.Length = TotalRoutingSize;
        }
        
        //! Prepare this buffer for receiving a routed packet
        void PrepareFoReceivingRoutedPacketBody()
        {
            PacketHeader& RHeader{ GetRoutingHeader() };
            const PacketHeader& PHeader{ this->GetPacketHeader() };

            //1. Copy header data
            RHeader = PHeader;
            
            SKL_ASSERT( CRoutedPacketOpcode == RHeader.Opcode );
            SKL_ASSERT( CRoutedPacketOpcode == PHeader.Opcode );
            SKL_ASSERT( RHeader.Size >= static_cast<TPacketSize>( Base::CPacketBodyOffset ) );

            //2. Prepare for receiving the routed packet
            StreamBase& Stream{ this->GetStream() };
            Stream.Position      = CPacketHeaderSize; // Here the position is used to indicated totally received data
            Stream.Buffer.Buffer = this->GetRoutingBodyBuffer();
            Stream.Buffer.Length = RHeader.Size - CPacketHeaderSize;
        }
        
        SKL_FORCEINLINE SKL_NODISCARD TPacketSize CalcultateTotalRoutingPacketSize() const noexcept
        {
#if !SKL_BUILD_SHIPPING
            const size_t Result      { static_cast<size_t>( Base::CSizeOfBufferPaddingBeforePacket ) + static_cast<size_t>( this->GetPacketHeader().Size ) };
            const bool   bHasOverflow{ static_cast<size_t>( CPacketMaximumSize ) < Result };

            if( bHasOverflow ) SKL_UNLIKELY
            {
                GLOG_DEBUG( "AsyncNetBuffer::CalcultateTotalRoutingPacketSize() Overflow!! Routing packet size: %llu", Result );
            }
            SKL_ASSERT( false == bHasOverflow );

            return static_cast<TPacketSize>( Result );
#else
            return static_cast<TPacketSize>( Base::CSizeOfBufferPaddingBeforePacket ) + this->GetPacketHeader().Size;
#endif
        }
        
        SKL_FORCEINLINE void SetEntityId( TEntityIdBase Id ) noexcept { *reinterpret_cast<TEntityIdBase*>( this->GetBuffer() + CEntityIdOffset ) = Id; }
        
        //! Build a new ref into the InBuffer interfaces as IAsyncNetBuffer
        SKL_FORCEINLINE SKL_NODISCARD static IRoutedAsyncNetBuffer& FromBuffer( Super& InBuffer ) noexcept { return reinterpret_cast<IRoutedAsyncNetBuffer&>( InBuffer ); }
        SKL_FORCEINLINE SKL_NODISCARD static const IRoutedAsyncNetBuffer& FromBuffer( const Super& InBuffer ) noexcept { return reinterpret_cast<const IRoutedAsyncNetBuffer&>( InBuffer ); }
        
        //! Build a new ptr into the InBuffer interfaces as IAsyncNetBuffer
        SKL_FORCEINLINE SKL_NODISCARD static IRoutedAsyncNetBuffer* FromBufferPtr( Super* InBuffer ) noexcept { return reinterpret_cast<IRoutedAsyncNetBuffer*>( InBuffer ); }
        SKL_FORCEINLINE SKL_NODISCARD static const IRoutedAsyncNetBuffer* FromBufferPtr( const Super* InBuffer ) noexcept { return reinterpret_cast<const IRoutedAsyncNetBuffer*>( InBuffer ); }
    };
    
    template<typename TEntityIdType, size_t CompletionTaskSize>
    struct IBroadcastAsyncNetBuffer: public IAsyncNetBufferBase<CompletionTaskSize>
    {
        using Base  = IAsyncNetBufferBase<CompletionTaskSize>;
        using Super = AsyncNetBuffer<CompletionTaskSize>;

        static constexpr uint32_t CEntityIdSize = sizeof( TEntityIdType );
        static_assert( CEntityIdSize <= 8U );
        static_assert( sizeof( TBroadcastType ) == sizeof( uint8_t ) );

        static constexpr size_t   CBHeaderSize     = CPacketHeaderSize; //!< [B Header]
        static constexpr size_t   CTypeSize        = 1U;                //!< [Type]
        static constexpr size_t   CPayloadSize     = 3U;                //!< [Payload]
        static constexpr size_t   CCountSize       = 2U;                //!< [Count]
        static constexpr size_t   COffsetSize      = 2U;                //!< [Offset]
        static constexpr uint32_t CPayloadMask     = 0xffffff00;
        static constexpr uint32_t CPayloadMaxValue = CPayloadMask >> 8;
        static constexpr uint32_t CTypeMask        = 0x000000ff;

        static constexpr size_t CBHeaderOffset = 0U; 
        static constexpr size_t CTypeOffset    = CPacketHeaderSize; 
        static constexpr size_t CPayloadOffset = CPacketHeaderSize; 
        static constexpr size_t CCountOffset   = CPacketHeaderSize + 4U; 
        static constexpr size_t COffsetOffset  = CCountOffset + CCountSize; 
        
        static constexpr size_t CMaxTargetEntitiesPossibleCount = Base::CPacketBodySize / CEntityIdSize;

        // This type cannot be used to create an object, its meant to be used for pointer/ref type
        IBroadcastAsyncNetBuffer() noexcept = delete;
        IBroadcastAsyncNetBuffer( const IBroadcastAsyncNetBuffer& ) = delete;
        IBroadcastAsyncNetBuffer& operator=( const IBroadcastAsyncNetBuffer& ) = delete;
        IBroadcastAsyncNetBuffer( IBroadcastAsyncNetBuffer&& ) = delete;
        IBroadcastAsyncNetBuffer& operator=( IBroadcastAsyncNetBuffer&& ) = delete;
        ~IBroadcastAsyncNetBuffer() = delete;
        
        //! Get the buffer pointer starting at the packet broadcast body 
        SKL_FORCEINLINE SKL_NODISCARD uint8_t* GetBroadcastBuffer() noexcept { return this->GetBuffer() + CBHeaderOffset; }
        
        //! Get the buffer pointer starting at the packet broadcast body 
        SKL_FORCEINLINE SKL_NODISCARD const uint8_t* GetBroadcastBuffer() const noexcept { return this->GetBuffer() + CBHeaderOffset; }
        
        //! Get the buffer pointer starting at the packet broadcast body 
        SKL_FORCEINLINE SKL_NODISCARD uint8_t* GetBroadcastBodyBuffer() noexcept { return this->GetBuffer() + CPacketHeaderSize; }
        
        //! Get the buffer pointer starting at the packet broadcast body 
        SKL_FORCEINLINE SKL_NODISCARD const uint8_t* GetBroadcastBodyBuffer() const noexcept { return this->GetBuffer() + CPacketHeaderSize; }
        
        //! Get the routing header
        SKL_FORCEINLINE SKL_NODISCARD PacketHeader& GetBroadcastHeader() noexcept { return *reinterpret_cast<PacketHeader*>( GetBroadcastBuffer() + CBHeaderOffset ); }
        
        //! Get the routing header
        SKL_FORCEINLINE SKL_NODISCARD PacketHeader const& GetBroadcastHeader() const noexcept { return *reinterpret_cast<const PacketHeader*>( GetBroadcastBuffer() + CBHeaderOffset ); }
    
        //! Does this buffer contain valid broadcast data
        SKL_FORCEINLINE SKL_NODISCARD bool HasValidBroadcastData() const noexcept
        {
            const PacketHeader BHeader{ GetBroadcastHeader() };
            return 0U != BHeader.Size && BHeader.Opcode == CBroadcastPacketOpcode;
        }
        
        //! Are the stream size and position valid to write at least min target entities
        SKL_FORCEINLINE SKL_NODISCARD bool CanStartWriteBroadcastTargets() const noexcept
        {
            const TPacketSize ExpectedPosition{ CalcultateTotalBroadcastPacketSize() };
            return this->GetStream().GetPosition() == ExpectedPosition;
        }

        //! Get the broadcast type
        SKL_FORCEINLINE SKL_NODISCARD TBroadcastType GetBroadcastType() const noexcept { return *reinterpret_cast<const TBroadcastType*>( this->GetBuffer() + CTypeOffset ); }
        
        //! Set the broadcast type as enum
        template<typename TEnum>
        SKL_FORCEINLINE SKL_NODISCARD TBroadcastType GetBroadcastTypeAs() const noexcept { return static_cast<TEnum>( GetBroadcastType() ); }
        
        //! Get the broadcast payload data
        SKL_FORCEINLINE SKL_NODISCARD uint32_t GetBroadcastPayload() const noexcept
        {
            const uint32_t RawPayload{ *reinterpret_cast<const uint32_t*>( this->GetBuffer() + CPayloadOffset ) };
            return ( RawPayload & CPayloadMask ) >> 8U;
        }
        
        //! Set the broadcast type
        SKL_FORCEINLINE SKL_NODISCARD void SetBroadcastType( TBroadcastType Value ) noexcept { *reinterpret_cast<TBroadcastType*>( this->GetBuffer() + CTypeOffset ) = Value; }

        //! Set the broadcast payload (value-max: 16777215)
        SKL_FORCEINLINE SKL_NODISCARD void SetBroadcastPayload( uint32_t Value ) noexcept
        {
            uint32_t& RawPayload{ *reinterpret_cast<uint32_t*>( this->GetBuffer() + CPayloadOffset ) };
            RawPayload = uint32_t( RawPayload & CTypeMask ) | uint32_t( ( Value << 8U ) & CPayloadMask );
        }
        
        //! Get the broadcast targets count
        SKL_FORCEINLINE SKL_NODISCARD TPacketSize GetBroadcastTargetsCount() const noexcept { return *reinterpret_cast<const TPacketSize*>( this->GetBuffer() + CCountOffset ); }
        
        //! Get the broadcast targets offset
        SKL_FORCEINLINE SKL_NODISCARD TPacketSize GetBroadcastTargetsOffset() const noexcept { return *reinterpret_cast<const TPacketSize*>( this->GetBuffer() + COffsetOffset ); }
        
        //! Calculate the broadcast targets offset
        SKL_FORCEINLINE SKL_NODISCARD TPacketSize CalculateBroadcastTargetsBufferOffset() const noexcept 
        {
            return CalcultateTotalBroadcastPacketSize( 0 );
        }
        
        //! Calculate the total no. of target entities that can be written in this buffer
        SKL_FORCEINLINE SKL_NODISCARD size_t CalculateTotalNoOfTargetsCount() const noexcept
        {
            const TPacketSize OffsetToTargets  { CalculateBroadcastTargetsBufferOffset() }; SKL_ASSERT( OffsetToTargets > 0U );
            const size_t      BytesCount       { this->GetTotalBufferSize() - OffsetToTargets }; SKL_ASSERT( BytesCount >= CEntityIdSize );
            const size_t      TotalTargetsCount{ BytesCount / CEntityIdSize };

            return TotalTargetsCount;
        }

        //! Get the broadcast targets array
        SKL_FORCEINLINE SKL_NODISCARD std::span<const TEntityIdType> GetBroadcastTargets() const noexcept
        {
            return std::span<const TEntityIdType>{
                reinterpret_cast<const TEntityIdType*>( this->GetBuffer() + GetBroadcastTargetsOffset() )
              , static_cast<size_t>( GetBroadcastTargetsCount() )
            };
        }
        
        //! Build new BinaryObjectStream<TEntityIdType> into the buffer where the target can are wrote
        SKL_FORCEINLINE SKL_NODISCARD BinaryObjectStream<TEntityIdType> BuildTargetsStream() noexcept
        {
            const TPacketSize OffsetToTargets{ CalculateBroadcastTargetsBufferOffset() };
            return BinaryObjectStream<TEntityIdType>( 
                  this->GetBuffer() + OffsetToTargets
                , CPacketMaximumSize - OffsetToTargets
                , 0U
                , false
            );
        }
        
        //! Prepare this buffer for sending as broadcast packet
        SKL_FORCEINLINE void PrepareBroadcastBuffer( TBroadcastType Type, uint32_t Payload ) noexcept
        {
            GetBroadcastHeader().Size   = Base::CSizeOfBufferPaddingBeforePacket + Super::CPacketHeaderOffset;
            GetBroadcastHeader().Opcode = CBroadcastPacketOpcode;

            SetBroadcastType( Type );
            SetBroadcastPayload( Payload );
        }

        //! Prepare this buffer for receiving a broadcast packet
        void PrepareFoReceivingBroadcastPacketBody()
        {
            PacketHeader& BHeader{ GetBroadcastHeader() };
            const PacketHeader& PHeader{ this->GetPacketHeader() };

            //1. Copy header data
            BHeader = PHeader;
            
            SKL_ASSERT( CBroadcastPacketOpcode == BHeader.Opcode );
            SKL_ASSERT( CBroadcastPacketOpcode == PHeader.Opcode );
            SKL_ASSERT( BHeader.Size >= static_cast<TPacketSize>( Base::CPacketBodyOffset ) );

            //2. Prepare for receiving the broadcast packet
            StreamBase& Stream{ this->GetStream() };
            Stream.Position       = CPacketHeaderSize;
            Stream.Buffer.Buffer  = GetBroadcastBodyBuffer();
            Stream.Buffer.Length  = BHeader.Size - CPacketHeaderSize;
        }

        //! Prepare this buffer for sending as broadcast packet
        void PrepareForSendingBroadcastPacket( TPacketSize TargetsCount ) noexcept
        {
            const TPacketSize TotalSize{ CalcultateTotalBroadcastPacketSize( TargetsCount ) };

            PacketHeader& BHeader{ GetBroadcastHeader() };
            SKL_ASSERT( CBroadcastPacketOpcode == BHeader.Opcode );
            BHeader.Size = TotalSize;

            StreamBase& Stream{ this->GetStream() }; 
            Stream.Buffer.Buffer = GetBroadcastBuffer();
            Stream.Buffer.Length = TotalSize;

            SetBroadcastTargetsCount( TargetsCount );
            UpdateBroadcastTargetsOffset();
        }
        
        //! Calculate the total size of the broadcasted packet
        SKL_FORCEINLINE SKL_NODISCARD TPacketSize CalcultateTotalBroadcastPacketSize( TPacketSize TargetsCount ) const noexcept
        {
#if !defined(SKL_BUILD_SHIPPING)
            const size_t Result{ static_cast<size_t>( Base::CSizeOfBufferPaddingBeforePacket ) 
                               + static_cast<size_t>( this->GetPacketHeader().Size ) 
                               + ( static_cast<size_t>( TargetsCount ) * CEntityIdSize ) };

            const bool bHasOverflow{ static_cast<size_t>( CPacketMaximumSize ) < Result };

            if( bHasOverflow ) SKL_UNLIKELY
            {
                GLOG_DEBUG( "AsyncNetBuffer::Calcultate TotalBroadcastPacketSize() Overflow!! Broadcast packet size: %llu", Result );
            }
            SKL_ASSERT( false == bHasOverflow );

            return static_cast<TPacketSize>( Result );
#else
            return Base::CSizeOfBufferPaddingBeforePacket + this->GetPacketHeader().Size + ( TargetsCount * CEntityIdSize );
#endif
        }
        
        //! Copy the broadcasted data into Other
        SKL_FORCEINLINE SKL_NODISCARD void CopyBroadcastedDataTo( Super& Other ) const noexcept
        {
            const TPacketSize ToCopy{ CalculateBroadcastTargetsBufferOffset() };
            SKL_ASSERT( ( Base::CSizeOfBufferPaddingBeforePacket + CPacketHeaderSize ) <= ToCopy );
            ( void )SKL_MEMCPY( Other.Buffer, CPacketMaximumSize, this->GetBuffer(), ToCopy );
        }

        //! Build a new ref into the InBuffer interfaces as IAsyncNetBuffer
        SKL_FORCEINLINE SKL_NODISCARD static IBroadcastAsyncNetBuffer& FromBuffer( Super& InBuffer ) noexcept { return reinterpret_cast<IBroadcastAsyncNetBuffer&>( InBuffer ); }
        SKL_FORCEINLINE SKL_NODISCARD static const IBroadcastAsyncNetBuffer& FromBuffer( const Super& InBuffer ) noexcept { return reinterpret_cast<const IBroadcastAsyncNetBuffer&>( InBuffer ); }
        
        //! Build a new ptr into the InBuffer interfaces as IAsyncNetBuffer
        SKL_FORCEINLINE SKL_NODISCARD static IBroadcastAsyncNetBuffer* FromBufferPtr( Super* InBuffer ) noexcept { return reinterpret_cast<IBroadcastAsyncNetBuffer*>( InBuffer ); }
        SKL_FORCEINLINE SKL_NODISCARD static const IBroadcastAsyncNetBuffer* FromBufferPtr( const Super* InBuffer ) noexcept { return reinterpret_cast<const IBroadcastAsyncNetBuffer*>( InBuffer ); }

    protected:
        SKL_FORCEINLINE void SetBroadcastTargetsCount( TPacketSize Count ) noexcept { *reinterpret_cast<TPacketSize*>( this->GetBuffer() + CCountOffset ) = Count; }
        
        SKL_FORCEINLINE void UpdateBroadcastTargetsOffset() noexcept
        {
            const TPacketSize Offset{ Base::CSizeOfBufferPaddingBeforePacket + this->GetPacketHeader().Size };
            *reinterpret_cast<TPacketSize*>( this->GetBuffer() + COffsetOffset ) = Offset;
        }
    };
    
    template<size_t CompletionTaskSize>
    AsyncNetBuffer<CompletionTaskSize>::AsyncNetBuffer() noexcept: Base()
    {
        auto&         Interface    = IAsyncNetBuffer<CompletionTaskSize>::FromBuffer( *this );
        PacketHeader& BufferHeader = Interface.GetBufferHeader();

        BufferHeader.Opcode = CInvalidOpcode;
        BufferHeader.Size   = 0U;
    }

    template<size_t CompletionTaskSize>
    void AsyncNetBuffer<CompletionTaskSize>::Reset() noexcept
    {
        auto&         Interface    = IAsyncNetBuffer<CompletionTaskSize>::FromBuffer( *this );
        PacketHeader& BufferHeader = Interface.GetBufferHeader();

        BufferHeader.Opcode = CInvalidOpcode;
        BufferHeader.Size   = 0U;

        this->ToOSOpaqueObject()->Reset();
    }

    template<size_t CompletionTaskSize>
    SKL_FORCEINLINE SKL_NODISCARD IAsyncNetBuffer<CompletionTaskSize>& EditAsyncNetBuffer( AsyncNetBuffer<CompletionTaskSize>& InBuffer ) noexcept { return reinterpret_cast<IAsyncNetBuffer<CompletionTaskSize>&>( InBuffer ); }
    
    template<size_t CompletionTaskSize>
    SKL_FORCEINLINE SKL_NODISCARD IRoutedAsyncNetBuffer<CompletionTaskSize>& EditRoutingAsyncNetBuffer( AsyncNetBuffer<CompletionTaskSize>& InBuffer ) noexcept { return reinterpret_cast<IRoutedAsyncNetBuffer<CompletionTaskSize>&>( InBuffer ); }
    
    template<typename TEntityIdType, size_t CompletionTaskSize>
    SKL_FORCEINLINE SKL_NODISCARD IBroadcastAsyncNetBuffer<TEntityIdType, CompletionTaskSize>& EditBroadcastAsyncNetBuffer( AsyncNetBuffer<CompletionTaskSize>& InBuffer ) noexcept { return reinterpret_cast<IBroadcastAsyncNetBuffer<TEntityIdType, CompletionTaskSize>&>( InBuffer ); }
}