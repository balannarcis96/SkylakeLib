//!
//! \file BinaryStream.h
//! 
//! \brief Binary stream maipulation abstractions
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    struct IStreamBase
    {
        uint32_t Position  { 0 };
        uint32_t BufferSize{ 0 };
        uint8_t* Buffer    { nullptr };
    };

    struct IStreamWriter;
    struct IBinaryStream;

    struct alignas( SKL_ALIGNMENT ) IStreamReader
    {
        //! Cast &Buffer[ CurrentPosition ] to const T& and return copy of T
        template<typename T>
        SKL_FORCEINLINE T ReadT( ) noexcept
        {
            constexpr auto TSize{ static_cast<uint32_t>( sizeof( T ) ) };
            Forward( TSize );
            return *reinterpret_cast< const T * >( GetFront( ) - TSize );
        }

        //! Push the stream position forward (asserts that GetRemainingSize() >= InAmount)
        SKL_FORCEINLINE void Forward( uint32_t InAmount ) noexcept 
        { 
            const auto Result{ GetPosition() + InAmount };
            SKL_ASSERT( Result < BufferSize );
            GetInterface().Position = Result;
        }
        
        //! Try to push the stream position forward (fails if GetRemainingSize() < InAmount)
        SKL_FORCEINLINE bool TryForward( uint32_t InAmount ) noexcept 
        { 
            const auto Result{ GetPosition() + InAmount };
            const auto bFits { Result < GetBufferSize() };

            GetInterface().Position += InAmount * static_cast<uint32_t>( bFits );

            return bFits;
        }
        
        //! Try to push the stream position forward (if GetRemainingSize() < InAmount then in forwards to the end)
        SKL_FORCEINLINE bool ForwardTruncate( uint32_t InAmount ) noexcept 
        { 
            const auto Delta          { GetRemnainingSize() };
            const auto Result         { GetPosition() + InAmount };
            const auto bShouldTruncate{ Result >= GetBufferSize() };

            GetInterface().Position += ( InAmount * static_cast<uint32_t>( !bShouldTruncate ) ) 
                                     + ( Delta     * static_cast<uint32_t>(  bShouldTruncate ) );

            return bShouldTruncate;
        }
       
        //! Get buffer at the current stream position 
        SKL_FORCEINLINE const uint8_t* GetFront() const noexcept { return &GetInterface().Buffer[ GetPosition() ]; }
        
        //! Get buffer 
        SKL_FORCEINLINE const uint8_t* GetBuffer() const noexcept { return GetInterface().Buffer; }
        
        //! Get buffer size
        SKL_FORCEINLINE uint32_t GetBufferSize() const noexcept { return GetInterface().BufferSize; }
        
        //! Get the current stream position (offset)
        SKL_FORCEINLINE uint32_t GetPosition() const noexcept { return GetInterface().Position; }
        
        //! Set the current stream position (offset)
        SKL_FORCEINLINE void SetPosition( uint32_t InPosition ) noexcept 
        { 
            SKL_ASSERT( InPosition < GetBufferSize() );
            GetInterface().Position = InPosition;
        }
        
        //! Set the current stream position (offset) and get the old one back
        SKL_FORCEINLINE uint32_t SwapPosition( uint32_t InPosition ) noexcept 
        { 
            SKL_ASSERT( InPosition < GetBufferSize() );
            const auto CurrentPosition{ GetPosition() };
            SetPosition( InPosition );
            return CurrentPosition;
        }

        //! Get the remaining size of the buffer that
        SKL_FORCEINLINE uint32_t GetRemnainingSize() const noexcept { return GetBufferSize() - GetPosition(); }

        //! Has the stream position reached the end of the buffer
        SKL_FORCEINLINE bool IsEOS() const noexcept { return 0 == GetRemnainingSize(); }

        //! Reset the stream position to the begining of the buffer
        SKL_FORCEINLINE void Reset() noexcept
        {
            SetPosition( 0 );
        }

        //! Read InReadSize bytes from the internal buffer, at the current position, into OutBuffer (if bTruncate is true, reads till end if GetRemainingSize() < InReadSize)
        bool Read( uint8_t* OutBuffer, uint32_t InReadSize, bool bTruncate = false ) noexcept
        {
            SKL_ASSERT( nullptr != OutBuffer );
            SKL_ASSERT( 0 != InReadSize );

            const auto RemainingSize{ GetRemnainingSize() };
            const auto bFits        { InReadSize <= RemainingSize };

            if( false == bFits && false == bTruncate )
            {
                return false;
            }

            const auto ActualReadSize{ 
                  ( InReadSize    * static_cast<uint32_t>(  bFits ) )
                + ( RemainingSize * static_cast<uint32_t>( !bFits ) )
            };

            const auto* Result{ memcpy( OutBuffer, GetFront(), ActualReadSize ) };
            SKL_ASSERT( OutBuffer == Result );

            return true;
        }

        //! Get the buffer at the current position as a ref to TObject class type
        template<typename TObject> requires( std::is_class_v<TObject> )
        SKL_FORCEINLINE const TObject& BuildObjectRef() const noexcept
        {
            return *reinterpret_cast<TObject*>( GetFront() );
        }

        //! Get the target data this interface operates on
        SKL_FORCEINLINE IStreamBase& GetInterface() noexcept
        {
            return *reinterpret_cast<IStreamBase*>( this );
        }

        //! Get the target data this interface operates on
        SKL_FORCEINLINE const IStreamBase& GetInterface() const noexcept
        {
            return *reinterpret_cast<const IStreamBase*>( this );
        }

    private:
        IStreamReader( const uint8_t* InBuffer, uint32_t InSize, uint32_t InPosition = 0 ) noexcept
        {
            auto& Interface{ GetInterface() };
            Interface.Position   = InPosition;
            Interface.BufferSize = InSize;
            Interface.Buffer     = const_cast<uint8_t*>( InBuffer );
        }

        friend IStreamWriter;
    };

    struct alignas( SKL_ALIGNMENT ) IStreamWriter: public IStreamReader
    {
        //! Can fit InAmount bytes into the stream starting at the current stream position
        SKL_FORCEINLINE bool CanFit( uint32_t InAmount ) const noexcept { return GetRemnainingSize() >= InAmount; }

        //! Get buffer at the current stream position 
        SKL_FORCEINLINE uint8_t* GetFront() const noexcept { return &GetInterface().Buffer[ GetPosition() ]; }
        
        //! Get buffer 
        SKL_FORCEINLINE uint8_t* GetBuffer() const noexcept { return GetInterface().Buffer; }
        
        //! Get the buffer at the current position as a ref to TObject class type
        template<typename TObject> requires( std::is_class_v<TObject> )
        SKL_FORCEINLINE TObject& BuildObjectRef() noexcept
        {
            return *reinterpret_cast<TObject*>( GetFront() );
        }

        //! Write InWriteSize bytes into the internal buffer, at the current position, from InBuffer (if bTruncate is true, writes only what fits)
        bool Write( const uint8_t* InBuffer, uint32_t InWriteSize, bool bTruncate = false ) noexcept
        {
            SKL_ASSERT( nullptr != InBuffer );
            SKL_ASSERT( 0 != InWriteSize );
            SKL_ASSERT( nullptr != GetBuffer() );

            if( IsEOS() ) { return true; }

            const auto RemainingSize       { GetRemnainingSize() };
            const auto bHasRequestedAmount{ InWriteSize <= RemainingSize };

            if( false == bHasRequestedAmount && false == bTruncate )
            {
                return false;
            }

            const auto ActualWriteSize{ 
                  ( InWriteSize   * static_cast<uint32_t>(  bHasRequestedAmount ) )
                + ( RemainingSize * static_cast<uint32_t>( !bHasRequestedAmount ) )
            };

            const auto* Result{ memcpy( GetFront(), InBuffer, ActualWriteSize ) };
            SKL_ASSERT( GetFront() == Result );

            Forward( ActualWriteSize );

            return true;
        }
        
        //! Write value into the buffer
        template<typename T> requires( false == std::is_class_v<T> )
        SKL_FORCEINLINE void WriteT( T InValue ) noexcept
        {
             *reinterpret_cast<T*>( GetFront() ) = InValue;
            Forward( static_cast<uint32_t>( InValue ) );
        }
        
        //! Write object by moving into the buffer
        template<typename T> requires( true == std::is_class_v<T> && ( true == std::is_nothrow_move_assignable_v<T> || true == std::is_nothrow_move_constructible_v<T> ) )
        SKL_FORCEINLINE void WriteT( T&& InValue ) noexcept
        {
            if constexpr( true == std::is_nothrow_move_constructible_v<T> )
            {
                new( GetFront() ) T( std::forward<T>( InValue ) );
            }
            else
            {
                *reinterpret_cast<T*>( GetFront() ) = InValue;
            }

            Forward( static_cast<uint32_t>( InValue ) );
        }
        
        //! Write object by copying into the buffer
        template<typename T> requires( true == std::is_class_v<T> && ( true == std::is_nothrow_copy_assignable_v<T> || true == std::is_nothrow_copy_constructible_v<T> ) )
        SKL_FORCEINLINE void WriteT( const T& InValue ) noexcept
        {
            if constexpr( true == std::is_nothrow_copy_constructible_v<T> )
            {
                new( GetFront() ) T( InValue );
            }
            else
            {
                *reinterpret_cast<T*>( GetFront() ) = InValue;
            }

            Forward( static_cast<uint32_t>( InValue ) );
        }

        //! Write string into buffer at current stream position, with a maximum of MaxCount of characters
        void WriteString( const char* InString, size_t MaxCount ) noexcept
        {
            const auto InStringLength{ SKL_STRLEN( InString, MaxCount ) };
            const auto Result{ Write( reinterpret_cast<const uint8_t*>( InString ), static_cast<uint32_t>( InStringLength + 1 ), false ) };
            SKL_ASSERT( true == Result );
        }
        
        //! Write wide string into buffer at current stream position, with a maximum of MaxCount of characters
        void WriteWString( const wchar_t* InString, size_t MaxCountWords ) noexcept
        {
            const auto InStringLength{ SKL_WSTRLEN( InString, MaxCountWords ) };
            const auto Result{ Write( reinterpret_cast<const uint8_t*>( InString ), static_cast<uint32_t>( InStringLength + 1 ) * 2, false ) };
            SKL_ASSERT( true == Result );
        }

        //! Write string into buffer at current stream position, with an exact count characters
        template<size_t N>
        void WriteString( const char( &String )[N] ) noexcept
        {
            const auto Result{ Write( String, static_cast<uint32_t>( N ), false ) };
            SKL_ASSERT( true == Result );
        }
        
        //! Write string into buffer at current stream position, with an exact count characters
        template<size_t N>
        void WriteWString( const wchar_t( &String )[N] ) noexcept
        {
            const auto Result{ Write( String, static_cast<uint32_t>( N ) * 2, false ) };
            SKL_ASSERT( true == Result );
        }
    
    private:
        IStreamWriter( uint8_t* InBuffer, uint32_t InSize, uint32_t InPosition = 0 ) noexcept
            : IStreamReader{ InBuffer, InSize, InPosition } {}

        friend IBinaryStream;
    };

    struct IBinaryStream: public IStreamWriter
    {
    private:
        IBinaryStream( uint8_t* InBuffer, uint32_t InSize, uint32_t InPosition = 0 ) noexcept
            : IStreamWriter{ InBuffer, InSize, InPosition } {}
    };
}