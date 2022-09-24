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
    struct IBuffer
    {
        IBuffer() noexcept = default;
        IBuffer( uint32_t BufferSize, uint8_t* Buffer ) noexcept
            : Length{ BufferSize }, Buffer{ Buffer } {}
        ~IBuffer() noexcept = default;

        uint32_t Length { 0 };
        //uint32_t Padding;
        uint8_t* Buffer { nullptr };
    };

    struct StreamBase
    {
        StreamBase( uint32_t Position, uint32_t BufferSize, uint8_t* Buffer, bool bOwnsBuffer = false ) noexcept
            :Position{ Position }, bOwnsBuffer{ static_cast<uint32_t>( bOwnsBuffer ) }, Buffer{ BufferSize, Buffer } {}
        ~StreamBase() noexcept = default;

        SKL_FORCEINLINE uint32_t GetPosition() const noexcept { return Position; }

        uint32_t Position   { 0 };
        uint32_t bOwnsBuffer{ FALSE };
        IBuffer  Buffer     {};
    };

    template<bool bIsBase_StreamObjectOrPtrToStreamObject>
    struct alignas( SKL_ALIGNMENT ) IStreamReader
    {
        //! Get the target data this interface operates on
        SKL_FORCEINLINE StreamBase& GetStream() noexcept
        {
            if constexpr( true == bIsBase_StreamObjectOrPtrToStreamObject )
            {
                return *reinterpret_cast<StreamBase*>( this );
            }
            else
            {
                return **reinterpret_cast<StreamBase**>( this );
            }
        }

        //! Get the target data this interface operates on
        SKL_FORCEINLINE const StreamBase& GetStream() const noexcept
        {
            if constexpr( true == bIsBase_StreamObjectOrPtrToStreamObject )
            {
                return *reinterpret_cast<const StreamBase*>( this );
            }
            else
            {
                return **reinterpret_cast<const StreamBase* const *>( this );
            }
        }

        //! Does this instance own its buffer
        SKL_FORCEINLINE bool OwnsBuffer() const noexcept { return TRUE == GetStream().bOwnsBuffer; }

        //! Get buffer 
        SKL_FORCEINLINE const uint8_t* GetBuffer() const noexcept { return GetStream().Buffer.Buffer; }

        //! Get buffer size
        SKL_FORCEINLINE uint32_t GetBufferSize() const noexcept { return GetStream().Buffer.Length; }

        //! Get the current stream position (offset)
        SKL_FORCEINLINE uint32_t GetPosition() const noexcept { return GetStream().Position; }

        //! Push the stream position forward (asserts that GetRemainingSize() >= InAmount)
        SKL_FORCEINLINE void ForwardToEnd( uint32_t InEndOffset = 0 ) noexcept 
        { 
            SKL_ASSERT( InEndOffset <= GetBufferSize() );
            const auto Result{ GetBufferSize() - InEndOffset };
            GetStream().Position = Result;
        }

        //! Push the stream position forward (asserts that GetRemainingSize() >= InAmount)
        SKL_FORCEINLINE void Forward( uint32_t InAmount ) noexcept 
        { 
            const auto Result{ GetPosition() + InAmount };
            SKL_ASSERT( Result <= GetBufferSize() );
            GetStream().Position = Result;
        }

        //! Try to push the stream position forward (fails if GetRemainingSize() < InAmount)
        SKL_FORCEINLINE bool TryForward( uint32_t InAmount ) noexcept 
        { 
            const auto Result{ GetPosition() + InAmount };
            const auto bFits { Result < GetBufferSize() };

            GetStream().Position += InAmount * static_cast<uint32_t>( bFits );

            return bFits;
        }

        //! Try to push the stream position forward (if GetRemainingSize() < InAmount then in forwards to the end)
        SKL_FORCEINLINE bool ForwardTruncate( uint32_t InAmount ) noexcept 
        { 
            const auto Delta          { GetRemainingSize() };
            const auto Result         { GetPosition() + InAmount };
            const auto bShouldTruncate{ Result >= GetBufferSize() };

            GetStream().Position += ( InAmount * static_cast<uint32_t>( !bShouldTruncate ) ) 
                                     + ( Delta    * static_cast<uint32_t>(  bShouldTruncate ) );

            return bShouldTruncate;
        }

        //! Get buffer at the current stream position 
        SKL_FORCEINLINE const uint8_t* GetFront() const noexcept { return &GetStream().Buffer.Buffer[ GetPosition() ]; }

        //! Get buffer at the current stream position as a string ptr
        SKL_FORCEINLINE const char* GetFrontAsString() const noexcept { return reinterpret_cast<const char*>( GetFront() ); }

        //! Get buffer at the current stream position as a wide string ptr
        SKL_FORCEINLINE const wchar_t* GetFrontAsWString() const noexcept { return reinterpret_cast<const wchar_t*>( GetFront() ); }

        //! Get strlen( buffer at the current stream position as a string ptr )
        SKL_FORCEINLINE uint32_t GetFrontAsString_Size() const noexcept { return static_cast<uint32_t>( SKL_STRLEN( GetFrontAsString(), GetRemainingSize() ) ); }

        //! Get wstrlen( buffer at the current stream position as a wide string ptr )
        SKL_FORCEINLINE uint32_t GetFrontAsWString_Size() const noexcept { return static_cast<uint32_t>( SKL_WSTRLEN( GetFrontAsWString(), GetRemainingSize() / 2 ) ); }

        //! Get buffer at the current stream position as a string ptr and advance stream position after the string
        SKL_FORCEINLINE const char* GetFrontAsStringAndAdvance() noexcept 
        { 
            const char* Str{ GetFrontAsString() };
            Forward( static_cast<uint32_t>( SKL_STRLEN( Str, GetRemainingSize() ) + 1 ) );
            return Str;
        }

        //! Get buffer at the current stream position as a string ptr and advance stream position after the string
        SKL_FORCEINLINE const wchar_t* GetFrontAsWStringAndAdvance() noexcept 
        { 
            const wchar_t* Str{ GetFrontAsWString() };
            Forward( static_cast<uint32_t>( ( SKL_WSTRLEN( Str, GetRemainingSize() ) * 2 ) + 2 ) );
            return Str;
        }

        //! Set the current stream position (offset)
        SKL_FORCEINLINE void SetPosition( uint32_t InPosition ) noexcept 
        { 
            SKL_ASSERT( InPosition < GetBufferSize() );
            GetStream().Position = InPosition;
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
        SKL_FORCEINLINE uint32_t GetRemainingSize() const noexcept { return GetBufferSize() - GetPosition(); }

        //! Cast &Buffer[ CurrentPosition ] to const T& and return copy of T
        template<typename T>
        SKL_FORCEINLINE T ReadT( ) noexcept
        {
            constexpr auto TSize{ static_cast<uint32_t>( sizeof( T ) ) };
            Forward( TSize );
            return *reinterpret_cast< const T * >( GetFront( ) - TSize );
        }

        //! Has the stream position reached the end of the buffer
        SKL_FORCEINLINE bool IsEOS() const noexcept { return 0 == GetRemainingSize(); }

        //! Has a valid buffer
        SKL_FORCEINLINE bool IsValid() const noexcept { return nullptr != GetBuffer(); }

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

            const auto RemainingSize{ GetRemainingSize() };
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

        //! Save the underlying buffer to a file
        bool SaveToFile( const char* InFileName, bool bAppendInsteadOfTruncate = true, bool bPositionAsSize = true, bool bSaveAsText = false ) const noexcept
        {
            const auto WriteSize{ ( GetPosition( ) * bPositionAsSize ) + ( GetBufferSize() * !bPositionAsSize ) };
            if( 0 == WriteSize )
            {
                return false;
            }

            auto File{ std::ofstream( InFileName
                                     , ( std::ofstream::binary * !bSaveAsText ) 
                                     | ( std::ofstream::app    *  bAppendInsteadOfTruncate ) 
                                     | ( std::ofstream::trunc  * !bAppendInsteadOfTruncate ) ) };
            if( false == File.is_open( ) )
            {
                //SKL_WRN_FMT( "IStreamReader::SaveToFile(InFileName) Failed to open file %s", InFileName );
                return false;
            }

            File.write( reinterpret_cast< const char * >( GetBuffer( ) ), WriteSize );

            const auto Result{ File.good() };
            if( false == Result )
            {
                //SKL_WRN_FMT( "IStreamReader::SaveToFile(InFileName) Failed to write bytesCount:%u to file %s ", WriteSize, InFileName );
            }

            File.close();

            return Result;
        }

        IStreamReader() noexcept = default;
        IStreamReader( const uint8_t* InBuffer, uint32_t InSize, uint32_t InPosition = 0 ) noexcept
        {
            auto& Interface{ GetStream() };
            Interface.Position      = InPosition;
            Interface.Buffer.Length = InSize;
            Interface.Buffer.Buffer = const_cast<uint8_t*>( InBuffer );
        }

        SKL_FORCEINLINE static IStreamReader<true>* FromStreamBase( StreamBase& InStream ) noexcept
        {
            return reinterpret_cast<IStreamReader<true>*>( &InStream );
        }
    };

    template<bool bIsBase_StreamObjectOrPtrToStreamObject>
    struct alignas( SKL_ALIGNMENT ) IStreamWriter: public IStreamReader<bIsBase_StreamObjectOrPtrToStreamObject>
    {
        //! Can fit InAmount bytes into the stream starting at the current stream position
        SKL_FORCEINLINE bool CanFit( uint32_t InAmount ) const noexcept { return this->GetRemainingSize() >= InAmount; }

        //! Get buffer at the current stream position 
        SKL_FORCEINLINE uint8_t* GetFront() const noexcept { return &this->GetStream().Buffer.Buffer[ this->GetPosition() ]; }

        //! Get buffer 
        SKL_FORCEINLINE uint8_t* GetBuffer() const noexcept { return this->GetStream().Buffer.Buffer; }
        
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
            SKL_ASSERT( nullptr != this->GetBuffer() );

            if( this->IsEOS() ) { return true; }

            const auto RemainingSize       { this->GetRemainingSize() };
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

            this->Forward( ActualWriteSize );

            return true;
        }
        
        //! Write value into the buffer
        template<typename T>
        SKL_FORCEINLINE void WriteT( T InValue ) noexcept
        {
             *reinterpret_cast<T*>( GetFront() ) = InValue;
             this->Forward( static_cast<uint32_t>( sizeof( T ) ) );
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
            const auto Result{ Write( reinterpret_cast<const uint8_t*>( String ), static_cast<uint32_t>( N ), false ) };
            SKL_ASSERT( true == Result );
        }
        
        //! Write string into buffer at current stream position, with an exact count characters
        template<size_t N>
        void WriteWString( const wchar_t( &String )[N] ) noexcept
        {
            const auto Result{ Write( reinterpret_cast<const uint8_t*>( String ), static_cast<uint32_t>( N ) * 2, false ) };
            SKL_ASSERT( true == Result );
        }
    
        //! Zero the whole buffer
        SKL_FORCEINLINE void ZeroBuffer() noexcept 
        { 
            SKL_ASSERT( nullptr != this->GetBuffer() );
            SKL_ASSERT( 0 != this->GetBufferSize() );
            memset( GetBuffer(), 0, this->GetBufferSize() );
        }

        //! Zero the the remaining portion of the buffer (does nothing if IsEOS() == true)
        SKL_FORCEINLINE void ZeroRemainingBuffer() noexcept 
        { 
            SKL_ASSERT( nullptr != this->GetBuffer() );
            SKL_ASSERT( 0 != this->GetBufferSize() );
            if( true == this->IsEOS() ){ return; }
            memset( GetFront(), 0, this->GetRemainingSize() );
        }

        //! Fill the buffer with bytes read from file (if bTruncate == true, reads at most GetBufferSize(), otherwise returns false)
        bool ReadFromFile( const char* InFileName, bool bTruncate = false ) noexcept
        {
            auto File{ std::ifstream( InFileName, std::ifstream::binary ) };
            if( false == File.is_open( ) )
            {
                //SKL_WRN_FMT( "IStreamWriter::ReadFromFile(InFileName) Failed to open file %s", InFileName );
                return false;
            } 

            File.seekg( 0, std::ifstream::end );
            const uint32_t ReadSize{ static_cast< uint32_t >( File.tellg( ) ) };
            File.seekg( 0, std::ifstream::beg );

            if( 0 == ReadSize ) SKL_UNLIKELY
            {
                //SKL_WRN_FMT( "IStreamWriter::ReadFromFile(InFileName) Empty file %s", InFileName );
                File.close();
                return false;
            }

            const auto bItFits{ this->CanFit( ReadSize ) };
            if( false == bItFits && false == bTruncate )
            {
                //SKL_VER_FMT( "IStreamWriter::ReadFromFile(InFileName) Failed to read file %s! Exceeds buffer size. BufferSize:%u FileSize:%u", InFileName, this->GetRemainingSize(), ReadSize );
                File.close();
                return false;
            }

            File.read( reinterpret_cast<char*>( this->GetFront() ), ReadSize );

            const auto Result{ File.good() };
            if( false == Result )
            {
                //SKL_WRN_FMT( "IStreamReader::ReadFromFile(InFileName) Failed to read bytesCount:%u to file %s ", ReadSize, InFileName );
            }

            File.close();

            return Result;
        }

        IStreamWriter() noexcept = default;
        IStreamWriter( uint8_t* InBuffer, uint32_t InSize, uint32_t InPosition = 0 ) noexcept
            : IStreamReader{ InBuffer, InSize, InPosition } {}

        SKL_FORCEINLINE static IStreamWriter<true>* FromStreamBase( StreamBase& InStream ) noexcept
        {
            return reinterpret_cast<IStreamWriter<true>*>( &InStream );
        }
    };

    template<bool bIsBase_StreamObjectOrPtrToStreamObject>
    struct alignas( SKL_ALIGNMENT ) IBinaryStream: public IStreamWriter<bIsBase_StreamObjectOrPtrToStreamObject>
    {
        IBinaryStream() noexcept = default;
        IBinaryStream( uint8_t* InBuffer, uint32_t InSize, uint32_t InPosition = 0 ) noexcept
            : IStreamWriter{ InBuffer, InSize, InPosition } {}

        SKL_FORCEINLINE static IBinaryStream<true>* FromStreamBase( StreamBase& InStream ) noexcept
        {
            return reinterpret_cast<IBinaryStream<true>*>( &InStream );
        }
    };

    struct alignas( SKL_ALIGNMENT ) BinaryStream: public IBinaryStream<true>
    {
        BinaryStream() noexcept = default;
        BinaryStream( uint8_t* InBuffer, uint32_t InSize, uint32_t InPosition, bool bOwnsBuffer ) noexcept
            : Base{ InPosition, InSize, InBuffer, bOwnsBuffer } {}

        BinaryStream( const BinaryStream& ) noexcept = default;
        BinaryStream& operator=( const BinaryStream& ) noexcept = default;

    protected:
        StreamBase Base;
    };

    struct alignas( SKL_ALIGNMENT ) BufferStreamInterface : public IBinaryStream<false>
    {
        BufferStreamInterface( StreamBase* SourceStream ) noexcept
            : IBinaryStream{}, SourceBase{ SourceStream } {}
        ~BufferStreamInterface() noexcept = default;

        BufferStreamInterface( const BufferStreamInterface& Other ) noexcept
            : SourceBase{ Other.SourceBase } {}
        BufferStreamInterface& operator=( const BufferStreamInterface& Other ) noexcept
        {
            SKL_ASSERT( this != &Other );
            SourceBase = Other.SourceBase;
            return *this;
        }

    private:
        StreamBase* SourceBase;
    };

    struct alignas( SKL_ALIGNMENT ) BufferStreamTransaction : public IBinaryStream<true>
    {
        BufferStreamTransaction( StreamBase* SourceStream ) noexcept
            : IBinaryStream{}, Base{ *SourceStream }, SourceBase{ SourceStream }
        {
            Base.bOwnsBuffer   = FALSE;
            Base.Buffer.Length = GetRemainingSize();
            Base.Buffer.Buffer = GetFront();
            Base.Position      = 0;
        }
        ~BufferStreamTransaction() noexcept
        {
            Commit();
            Release();
        }

        BufferStreamTransaction( const BufferStreamTransaction& Other ) noexcept
            : Base{ Other.Base }, SourceBase{ Other.SourceBase }
        {
            SKL_ASSERT( false == Other.OwnsBuffer() );
        }
        BufferStreamTransaction& operator=( const BufferStreamTransaction& Other ) noexcept
        {
            SKL_ASSERT( this != &Other );
            Base       = Other.Base;
            SourceBase = Other.SourceBase;
            SKL_ASSERT( false == Other.OwnsBuffer() );
            return *this;
        }

        //!
        //! "Commits" changes to the underlying Stream
        //! 
        //! \remarks After Commiting, the Buffer(ptr) is pushed forward by the committed amount and the Position is reset to 0 (rebased)
        //! 
        void CommitAndRebase() noexcept
        {
            // Commit to base 
            SourceBase->Position += GetPosition();

            // Rebase
            Base.Buffer.Length    = GetRemainingSize();
            Base.Buffer.Buffer   += GetPosition();
            Base.Position         = 0;
        }

        //! "Commits" changes to the underlying Stream
        SKL_FORCEINLINE void Commit() noexcept
        {
            // Commit to base 
            SourceBase->Position += GetPosition();
        }

        //! "Rolls back" changes by resetting the Position to 0
        SKL_FORCEINLINE void Rollback( ) noexcept
        {
            Base.Position = 0;
        }

        //! Release the underlying stream
        void Release() noexcept
        {
            Base.Position      = 0;
            Base.Buffer.Length = 0;
            Base.Buffer.Buffer = nullptr;
            SourceBase         = nullptr;
        }

    private:
        StreamBase  Base;
        StreamBase* SourceBase;
    };
}