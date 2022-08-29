//!
//! \file BinaryStream.cpp
//! 
//! \brief Binary stream maipulation abstractions
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//!
#include<SkylakeLib.h>

namespace SKL
{
    std::optional<BufferStream> BufferStream::OpenFile( const char * InFileName ) noexcept
    {
        auto File{ std::ifstream( InFileName, std::ifstream::binary ) };
        if( false == File.is_open( ) )
        {
            SKL_WRN_FMT( "BufferStream::OpenFile(InFileName) Failed to open file %s", InFileName );
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
}