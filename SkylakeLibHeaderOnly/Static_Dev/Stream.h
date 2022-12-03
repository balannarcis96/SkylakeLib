//!
//! \file BinaryStream.h
//! 
//! \brief Binary stream manipulation abstractions
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    struct IBuffer
    {
        IBuffer() noexcept
            : Length{ 0U }
            , Padding{ 0U }
            , Buffer{ nullptr } {}

        IBuffer( uint32_t BufferSize, uint8_t* Buffer ) noexcept
            : Length{ BufferSize }
            , Padding{ 0 }
            , Buffer{ Buffer } {}
            
        IBuffer( const IBuffer& Other ) noexcept = default;
        IBuffer& operator=( const IBuffer& ) noexcept = default;
        
        IBuffer( IBuffer&& Other ) noexcept
            : Length{ Other.Length }
            , Padding{ 0U }
            , Buffer{ Other.Buffer } 
        {
            Other.Length = 0U;
            Other.Buffer = nullptr;
        }
        IBuffer& operator=( IBuffer&& Other ) noexcept
        {
            SKL_ASSERT( this != &Other );

            Length  = Other.Length;
            Padding = 0U;
            Buffer  = Other.Buffer;

            Other.Length = 0U;
            Other.Buffer = nullptr;

            return *this;
        }

        ~IBuffer() noexcept = default;

        uint32_t Length;
        uint32_t Padding;
        uint8_t* Buffer;
    };

    struct StreamBase
    {
        StreamBase() noexcept
            : Position{ 0U }
            , bOwnsBuffer{ FALSE }
            , Buffer{} {}

        StreamBase( uint32_t Position, uint32_t BufferSize, uint8_t* Buffer, bool bOwnsBuffer = false ) noexcept
            : Position{ Position }
            , bOwnsBuffer{ static_cast<uint32_t>( bOwnsBuffer ) }
            , Buffer{ BufferSize, Buffer } {}

        StreamBase( const StreamBase& ) noexcept = default;
        StreamBase& operator=( const StreamBase& ) noexcept = default;

        StreamBase( StreamBase&& Other ) noexcept 
            : Position{ Other.Position }
            , bOwnsBuffer{ Other.bOwnsBuffer }
            , Buffer{ std::move( Other.Buffer ) } {}
        StreamBase& operator=( StreamBase&& Other ) noexcept
        {
            SKL_ASSERT( this != &Other );

            Position    = Other.Position;
            bOwnsBuffer = Other.bOwnsBuffer;
            Buffer      = std::move( Other.Buffer );

            Other.Position    = 0U;
            Other.bOwnsBuffer = FALSE;

            return *this;
        }
        
        ~StreamBase() noexcept = default;

        SKL_FORCEINLINE SKL_NODISCARD uint32_t GetPosition()      const noexcept { return Position; }
        SKL_FORCEINLINE SKL_NODISCARD uint32_t GetBufferLength()  const noexcept { return Buffer.Length; }
        SKL_FORCEINLINE SKL_NODISCARD uint8_t* GetBuffer()        const noexcept { return Buffer.Buffer; }
        SKL_FORCEINLINE SKL_NODISCARD uint8_t* GetFront()         const noexcept { return GetBuffer() + GetPosition(); }
        SKL_FORCEINLINE SKL_NODISCARD uint32_t GetRemainingSize() const noexcept { return GetBufferLength() - GetPosition(); }
        SKL_FORCEINLINE SKL_NODISCARD bool     OwnsBuffer()       const noexcept { return 0 != bOwnsBuffer; }
        SKL_FORCEINLINE SKL_NODISCARD bool     IsEOS()            const noexcept { return 0 == GetRemainingSize(); }

        template<typename T> SKL_FORCEINLINE SKL_NODISCARD       T* GetBufferAsTypePtr()       noexcept { return  reinterpret_cast<T*>      ( GetBuffer() ); }
        template<typename T> SKL_FORCEINLINE SKL_NODISCARD const T* GetBufferAsTypePtr() const noexcept { return  reinterpret_cast<const T*>( GetBuffer() ); }
        template<typename T> SKL_FORCEINLINE SKL_NODISCARD       T& GetBufferAsTypeRef()       noexcept { return *reinterpret_cast<T*>      ( GetBuffer() ); }
        template<typename T> SKL_FORCEINLINE SKL_NODISCARD const T& GetBufferAsTypeRef() const noexcept { return *reinterpret_cast<const T*>( GetBuffer() ); }
        template<typename T> SKL_FORCEINLINE SKL_NODISCARD       T  GetBufferAsTypeVal() const noexcept { return T{ GetBufferAsTypeRef() }; }
        
        template<typename T> SKL_FORCEINLINE SKL_NODISCARD       T* GetFrontAsTypePtr()       noexcept { return  reinterpret_cast<T*>      ( GetFront() ); }
        template<typename T> SKL_FORCEINLINE SKL_NODISCARD const T* GetFrontAsTypePtr() const noexcept { return  reinterpret_cast<const T*>( GetFront() ); }
        template<typename T> SKL_FORCEINLINE SKL_NODISCARD       T& GetFrontAsTypeRef()       noexcept { return *reinterpret_cast<T*>      ( GetFront() ); }
        template<typename T> SKL_FORCEINLINE SKL_NODISCARD const T& GetFrontAsTypeRef() const noexcept { return *reinterpret_cast<const T*>( GetFront() ); }
        template<typename T> SKL_FORCEINLINE SKL_NODISCARD       T  GetFrontAsTypeVal() const noexcept { return T{ GetFrontAsTypeRef() }; }

        uint32_t Position;   
        uint32_t bOwnsBuffer;
        IBuffer  Buffer;
    };

    template<bool bIsBase_StreamObjectOrPtrToStreamObject>
    struct IStreamReader
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
            
            this->Forward( ActualReadSize );

            return true;
        }

        //! Get the buffer at the current position as a ref to TObject class type
        template<typename TObject>
        SKL_FORCEINLINE const TObject& BuildObjectRef() const noexcept
        {
            static_assert( std::is_class_v<TObject> );
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
                //SKLL_WRN_FMT( "IStreamReader::SaveToFile(InFileName) Failed to open file %s", InFileName );
                return false;
            }

            File.write( reinterpret_cast< const char * >( GetBuffer( ) ), WriteSize );

            const auto Result{ File.good() };
            if( false == Result )
            {
                //SKLL_WRN_FMT( "IStreamReader::SaveToFile(InFileName) Failed to write bytesCount:%u to file %s ", WriteSize, InFileName );
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
        
        SKL_FORCEINLINE static IStreamReader<true>& FromStreamBaseRef( StreamBase& InStream ) noexcept
        {
            return reinterpret_cast<IStreamReader<true>&>( InStream );
        }
    };

    using IStreamObjectReader    = IStreamReader<true>;
    using IStreamObjectPtrReader = IStreamReader<false>;

    template<bool bIsBase_StreamObjectOrPtrToStreamObject>
    struct IStreamWriter: public IStreamReader<bIsBase_StreamObjectOrPtrToStreamObject>
    {
        //! Can fit InAmount bytes into the stream starting at the current stream position
        SKL_FORCEINLINE bool CanFit( uint32_t InAmount ) const noexcept { return this->GetRemainingSize() >= InAmount; }

        //! Get buffer at the current stream position 
        SKL_FORCEINLINE uint8_t* GetFront() const noexcept { return &this->GetStream().Buffer.Buffer[ this->GetPosition() ]; }

        //! Get buffer 
        SKL_FORCEINLINE uint8_t* GetBuffer() const noexcept { return this->GetStream().Buffer.Buffer; }
        
        //! Get the buffer at the current position as a ref to TObject class type
        template<typename TObject>
        SKL_FORCEINLINE TObject& BuildObjectRef() noexcept
        {
            static_assert( std::is_class_v<TObject> );
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
                //SKLL_WRN_FMT( "IStreamWriter::ReadFromFile(InFileName) Failed to open file %s", InFileName );
                return false;
            } 

            File.seekg( 0, std::ifstream::end );
            const uint32_t ReadSize{ static_cast<uint32_t>( File.tellg( ) ) };
            File.seekg( 0, std::ifstream::beg );

            if( 0 == ReadSize ) SKL_UNLIKELY
            {
                //SKLL_WRN_FMT( "IStreamWriter::ReadFromFile(InFileName) Empty file %s", InFileName );
                File.close();
                return false;
            }

            const auto bItFits{ CanFit( ReadSize + 1 ) };
            if( false == bItFits && false == bTruncate )
            {
                //SKLL_VER_FMT( "IStreamWriter::ReadFromFile(InFileName) Failed to read file %s! Exceeds buffer size. BufferSize:%u FileSize:%u", InFileName, this->GetRemainingSize(), ReadSize );
                File.close();
                return false;
            }

            File.read( reinterpret_cast<char*>( GetFront() ), ReadSize );

            const auto Result{ File.good() };
            if( false == Result )
            {
                //SKLL_WRN_FMT( "IStreamReader::ReadFromFile(InFileName) Failed to read bytesCount:%u to file %s ", ReadSize, InFileName );
            }

            File.close();

            GetBuffer()[ReadSize] = '\0';

            return Result;
        }

        IStreamWriter() noexcept = default;
        IStreamWriter( uint8_t* InBuffer, uint32_t InSize, uint32_t InPosition = 0 ) noexcept
            : IStreamReader{ InBuffer, InSize, InPosition } {}

        SKL_FORCEINLINE static IStreamWriter<true>* FromStreamBase( StreamBase& InStream ) noexcept
        {
            return reinterpret_cast<IStreamWriter<true>*>( &InStream );
        }
        
        SKL_FORCEINLINE static IStreamWriter<true>& FromStreamBaseRef( StreamBase& InStream ) noexcept
        {
            return reinterpret_cast<IStreamWriter<true>&>( InStream );
        }
    };
    
    using IStreamObjectWriter    = IStreamWriter<true>;
    using IStreamObjectPtrWriter = IStreamWriter<false>;

    template<bool bIsBase_StreamObjectOrPtrToStreamObject>
    struct IBinaryStream: public IStreamWriter<bIsBase_StreamObjectOrPtrToStreamObject>
    {
        IBinaryStream() noexcept = default;
        IBinaryStream( uint8_t* InBuffer, uint32_t InSize, uint32_t InPosition = 0 ) noexcept
            : IStreamWriter{ InBuffer, InSize, InPosition } {}

        SKL_FORCEINLINE static IBinaryStream<true>* FromStreamBase( StreamBase& InStream ) noexcept
        {
            return reinterpret_cast<IBinaryStream<true>*>( &InStream );
        }

        SKL_FORCEINLINE static IBinaryStream<true>& FromStreamBaseRef( StreamBase& InStream ) noexcept
        {
            return reinterpret_cast<IBinaryStream<true>&>( InStream );
        }
    };
    
    using IBinaryStreamObject    = IBinaryStream<true>;
    using IBinaryStreamObjectPtr = IBinaryStream<false>;

    struct alignas( SKL_ALIGNMENT ) BinaryStream: public IBinaryStreamObject
    {
        BinaryStream() noexcept = default;
        BinaryStream( uint8_t* InBuffer, uint32_t InSize, uint32_t InPosition, bool bOwnsBuffer ) noexcept
            : Stream{ InPosition, InSize, InBuffer, bOwnsBuffer } {}

        BinaryStream( const BinaryStream& ) noexcept = default;
        BinaryStream& operator=( const BinaryStream& ) noexcept = default;

        SKL_FORCEINLINE StreamBase& GetStreamBase() noexcept { return Stream; }
        SKL_FORCEINLINE const StreamBase& GetStreamBase() const noexcept { return Stream; }

    protected:
        StreamBase Stream{};
    };

    struct alignas( SKL_ALIGNMENT ) BinaryStreamInterface : public IBinaryStreamObjectPtr
    {
        BinaryStreamInterface( StreamBase* SourceStream ) noexcept
            : IBinaryStream{}, SourceBase{ SourceStream } {}
        ~BinaryStreamInterface() noexcept = default;

        BinaryStreamInterface( const BinaryStreamInterface& Other ) noexcept
            : SourceBase{ Other.SourceBase } {}
        BinaryStreamInterface& operator=( const BinaryStreamInterface& Other ) noexcept
        {
            SKL_ASSERT( this != &Other );
            SourceBase = Other.SourceBase;
            return *this;
        }

    private:
        StreamBase* SourceBase{ nullptr };
    };

    struct alignas( SKL_ALIGNMENT ) BinaryStreamTransaction : public IBinaryStreamObject
    {
        BinaryStreamTransaction( StreamBase& SourceStream ) noexcept
            : IBinaryStream{}, TransactionStream{ SourceStream }, TargetStream{ &SourceStream }
        {
            TransactionStream.bOwnsBuffer   = FALSE;
            TransactionStream.Buffer.Length = GetRemainingSize();
            TransactionStream.Buffer.Buffer = GetFront();
            TransactionStream.Position      = 0;
        }
        ~BinaryStreamTransaction() noexcept
        {
            Commit();
            Release();
        }

        BinaryStreamTransaction( const BinaryStreamTransaction& Other ) noexcept
            : TransactionStream{ Other.TransactionStream }, TargetStream{ Other.TargetStream }
        {
            SKL_ASSERT( false == Other.OwnsBuffer() );
        }
        BinaryStreamTransaction& operator=( const BinaryStreamTransaction& Other ) noexcept
        {
            SKL_ASSERT( this != &Other );
            TransactionStream = Other.TransactionStream;
            TargetStream      = Other.TargetStream;
            SKL_ASSERT( false == Other.OwnsBuffer() );
            return *this;
        }

        //!
        //! "Commits" changes to the underlying Stream
        //! 
        //! \remarks After Committing, the Buffer(ptr) is pushed forward by the committed amount and the Position is reset to 0 (rebased)
        //! 
        void CommitAndRebase() noexcept
        {
            // Commit to base 
            TargetStream->Position += GetPosition();

            // Rebase
            TransactionStream.Buffer.Length  = GetRemainingSize();
            TransactionStream.Buffer.Buffer += GetPosition();
            TransactionStream.Position       = 0;
        }

        //! "Commits" changes to the underlying Stream
        SKL_FORCEINLINE void Commit() noexcept
        {
            // Commit to base 
            TargetStream->Position += GetPosition();
        }

        //! "Rolls back" changes by resetting the Position to 0
        SKL_FORCEINLINE void Rollback( ) noexcept
        {
            TransactionStream.Position = 0;
        }

        //! Release the underlying stream
        void Release() noexcept
        {
            TransactionStream.Position      = 0;
            TransactionStream.Buffer.Length = 0;
            TransactionStream.Buffer.Buffer = nullptr;
            TargetStream                    = nullptr;
        }

        //! Get the transaction stream 
        SKL_FORCEINLINE StreamBase& GetStream() noexcept 
        { 
            return TransactionStream;
        }
        
        //! Get the transaction stream 
        SKL_FORCEINLINE const StreamBase& GetStream() const noexcept 
        { 
            return TransactionStream;
        }

        //! Get the transaction target stream 
        SKL_FORCEINLINE StreamBase& GetTargetStream() noexcept 
        { 
            SKL_ASSERT( nullptr != TargetStream );
            return *TargetStream;
        }
        
        //! Get the transaction target stream 
        SKL_FORCEINLINE const StreamBase& GetTargetStream() const noexcept 
        { 
            SKL_ASSERT( nullptr != TargetStream );
            return *TargetStream;
        }

        //! Construct a transaction stream for a given stream
        SKL_FORCEINLINE SKL_NODISCARD static StreamBase CreateTransactionStream( const StreamBase& InTargetStream ) noexcept
        {
            SKL_ASSERT( ( static_cast<int64_t>( InTargetStream.Buffer.Length ) - InTargetStream.Position ) > 0 );

            return StreamBase
            (
                  0
                , InTargetStream.Buffer.Length - InTargetStream.Position
                , InTargetStream.Buffer.Buffer + InTargetStream.Position
                , false
            );
        }
        
        //! Commit the transaction 
        SKL_FORCEINLINE static void CommitTransactionStream( StreamBase& InTransactionStream, StreamBase& InTargetStream  ) noexcept
        {
            SKL_ASSERT( ( static_cast<size_t>( InTransactionStream.GetPosition() ) + InTargetStream.GetPosition() ) < std::numeric_limits<uint32_t>::max() );
            InTargetStream.Position += InTransactionStream.GetPosition();
        }
        
        //! Commit the transaction and rebase
        SKL_FORCEINLINE static void CommitTransactionStreamAndRebase( StreamBase& InTransactionStream, StreamBase& InTargetStream  ) noexcept
        {
            SKL_ASSERT( ( static_cast<size_t>( InTransactionStream.GetPosition() ) + InTargetStream.GetPosition() ) < std::numeric_limits<uint32_t>::max() );
            InTargetStream.Position += InTransactionStream.GetPosition();

            InTransactionStream.Buffer.Length  = IStreamObjectReader::FromStreamBase( InTransactionStream )->GetRemainingSize();
            InTransactionStream.Buffer.Buffer += InTransactionStream.GetPosition();
            InTransactionStream.Position       = 0;
        }

    private:
        StreamBase  TransactionStream;
        StreamBase* TargetStream;
    };
}