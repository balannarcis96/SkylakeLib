//!
//! \file Port_Windows.h
//! 
//! \brief Windows platform abstraction layer for SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    constexpr TSocket CInvalidSocket = (TSocket)(~0);

    struct AsyncIOOpaqueType
    {
        AsyncIOOpaqueType() 
        {
            Reset();    
        }

        uint8_t Body[32]; // sizeof(OVERLAPPED);
        
        //! \brief Reset this instance for reuse 
        SKL_FORCEINLINE void Reset() noexcept
        {
            memset( Body, 0, sizeof( uint8_t ) * std::size( Body ) );
        }
    };

    struct Timer
    {   
        Timer() = default;
        ~Timer() = default;
        
        //! \brief Init the timer
        bool Init() noexcept;

        //! \brief Tick(update) the timer and get the total milliseconds elapsed since the first Tick()
        double Tick() noexcept;        

        //! \brief Get total milliseconds elapsed since the last Tick()
        SKL_FORCEINLINE double GetElapsed() const noexcept { return Elapsed; }

        //! \brief Get total milliseconds elapsed since the first Tick()
        SKL_FORCEINLINE double GetTotalTime() const noexcept { return TotalTime; }

    private:
        double  Elapsed          { 0.0 };
        double  TotalTime        { 0.0 };
        double  FrequencySeconds { 0.0 };
        int64_t I                { 0 };
        int64_t Start            { 0 };
    };
    
    int32_t GGetLastError( ) noexcept;

    uint32_t IPv4FromStringA( const char* IpString )noexcept;

    uint32_t IPv4FromStringW( const wchar_t* IpString )noexcept;

    bool GWideCharToMultiByte( const wchar_t * InBuffer, size_t InBufferSize, char* OutBuffer, int32_t InOutBufferSize ) noexcept;

    bool GMultiByteToWideChar( const char * InBuffer, size_t InBufferSize, wchar_t* OutBuffer, int32_t InOutBufferSize ) noexcept;

    //UTF16 -> UTF8
    template<size_t N>
    bool GWideCharToMultiByte( const wchar_t( &InBuffer ) [ N ], char* OutBuffer, int32_t OutBufferSize ) noexcept
    {
        return GWideCharToMultiByte( InBuffer, N, OutBuffer, OutBufferSize );
    }

    //UTF8 -> UTF16
    template<size_t N>
    bool GMultiByteToWideChar( const char( &InBuffer ) [ N ], wchar_t* OutBuffer, int32_t OutBufferSize ) noexcept
    {
        return GMultiByteToWideChar( InBuffer, N, OutBuffer, OutBufferSize );
    }

    //UTF16 -> UTF8
    template<size_t N, size_t M>
    bool GWideCharToMultiByte( const wchar_t( &InBuffer ) [ N ], char( &OutBuffer ) [ M ] ) noexcept
    {
        return GWideCharToMultiByte( InBuffer, N, OutBuffer, M );
    }

    //UTF8 -> UTF16
    template<size_t N, size_t M>
    bool GMultiByteToWideChar( const char( &InBuffer ) [ N ], wchar_t( &OutBuffer ) [ M ] ) noexcept
    {
        return GMultiByteToWideChar( InBuffer, N, OutBuffer, M );
    }
}

