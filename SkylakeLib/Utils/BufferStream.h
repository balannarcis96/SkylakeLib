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
    struct alignas( SKL_ALIGNMENT ) BufferStream : public BinaryStream
    {
        BufferStream( uint8_t* InBuffer, uint32_t InSize, uint32_t InPosition = 0 ) noexcept
            : BinaryStream{ InBuffer, InSize, InPosition, false } {}
        BufferStream( uint32_t InSize, uint32_t InPosition = 0 ) noexcept
            : BinaryStream{ nullptr, InSize, InPosition, true }
        {
            Stream.Buffer.Buffer = reinterpret_cast<uint8_t*>( SKL_MALLOC_ALIGNED( InSize, SKL_ALIGNMENT ) );
            SKL_ASSERT( nullptr != Stream.Buffer.Buffer ); 
        }
        ~BufferStream() noexcept
        {
            Clear();
        }

        BufferStream( const BufferStream& Other ) noexcept
            : BinaryStream{ Other }
        {
            if( true == OwnsBuffer() )
            {
                Stream.Buffer.Buffer = reinterpret_cast<uint8_t*>( SKL_MALLOC_ALIGNED( Stream.Buffer.Length, SKL_ALIGNMENT ) );
                SKL_ASSERT( nullptr != Stream.Buffer.Buffer );
                memcpy( Stream.Buffer.Buffer, Other.Stream.Buffer.Buffer, Stream.Buffer.Length );
            }
        }
        BufferStream& operator=( const BufferStream& Other ) noexcept
        {
            SKL_ASSERT( this != &Other );

            Clear();

            Stream = Other.Stream;

            if( true == OwnsBuffer() )
            {
                Stream.Buffer.Buffer = reinterpret_cast<uint8_t*>( SKL_MALLOC_ALIGNED( Stream.Buffer.Length, SKL_ALIGNMENT ) );
                SKL_ASSERT( nullptr != Stream.Buffer.Buffer );
                memcpy( Stream.Buffer.Buffer, Other.Stream.Buffer.Buffer, Stream.Buffer.Length );
            }

            return *this;
        }
        BufferStream( BufferStream&& Other ) noexcept
            : BinaryStream{ Other }
        {
            Other.Stream.Position      = 0;
            Other.Stream.Buffer.Length = 0;
            Other.Stream.Buffer.Buffer = nullptr;
            Other.Stream.bOwnsBuffer   = FALSE;
        }
        BufferStream& operator=( BufferStream&& Other ) noexcept
        {
            SKL_ASSERT( this != &Other );

            Clear();

            Stream                     = Other.Stream;
            Other.Stream.Position      = 0;
            Other.Stream.Buffer.Length = 0;
            Other.Stream.Buffer.Buffer = nullptr;
            Other.Stream.bOwnsBuffer   = FALSE;

            return *this;
        }

        static std::optional<BufferStream> OpenFile( const char * InFileName ) noexcept
        {
            auto File{ std::ifstream( InFileName, std::ifstream::binary ) };
            if( false == File.is_open( ) )
            {
                SKLL_WRN_FMT( "BufferStream::OpenFile(InFileName) Failed to open file %s", InFileName );
                return {};
            } 

            File.seekg( 0, std::ifstream::end );
            const uint32_t ReadSize{ static_cast< uint32_t >( File.tellg( ) ) };
            File.seekg( 0, std::ifstream::beg );
            File.close();

            BufferStream Result{ ReadSize };
            SKL_ASSERT( Result.GetBufferSize() == ReadSize );
            if( false == Result.ReadFromFile( InFileName ) ) SKL_UNLIKELY
            {
                return {};
            }

            return { Result };
        }

        //! Clear this buffer stream, deallocates buffer is bOwnsBuffer == true
        void Clear() noexcept
        {
            if( true == OwnsBuffer() && nullptr != Stream.Buffer.Buffer )
            {
                SKL_FREE_SIZE_ALIGNED( Stream.Buffer.Buffer, Stream.Buffer.Length, SKL_ALIGNMENT );
            }

            Stream.Position      = 0;
            Stream.Buffer.Length = 0;
            Stream.Buffer.Buffer = nullptr;
            Stream.bOwnsBuffer   = FALSE;
        }
    };
}