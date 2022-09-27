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
    struct alignas( SKL_ALIGNMENT ) BufferStream : public BinaryStream
    {
        BufferStream( uint8_t* InBuffer, uint32_t InSize, uint32_t InPosition = 0 ) noexcept
            : BinaryStream{ InBuffer, InSize, InPosition, false } {}
        BufferStream( uint32_t InSize, uint32_t InPosition = 0 ) noexcept
            : BinaryStream{ nullptr, InSize, InPosition, true }
        {
            Base.Buffer.Buffer = reinterpret_cast<uint8_t*>( SKL_MALLOC_ALIGNED( InSize, SKL_ALIGNMENT ) );
            SKL_ASSERT( nullptr != Base.Buffer.Buffer ); 
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
                Base.Buffer.Buffer = reinterpret_cast<uint8_t*>( SKL_MALLOC_ALIGNED( Base.Buffer.Length, SKL_ALIGNMENT ) );
                SKL_ASSERT( nullptr != Base.Buffer.Buffer );
                memcpy( Base.Buffer.Buffer, Other.Base.Buffer.Buffer, Base.Buffer.Length );
            }
        }
        BufferStream& operator=( const BufferStream& Other ) noexcept
        {
            SKL_ASSERT( this != &Other );

            Clear();

            Base = Other.Base;

            if( true == OwnsBuffer() )
            {
                Base.Buffer.Buffer = reinterpret_cast<uint8_t*>( SKL_MALLOC_ALIGNED( Base.Buffer.Length, SKL_ALIGNMENT ) );
                SKL_ASSERT( nullptr != Base.Buffer.Buffer );
                memcpy( Base.Buffer.Buffer, Other.Base.Buffer.Buffer, Base.Buffer.Length );
            }

            return *this;
        }
        BufferStream( BufferStream&& Other ) noexcept
            : BinaryStream{ Other }
        {
            Other.Base.Position      = 0;
            Other.Base.Buffer.Length = 0;
            Other.Base.Buffer.Buffer = nullptr;
            Other.Base.bOwnsBuffer   = FALSE;
        }
        BufferStream& operator=( BufferStream&& Other ) noexcept
        {
            SKL_ASSERT( this != &Other );

            Clear();

            Base                     = Other.Base;
            Other.Base.Position      = 0;
            Other.Base.Buffer.Length = 0;
            Other.Base.Buffer.Buffer = nullptr;
            Other.Base.bOwnsBuffer   = FALSE;

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
            if( true == OwnsBuffer() && nullptr != Base.Buffer.Buffer )
            {
                SKL_FREE_SIZE_ALIGNED( Base.Buffer.Buffer, Base.Buffer.Length, SKL_ALIGNMENT );
            }

            Base.Position      = 0;
            Base.Buffer.Length = 0;
            Base.Buffer.Buffer = nullptr;
            Base.bOwnsBuffer   = FALSE;
        }
    };
}