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
    bool IStreamReader::SaveToFile( const char* InFileName, bool bAppendInsteadOfTruncate, bool bPositionAsSize, bool bSaveAsText ) const noexcept
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
            SKL_WRN_FMT( "IStreamReader::SaveToFile(InFileName) Failed to open file %s", InFileName );
            return false;
        }

        File.write( reinterpret_cast< const char * >( GetBuffer( ) ), WriteSize );

        const auto Result{ File.good() };
        if( false == Result )
        {
            SKL_WRN_FMT( "IStreamReader::SaveToFile(InFileName) Failed to write bytesCount:%u to file %s ", WriteSize, InFileName );
        }

        File.close();

        return Result;
    }

    bool IStreamWriter::ReadFromFile( const char* InFileName, bool bTruncate ) noexcept
    {
        auto File{ std::ifstream( InFileName, std::ifstream::binary ) };
        if( false == File.is_open( ) )
        {
            SKL_WRN_FMT( "IStreamWriter::ReadFromFile(InFileName) Failed to open file %s", InFileName );
            return false;
        } 

        File.seekg( 0, std::ifstream::end );
        const uint32_t ReadSize{ static_cast< uint32_t >( File.tellg( ) ) };
        File.seekg( 0, std::ifstream::beg );

        if( 0 == ReadSize ) SKL_UNLIKELY
        {
            SKL_WRN_FMT( "IStreamWriter::ReadFromFile(InFileName) Empty file %s", InFileName );
            File.close();
            return false;
        }

        const auto bItFits{ CanFit( ReadSize ) };
        if( false == bItFits && false == bTruncate )
        {
            SKL_VER_FMT( "IStreamWriter::ReadFromFile(InFileName) Failed to read file %s! Exceeds buffer size. BufferSize:%u FileSize:%u", InFileName, GetRemainingSize(), ReadSize );
            File.close();
            return false;
        }

        File.read( reinterpret_cast<char*>( GetFront() ), ReadSize );

        const auto Result{ File.good() };
        if( false == Result )
        {
            SKL_WRN_FMT( "IStreamReader::ReadFromFile(InFileName) Failed to read bytesCount:%u to file %s ", ReadSize, InFileName );
        }

        File.close();

        return Result;
    }

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