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

        //! Construct span for the internal buffer
        SKL_FORCEINLINE std::span<uint8_t> get_span() noexcept
        {
            return { GetInterface().Buffer, static_cast<size_t>( GetInterface().Length ) };
        }

        //! Construct a new stream interface for this async IO buffer
        SKL_FORCEINLINE BufferStreamInterface GetStreamInterface() noexcept
        {
            return BufferStreamInterface{ &Stream };
        }

        //! Construct a new binary stream interface for this async IO buffer
        SKL_FORCEINLINE IBinaryStream<true>* GetStream() noexcept
        {
            return IBinaryStream<true>::FromStreamBase( Stream );
        }

        //! Construct stream transaction interface into this buffer at the current position
        SKL_FORCEINLINE BufferStreamTransaction NewTransaction() noexcept
        {
            return BufferStreamTransaction{ &Stream };
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
        StreamBase       Stream;        //!< Cached buffer data and manipulation info
    };

    template<uint32_t BufferSize, size_t CompletionTaskSize = 16> 
    struct AsyncIOBuffer : IAsyncIOTask
    {
        static_assert( CompletionTaskSize % 8 == 0 );

        using TDispatch = ASD::UniqueFunctorWrapper<CompletionTaskSize, typename IAsyncIOTask::TDispatchFunctionPtr>;
        
        AsyncIOBuffer() noexcept 
            : IAsyncIOTask( IBuffer{ BufferSize, Buffer } ), OnDispatch{} {}

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

    private:
        TDispatch OnDispatch;           //!< The functor to dispatch when the async IO operation is completed
        uint8_t   Buffer[ BufferSize ]; //!< The buffer to carry the data
    };

#define SKL_ASYNCIO_BUFFER_TRANSACTION( BufferPtr )          \
    if( auto Transaction{ NewTask->NewTransaction() }; true )
}